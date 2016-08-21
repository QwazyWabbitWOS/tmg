#include "g_local.h"
#include "performance.h"
#include "g_items.h"
#include "g_cmds.h"
#include "timer.h"
#include "s_map.h"
#include "bot.h"

// global
int match_state;
float match_state_end;

// static
float match_nextthink;

void ResetItems (void)
{
	int i;
	edict_t *ent;

	ent = &g_edicts[1];
	for (i = 1; i < globals.num_edicts; i++, ent++)
	{
		if (!ent->inuse || ent->client || !ent->item)
			continue;

		// Remove any dropped items
		if (ent->spawnflags & (DROPPED_ITEM | DROPPED_PLAYER_ITEM))
		{
			ent->nextthink = level.time;
			ent->think = G_FreeEdict;
		}
		else if (Q_stricmp(ent->classname, "item_flag_team1") == 0 ||
				 Q_stricmp(ent->classname, "item_flag_team2") == 0)
		{
			if (!(ent->spawnflags & DROPPED_ITEM))
			{
				ent->flags |= FL_RESPAWN;
				ent->svflags |= SVF_NOCLIENT;
				ent->solid = SOLID_NOT;
			}
		}
		else
			SetRespawn (ent, 0);
	}
	CTFCheckRules();
	CTFResetFlags();
}

//#define rndnum(y,z) ((random()*((z)-((y)+1)))+(y))
void RestartLevel()
{
	edict_t *player;
	int i;
	edict_t *dropped = NULL;
	techspawn = false;
	ResetItems();
	match_nextthink = level.time + FRAMETIME * 10.0;
	match_state = STATE_NEEDPLAYERS;
	match_state_end = 1.0f;
	ResetCaps();
	hstime = level.time - 10.0;
	mapvoteactive = false;

	for_each_player(player, i)
	{
		player->client->resp.score = 0;
		player->client->resp.frags = 0;
		player->client->resp.spree = 0;
		player->client->resp.deaths = 0;
		player->client->pers.db_hud = true;
		if(player->client->pers.pl_state == PL_WARMUP)
			player->client->pers.pl_state = PL_NEEDSPAWN;
		player->client->resp.startframe = level.newframenum;
		if (ctf->value)
		{
			if ((!flag1_item || !flag2_item) && ctf->value)
				CTFInit();
			if (player->client->pers.inventory[ITEM_INDEX(flag1_item)])
			{
				dropped = Drop_Item(player, flag1_item);
				player->client->pers.inventory[ITEM_INDEX(flag1_item)] = 0;
				my_bprintf(PRINT_HIGH, "%s lost the %s flag!\n",
						   player->client->pers.netname, CTFTeamName(CTF_TEAM1));
			}
			else if (player->client->pers.inventory[ITEM_INDEX(flag2_item)])
			{
				dropped = Drop_Item(player, flag2_item);
				player->client->pers.inventory[ITEM_INDEX(flag2_item)] = 0;
				my_bprintf(PRINT_HIGH, "%s lost the %s flag!\n",
						   player->client->pers.netname, CTFTeamName(CTF_TEAM2));
			}
			if (dropped)
			{
				dropped->think = G_FreeEdict;
				dropped->timestamp = level.time;
				dropped->nextthink = level.time + FRAMETIME;
			}
			//JSW - clear flag carrier var
			player->hasflag = 0;
		}
	}
}

/**
 start the server in a 'waiting for players' state
 */
void ServerInit (int resetall)
{
	int i;
	// reset server state.
	techspawn = false;
	level.allowpickup = level.time;
	match_nextthink = level.time + FRAMETIME * 10.0;
	match_state = STATE_NEEDPLAYERS;
	match_state_end = 1;
	locked_teams = false;
	serverlocked = false;
	dmflagtimer = level.time;
	hstime = level.time - 10;
	mapvoteactive = false;
	votetime = 0;
	maplist->currentmapvote = -1;
	maplist->nextmap = -1;

	for (i = 0; i < maplist->nummaps; i++)
	{
		if (strcmp(maplist->mapname[i], level.mapname) == 0)
			maplist->currentmap = i;
	}

	//	gi.cvar_set("mapvote", "0");
	i = rndnum(0,9);
	//set the info string value for new bot detection
	switch (i)
	{
		case 0:
			gi.cvar_set("prox", "norat");
			break;
		case 1:
			gi.cvar_set("prox", "nobot");
			break;
		case 2:
			gi.cvar_set("prox", "nohack");
			break;
		case 3:
			gi.cvar_set("prox", "nolamma");
			break;
		case 4:
			gi.cvar_set("prox", "nrat");
			break;
		case 5:
			gi.cvar_set("prox", "orat");
			break;
		case 6:
			gi.cvar_set("prox", "not");
			break;
		case 7:
			gi.cvar_set("prox", "nt");
			break;
		case 8:
			gi.cvar_set("prox", "rat");
			break;
		case 9:
			gi.cvar_set("prox", "nat");
			break;
		default:
			gi.cvar_set("prox", "nohom");
	}
}

// is the countdown in the final minute ???
int CountDownInFinalMinute ()
{
	// no count-down if not in the right state
	if (match_state < STATE_WARMUP)
		return false;
	// last 60 seconds ?
	return (match_state_end - level.time <= 60);
}

// display minute warnings and the countdown on the screen near end.
void CountDown()
{
	edict_t *player;
	int i;
	long seconds_left;

	// no countdown unless in countdown or playing states
	if (match_state < STATE_COUNTDOWN)
		return;

	// how long left ?
	seconds_left = (long) ceil(match_state_end - level.time);
	//DbgPrintf("%d %f %f\n", seconds_left, match_state_end, level.time);
	// ignore if <0 seconds
	if (seconds_left < 0)
		return;

	// not in the final minute yet ?
	if (!CountDownInFinalMinute ())
	{
		// print a warning each whole minute
		if ((seconds_left % 60) == 0)
		{
			if (seconds_left > 60 && seconds_left < 301)  // five mins down ..
				safe_bprintf (PRINT_HIGH,
							  "%d minutes remaining.\n", seconds_left/60);
		}
	}
	else	// final minute, countdown on screen !
	{
		if (seconds_left == 60)
			if (match_state == STATE_WARMUP)
				safe_bprintf (PRINT_HIGH, "1 minute left in Warmup!\n");
			else
				safe_bprintf (PRINT_HIGH, "FINAL MINUTE !\n");
		//raven mapvoting
			else if ((seconds_left == 2) &&
					 mapvote->value && (maplist->nextmap == -1))
			{
				safe_bprintf (3, "Use menu to vote for next map!\n");
				FillMapNames();
			}
			else if (seconds_left == 30)
				safe_bprintf (PRINT_HIGH, "30 seconds left\n");
			else if (seconds_left == 20)
				safe_bprintf (PRINT_HIGH, "20 seconds left\n");
			else if (seconds_left == 11)
			{
				if (match_state == STATE_WARMUP)
					safe_bprintf (PRINT_HIGH, "10 seconds left in Warmup\n");
				else
					for_each_player(player, i)
				{
					gi.sound (player, CHAN_AUTO,
							  gi.soundindex ("world/10_0.wav"),
							  1, ATTN_NORM, 0);
				}
			}
	}

	// do K3WL stuff for the countdown to start.
	if (match_state == STATE_COUNTDOWN && seconds_left <= 30)
	{
		// iterate thru all clients, count down by seconds now.
		for_each_player(player, i)
		{
			// show digits on the screen
			player->client->ps.stats[STAT_COUNTDOWN] = seconds_left;

			// play a sound during countdown phase
			if (match_state == STATE_COUNTDOWN &&
				seconds_left < 30 &&
				seconds_left > 12)
					gi.sound (player, CHAN_ITEM,
							  gi.soundindex ("misc/secret.wav"),
							  1, ATTN_NORM, 0);

			if (match_state == STATE_COUNTDOWN && seconds_left == 10)
			{
				gi.sound (player, CHAN_AUTO,
						  gi.soundindex ("world/10_0.wav"),
						  1, ATTN_NORM, 0);
			}
		}
	}
}

void CheckState()
{
	edict_t *player;
	int i;
	// DONT CHECK STATE DURING INTERMISSION (END OF LEVEL) TIME
	if (level.intermissiontime)
		return;


	//start warmup mode
	if(match_state == STATE_NEEDPLAYERS)
	{
		match_state = STATE_WARMUP;
		match_state_end = level.time + 1;
		level.warmup = true;//put all players into limbo mode
	}

	//start
	if ((match_state == STATE_WARMUP) && (level.time >= match_state_end))
	{
		match_state = STATE_COUNTDOWN;
		match_state_end = level.time + warmup_time->value;
		match_nextthink = level.time + FRAMETIME;
		CountDown ();
		return;
	}

	//start game !
	if (match_state == STATE_COUNTDOWN && level.time >= match_state_end)
	{
		for_each_player(player, i)
		{
			if(player->client->pers.pl_state == PL_WARMUP)
				player->client->pers.pl_state = PL_NEEDSPAWN;

			player->client->resp.startframe = level.newframenum;
		}

		match_state = STATE_PLAYING;
		level.warmup = false;
		if(timelimit->value > 0)
			match_state_end = level.time + timelimit->value * 60;
		else
			match_state_end = level.time + 999 * 60;

//		if (developer->value)
//			gi.dprintf("2 match_state_end = %f, "
//					   "level.time = %f, votetime = %f\n",
//					   match_state_end, level.time, votetime);

		level.newframenum = 0;
		level.allowpickup = level.time + 1;
		gi.dprintf("Match Started!! Level: %s\n", level.mapname);

		//BOTZ spawning
		if(use_bots->value)
		{
			//bot spawning
			spawncycle = level.time + 1;
			SpawnWaitingBots = bot_num->value;
			//set up for next check
			bot_time = level.time + 5;
			if(!bot_num->value && use_bots->value)
				wait_time = level.time + 5;	//spawning
			kill_time = level.time + 10;	//removing
											//end bot stuff
		}
	}
}

void TimerThink (void)
{
	int newdmflag;
	int i;
	edict_t *carrier;

	if (level.time > match_nextthink)
	{
		// countdowns ?
		CountDown ();
		// check duel state...
		CheckState ();

		if(use_bots->value)
			Adjust_Bot_Number();

		match_nextthink += FRAMETIME * 10.0;
		ctfgame.players1 = 0;
		ctfgame.players2 = 0;
		ctfgame.players_total = 0;
		ctfgame.specs = 0;
		yesvotes = 0;
		novotes = 0;

//		gi.dprintf ("M. map to be voted on is %s %s\n",
//					maplist->mapname[maplist->currentmapvote],
//					maplist->mapnick[maplist->currentmapvote]);

		for (i = 1; i <= maxclients->value; i++)
		{
			carrier = g_edicts + i;
			if (carrier->inuse && mapvoteactive)
			{
				if (carrier->client->resp.vote == true)
					yesvotes++;
				else
					if (!carrier->bot_client) // bots can't vote
						novotes++;
			}
			if (ctf->value)
			{
				if (carrier->inuse && carrier->client->resp.ctf_team > 0)
				{
					if(carrier->client->resp.ctf_team == 1)
						ctfgame.players1++;
					else
						ctfgame.players2++;
				}
			}
			else if (carrier->client->pers.pl_state == PL_NEEDSPAWN)
			{
				ctfgame.players_total++;
			}
			else
				ctfgame.specs++;
		}
		if (mapvoteactive)
		{
//			gi.dprintf ("B. map to be voted on is %s %s\n",
//						maplist->mapname[maplist->currentmapvote],
//						maplist->mapnick[maplist->currentmapvote]);
			if (yesvotes + novotes > 0)
			{
				if ((yesvotes / (yesvotes + novotes)) >
					((float)vote_percentage->value / 100))
				{
					if (votemapnow)
						MapVoteThink(true, true);
					else
						MapVoteThink(true, false);
				}
				else
				{
					if (mapvotetime < level.time)
						MapVoteThink(false, true);
				}
			}

		}
	}

	// JSW - Do this calc every 10 seconds instead of through
	// all the functions that call (int)dmflags->value and
	// print when they are changed
	if (dmflagtimer < level.time)
	{
		if (randomrcon->value)
		{
			char cmd[80];
			char buff[80];
			char ch[4];
			int i;
			cmd[0] = '\0';
			strcpy (buff, "");
			for (i = 0; i < 10; i++)
			{
				int iNumber = rand() % 122;
				if( 48 > iNumber )
					iNumber += 48;
				if( ( 57 < iNumber ) &&
				   ( 65 > iNumber ) )
					iNumber += 7;
				if( ( 90 < iNumber ) &&
				   ( 97 > iNumber ) )
					iNumber += 6;
				sprintf(ch, "%i", iNumber);
				strcat (buff, ch);
			}
			strcat (buff, "\0");
			Com_sprintf (cmd, sizeof cmd, "rcon_password \"%s\" ", buff);
			gi.AddCommandString(cmd);
		}

		newdmflag = (int)dmflags->value;
		if (dmflag != newdmflag)
			my_bprintf (PRINT_CHAT,
						"DMFlags have been changed from %i to %i.\n",
						dmflag, newdmflag);

		dmflagtimer = level.time + 10;
		dmflag = newdmflag;

		RavCheckTeams();
		if (even_teams->value == 0)
			gi.cvar_forceset("even_teams", "100");
	}
	//end
	
}

