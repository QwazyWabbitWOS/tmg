#include "g_local.h"
#include "g_items.h"
#include "p_hud.h"
#include "hud.h"
#include "p_client.h"
#include "runes.h"
#include "highscore.h"
#include "statslog.h"
#include "intro.h"

/*
======================================================================

INTERMISSION

======================================================================
*/

void MoveClientToIntermission (edict_t *ent)
{
	gclient_t	*client;
	//RAV
	char song[80];
	//
	client = ent->client;

	if (deathmatch->value || coop->value)
		ent->client->showscores = true;
	VectorCopy (level.intermission_origin, ent->s.origin);

	ent->client->ps.pmove.origin[0] = level.intermission_origin[0]*8;
	ent->client->ps.pmove.origin[1] = level.intermission_origin[1]*8;
	ent->client->ps.pmove.origin[2] = level.intermission_origin[2]*8;
	VectorCopy (level.intermission_angle, ent->client->ps.viewangles);
	ent->client->ps.pmove.pm_type = PM_FREEZE;
	ent->client->ps.gunindex = 0;
	ent->client->ps.blend[3] = 0;
	ent->client->ps.rdflags &= ~RDF_UNDERWATER;

	// clean up powerup info
	client->quad_framenum = 0;
	client->invincible_framenum = 0;
	client->breather_framenum = 0;
	client->enviro_framenum = 0;
	client->grenade_blew_up = false;
	client->grenade_time = 0;

	ent->viewheight = 0;
	ent->s.modelindex = 0;
	ent->s.modelindex2 = 0;
	ent->s.modelindex3 = 0;
	ent->s.modelindex = 0;
	ent->s.effects = 0;
	ent->s.sound = 0;
	ent->solid = SOLID_NOT;

	// add the layout

	if (!ent->bot_client && (deathmatch->value || coop->value))
	{
		DeathmatchScoreboardMessage (ent, NULL);
		gi.unicast (ent, true);
	}
	//RAV
	if(wavs->value && !ent->bot_client)
	{
		Com_sprintf(song, sizeof(song), songtoplay->string);
		//gi.sound (ent, CHAN_ITEM, gi.soundindex (song), 1, ATTN_NORM, 0);
		StuffCmd(ent, va("play %s\n", song));
	}
}

void BeginIntermission (edict_t *targ)
{
	int		i, n;
	edict_t	*ent, *client;

	if (level.intermissiontime)
		return;		// already activated

	//RAV WAVMOD
	if (use_song_file->value)
	{
		char *nextwav;
		char sound[MAX_QPATH];

		if ((nextwav = Wav_Mod_Next()) != NULL)
		{
			gi.cvar_set("wav", nextwav);
			//set up the songtoplay cvar
			Com_sprintf(sound, sizeof sound, "misc/%s.wav", nextwav);
			gi.cvar_set("song", sound);
		}
	}

	game.autosaved = false;

	// respawn any dead clients
	for (i=0 ; i<maxclients->value ; i++)
	{
		client = g_edicts + 1 + i;
		if (!client->inuse)
			continue;
		if (client->health <= 0)
			Respawn(client, false);
	}

	level.intermissiontime = level.time;
	level.changemap = targ->map;

	if (strstr(level.changemap, "*"))
	{
		if (coop->value)
		{
			for (i=0 ; i<maxclients->value ; i++)
			{
				client = g_edicts + 1 + i;
				if (!client->inuse)
					continue;
				// strip players of all keys between units
				for (n = 0; n < MAX_ITEMS; n++)
				{
					if (itemlist[n].flags & IT_KEY)
						client->client->pers.inventory[n] = 0;
				}
			}
		}
	}
	else
	{
		if (!deathmatch->value)
		{
			level.exitintermission = 1;		// go immediately to the next level
			return;
		}
	}

	level.exitintermission = 0;

	// find an intermission spot
	ent = G_Find (NULL, FOFS(classname), "info_player_intermission");
	if (!ent)
	{	// the map creator forgot to put in an intermission point...
		ent = G_Find (NULL, FOFS(classname), "info_player_start");
		if (!ent)
			ent = G_Find (NULL, FOFS(classname), "info_player_deathmatch");
	}
	else
	{	// chose one of four spots
		i = rand() & 3;
		while (i--)
		{
			ent = G_Find (ent, FOFS(classname), "info_player_intermission");
			if (!ent)	// wrap around the list
				ent = G_Find (ent, FOFS(classname), "info_player_intermission");
		}
	}

	VectorCopy (ent->s.origin, level.intermission_origin);
	VectorCopy (ent->s.angles, level.intermission_angle);

	if (deathmatch->value && ctf->value)
		CTFCalcScores();

	if(highscores->value)
		SaveHighScores();

	//QW// logging of hook stats per player
	for (i = 0; i < maxclients->value; i++)
	{
		ent = g_edicts + 1 + i;
		if (ent->inuse && !ent->bot_client)
			StatsLogHooks(ent);
	}

	// move all clients to the intermission point
	for (i=0 ; i<maxclients->value ; i++)
	{
		client = g_edicts + 1 + i;
		if (client->inuse)
			MoveClientToIntermission (client);
	}
}

void DeathmatchScoreboardMessage (edict_t *ent, edict_t *killer)
{
	char	entry[MAX_MSGLEN];	// Temporary string store
	char	string[MAX_MSGLEN]; // The scoreboard message
	size_t	len;
	int		i;
	int		j;
	int		k;
	int		sorted[MAX_CLIENTS] = {0};
	int		sortedscores[MAX_CLIENTS] = {0};
	int		score, total;
	int		last = 0;
	gclient_t	*cl;
	edict_t		*cl_ent;
	int maxsize = MAX_MSGLEN;
	size_t	stringlength;

	// Highscores
	FILE    *hs_file;	// the highscore file for this map
	char    filename[MAX_QPATH];
	char    line[80];

	Com_sprintf(filename, sizeof filename, "%s/%s/%s/hs/%s_hs.txt", 
		basedir->string, game_dir->string, cfgdir->string, level.mapname);

	entry[0] = 0;
	string[0] = 0;
	stringlength = strlen(string);

	if(ent->client->overflowed)
		return;

	if (ent->client->showscores)
	{
		// schedule a hud update
		ent->client->hudtime = level.framenum + 2; //QW// was: 8
		if (ctf->value)
		{
			if (newscore->value)
				CTFScoreboardMessageNew (ent, killer);
			else
				CTFScoreboardMessage (ent, killer);
			return;
		}

		// sort the clients by team and score
		total = 0;
		for (i = 0; i < game.maxclients; i++)
		{
			cl_ent = g_edicts + 1 + i;
			if ((!cl_ent->inuse) || 
				(cl_ent->client->resp.spectator) ||
				(!cl_ent->client->pers.pl_state))
				continue;

			score = game.clients[i].resp.score;
			for (j = 0; j < total; j++)
			{
				if (score > sortedscores[j])
					break;
			}

			for (k = total; k > j; k--)
			{
				sorted[k] = sorted[k-1];
				sortedscores[k] = sortedscores[k-1];
			}
			sorted[j] = i;
			sortedscores[j] = score;
			total++;
		}

		// print level name & header
		Com_sprintf(string, sizeof string,
			//"xv 0 yv -100 cstring2 \"%s\" "
			"xv 0 yv -90 cstring2 \"MAP: %s\" "
			"xv 0 yv -70 cstring2 \"%s %s\" "
			//"xv 20 yv -60 string2 \"Longest\" "
			"xv -130 yv -50 string2 \"Kills Ping FPH Time Spree Name \"",
			/*hostname->string,*/ level.mapname, MOD, MOD_VERSION);

		len = strlen(string);

		// add the clients in sorted order
		for (i = 0; i < 12; i++) //only allow 12 per board !!!
		{
			if (i >= total)
				break; // we're done

			if (i < total)
			{
				cl = &game.clients[sorted[i]];
				cl_ent = g_edicts + 1 + sorted[i];
				Com_sprintf(entry, sizeof entry, "yv %d ", -30 + i * 8);
				sprintf(entry + strlen(entry),
					"xv -125 %s \"%3d  %3d  %3d %3d %3d    %s\"",
					(cl_ent == ent) ? "string2" : "string",
					cl->resp.score,
					(cl->ping > 999) ? 999 : cl->ping, cl->resp.fph, 
					(level.framenum - cl->resp.enterframe) / 600,
					cl->resp.bigspree, cl->pers.netname);

				if (maxsize - len > strlen(entry))
				{
					strcat(string, entry);
					len = strlen(string);
					last = i;
				}
			}
		}

		// Show how many didn't fit
		if (total - last > 1)
			sprintf(string + strlen(string), 
			"xv 108 yv %d string2 \"& %d more\" ",
			-30 + (last + 1) * 8, total - last - 1);

		// put in spectators if we have enough room
		j = -30; // set up yv position for spectator list
		if (maxsize - len > 50)
		{
			for (i = 0; i < maxclients->value; i++)
			{
				cl_ent = g_edicts + 1 + i;
				cl = &game.clients[i];
				if (cl_ent->inuse && cl_ent->client->pers.pl_state == PL_SPECTATOR)
				{
					Com_sprintf(entry, sizeof entry,
						"xv 195 yv %d %s \"%s\" " // spectator name
						"xv 300 yv %d %s \"<%s>\" ", // who he's chasing
						j, (cl_ent == ent) ? "string2" : "string", cl->pers.netname,
						j, (cl_ent == ent) ? "string2" : "string",
						(cl_ent->client->chase_target != NULL) ? 
						cl_ent->client->chase_target->client->pers.netname : "S");

					if (maxsize - len > strlen(entry))
					{
						strcat(string, entry);
						len = strlen(string);
					}
					j += 8;
				}
			}
		}

		//DB Highscores
		if (ent->client->showhighscores)
		{
			if ((hs_file = fopen(filename, "r")) != NULL)
			{
				i = 0;
				while ( fgets(line, 80, hs_file) )
				{
					Com_sprintf (entry, sizeof entry,
						"xv 2 yv %i string \"%s\"",
						i * 8 + 24, line);
					j = strlen(entry);
					if (stringlength + j > 1400)
						break;
					strcpy (string + stringlength, entry);
					stringlength += j;
					i++;
				}

				fclose(hs_file);
				j = strlen(entry);
				if (stringlength + j < 1400)
				{
					strcpy (string + stringlength, entry);
					stringlength += j;
				}
			}
		}
		//end highscores
		gi.WriteByte (svc_layout);
		gi.WriteString (string);
	}
	else if (ent->client->pers.db_hud || ent->client->pers.motd == true)
	{
		string[0] = 0;
		stringlength = strlen(string);
		Com_sprintf (entry, sizeof entry, ShowHud(ent));
		j = strlen(entry);
		if (!(stringlength + j > 1400))
		{
			strcpy (string + stringlength, entry);
			gi.WriteByte (svc_layout);
			gi.WriteString (string);
		}
	}
	//DbgPrintf("%d Length: %d %s\n", level.framenum, strlen(string), string);
}

void DeathmatchScoreboard (edict_t *ent)
{
	// Make sure ent exists!
	if (!G_EntExists(ent))
		return;

	DeathmatchScoreboardMessage (ent, ent->enemy);
	gi.unicast (ent, true);
}


/* 
Interlocked with Cmd_HighScore_f.
Toggle scoreboard on/off on command.
If highscores is up, change to scoreboard
but toggle off if commanded again.
*/
void Cmd_Score_f (edict_t *ent)
{
	ent->client->showinventory = false;
	ent->client->showhelp = false;

	// if high scores are up, flip to scoreboard
	if (ent->client->showhighscores)
	{
		ent->client->showscores = false;
		ent->client->showhighscores = false;
	}

	//QW// this block may be obsolete on r1q2 server
	//RAV
	// do not update twice prevent netchan bug, overflows
	if (ent->client->resp.menu_time == level.framenum)
		return; 

	ent->client->resp.menu_time = level.framenum + 1;
	//

	//ZOID
	if (ent->client->menu)
		PMenu_Close(ent);
	//ZOID

	if (ent->client->showscores)
	{
		ent->client->showscores = false;
		return;
	}

	ent->client->showscores = true;
	DeathmatchScoreboard (ent);
}

/* 
Interlocked with Cmd_Score_f.
Toggle highscores on/off on command.
If scoreboard is up, change to highscores
but toggle off if commanded again.
*/
void Cmd_HighScore_f (edict_t *ent)
{
	ent->client->showinventory = false;
	ent->client->showhelp = false;

	if (ent->client->showhighscores)
	{
		ent->client->showscores = false;
		ent->client->showhighscores = true;
	}

	//RAV //QW// this block may be obsolete on r1q2 server
	// do not update twice prevent netchan bug, overflows
	if (ent->client->resp.menu_time == level.framenum)
		return; 

	ent->client->resp.menu_time = level.framenum + 1;
	//

	if (ent->client->menu)
		PMenu_Close(ent);

	//if (!deathmatch->value && !coop->value)
	//	return;

	if (ent->client->showhighscores)
	{
		ent->client->showscores = false;
		ent->client->showhighscores = false;
		return;
	}

	ent->client->showscores = true;
	ent->client->showhighscores = true;
	DeathmatchScoreboard (ent);
}

/*
==================
HelpComputer

Draw help computer.
==================
*/
void HelpComputer (edict_t *ent)
{
	char	string[1024];
	char	*sk;

	if (skill->value == 0)
		sk = "easy";
	else if (skill->value == 1)
		sk = "medium";
	else if (skill->value == 2)
		sk = "hard";
	else
		sk = "hard+";

	// send the layout
	Com_sprintf (string, sizeof(string),
		"xv 32 yv 8 picn help "			// background
		"xv 202 yv 12 string2 \"%s\" "		// skill
		"xv 0 yv 24 cstring2 \"%s\" "		// level name
		"xv 0 yv 54 cstring2 \"%s\" "		// help 1
		"xv 0 yv 110 cstring2 \"%s\" "		// help 2
		"xv 50 yv 164 string2 \" kills     goals    secrets\" "
		"xv 50 yv 172 string2 \"%3i/%3i     %i/%i       %i/%i\" ", 
		sk,
		level.level_name,
		game.helpmessage1,
		game.helpmessage2,
		level.killed_monsters, level.total_monsters, 
		level.found_goals, level.total_goals,
		level.found_secrets, level.total_secrets);

	gi.WriteByte (svc_layout);
	gi.WriteString (string);
	gi.unicast (ent, true);
}


/*
==================
Cmd_Help_f

Display the current help message
==================
*/
void Cmd_Help_f (edict_t *ent)
{
	// this is for backwards compatibility
	if (deathmatch->value)
	{
		Cmd_Score_f (ent);
		return;
	}

	ent->client->showinventory = false;
	ent->client->showscores = false;

	if (ent->client->showhelp && 
		(ent->client->resp.game_helpchanged == game.helpchanged))
	{
		ent->client->showhelp = false;
		return;
	}

	ent->client->showhelp = true;
	ent->client->resp.helpchanged = 0;
	HelpComputer (ent);
}


//=======================================================================

/*
===============
G_SetStats
===============
*/
void G_SetStats (edict_t *ent)
{
	gitem_t		*item;
	int			index, cells = 0;
	int			power_armor_type;
	int i;

	// Make sure ent exists!
	if (!G_EntExists(ent)) 
		return;
	
	CalcFPH(ent);	//QW// Refactored
	CalcFPM(ent);	//QW//

	//
	// health
	//
	if(!voosh->value)
	{
		ent->client->ps.stats[STAT_HEALTH_ICON] = level.pic_health;
		ent->client->ps.stats[STAT_HEALTH] = ent->health;


		//
		// ammo
		//
		if (!ent->client->ammo_index)
		{
			ent->client->ps.stats[STAT_AMMO_ICON] = 0;
			ent->client->ps.stats[STAT_AMMO] = 0;
		}
		else
		{
			item = &itemlist[ent->client->ammo_index];
			ent->client->ps.stats[STAT_AMMO_ICON] = gi.imageindex (item->icon);
			ent->client->ps.stats[STAT_AMMO] = ent->client->pers.inventory[ent->client->ammo_index];
		}

		//
		// armor
		//
		power_armor_type = PowerArmorType (ent);
		if (power_armor_type)
		{
			cells = ent->client->pers.inventory[ITEM_INDEX(FindItem ("cells"))];
			if (cells == 0)
			{	// ran out of cells for power armor
				ent->flags &= ~FL_POWER_ARMOR;
				gi.sound(ent, CHAN_ITEM, gi.soundindex("misc/power2.wav"), 1, ATTN_NORM, 0);
				power_armor_type = 0;;
			}
		}

		index = ArmorIndex (ent);

		if (power_armor_type && (!index || (level.framenum & 8) ) )
		{	// flash between power armor and other armor icon
			ent->client->ps.stats[STAT_ARMOR_ICON] = gi.imageindex ("i_powershield");
			ent->client->ps.stats[STAT_ARMOR] = cells;
		}
		else if (index)
		{
			item = GetItemByIndex (index);
			ent->client->ps.stats[STAT_ARMOR_ICON] = gi.imageindex (item->icon);
			ent->client->ps.stats[STAT_ARMOR] = ent->client->pers.inventory[index];
		}
		else
		{
			ent->client->ps.stats[STAT_ARMOR_ICON] = 0;
			ent->client->ps.stats[STAT_ARMOR] = 0;
		}
	}
	//
	// pickup message
	//
	if (level.time > ent->client->pickup_msg_time)
	{
		ent->client->ps.stats[STAT_PICKUP_ICON] = 0;
		ent->client->ps.stats[STAT_PICKUP_STRING] = 0;
	}

	//
	// timers
	//
	if (ent->client->quad_framenum > level.framenum)
	{
		ent->client->ps.stats[STAT_TIMER_ICON] = gi.imageindex ("p_quad");
		ent->client->ps.stats[STAT_TIMER] = (ent->client->quad_framenum - level.framenum)/10;
	}
	else if (ent->client->invincible_framenum > level.framenum)
	{
		ent->client->ps.stats[STAT_TIMER_ICON] = gi.imageindex ("p_invulnerability");
		ent->client->ps.stats[STAT_TIMER] = (ent->client->invincible_framenum - level.framenum)/10;
	}
	//RAV
	//respawn protection
	else if (ent->client->respawn_framenum > level.framenum)
	{
		ent->client->ps.stats[STAT_TIMER_ICON] = gi.imageindex ("p_invulnerability");
		ent->client->ps.stats[STAT_TIMER] = (ent->client->respawn_framenum - level.framenum)/10;
	}
	//
	else if (ent->client->enviro_framenum > level.framenum)
	{
		ent->client->ps.stats[STAT_TIMER_ICON] = gi.imageindex ("p_envirosuit");
		ent->client->ps.stats[STAT_TIMER] = (ent->client->enviro_framenum - level.framenum)/10;
	}
	else if (ent->client->breather_framenum > level.framenum)
	{
		ent->client->ps.stats[STAT_TIMER_ICON] = gi.imageindex ("p_rebreather");
		ent->client->ps.stats[STAT_TIMER] = (ent->client->breather_framenum - level.framenum)/10;
	}
	else
	{
		ent->client->ps.stats[STAT_TIMER_ICON] = 0;
		ent->client->ps.stats[STAT_TIMER] = 0;
	}

	//
	// selected item
	//
	if (ent->client->pers.selected_item == -1)
		ent->client->ps.stats[STAT_SELECTED_ICON] = 0;
	else
		ent->client->ps.stats[STAT_SELECTED_ICON] = gi.imageindex (itemlist[ent->client->pers.selected_item].icon);

	ent->client->ps.stats[STAT_SELECTED_ITEM] = ent->client->pers.selected_item;

	//RAV
	ent->client->ps.stats[STAT_RUNE] = 0;
	for (i = RUNE_FIRST; i <= RUNE_LAST; i++) 
	{
		if (rune_has_rune(ent, i))
			ent->client->ps.stats[STAT_RUNE] = CS_ITEMS + ITEM_INDEX(FindItem(rune_namefornum[i]));
	}
	//
	//
	// layouts
	//
	ent->client->ps.stats[STAT_LAYOUTS] = 0;

	if (deathmatch->value)
	{
		if (ent->client->pers.health <= 0 || level.intermissiontime || ent->client->showhelp
			|| ent->client->showscores || ent->client->pers.db_hud || ent->client->pers.motd )//RAV
		{
			ent->client->ps.stats[STAT_LAYOUTS] |= 1;
		}
		if (ent->client->showinventory && ent->client->pers.health > 0)
		{
			ent->client->ps.stats[STAT_LAYOUTS] |= 2;
		}
	}
	else
	{
		if (ent->client->showscores || ent->client->showhelp || ent->client->pers.db_hud || ent->client->pers.motd )//RAV
		{
			ent->client->ps.stats[STAT_LAYOUTS] |= 1;
		}
		if (ent->client->showinventory && ent->client->pers.health > 0)
		{
			ent->client->ps.stats[STAT_LAYOUTS] |= 2;
		}
	}

	// frags
	//
	ent->client->ps.stats[STAT_FRAGS] = ent->client->resp.score;

	//
	// help icon / current weapon if not shown
	//
	if (ent->client->resp.helpchanged && (level.framenum&8) )
		ent->client->ps.stats[STAT_HELPICON] = gi.imageindex ("i_help");
	else if ( (ent->client->pers.hand == CENTER_HANDED || ent->client->ps.fov > 91)
		&& ent->client->pers.weapon)
		ent->client->ps.stats[STAT_HELPICON] = gi.imageindex (ent->client->pers.weapon->icon);
	else
		ent->client->ps.stats[STAT_HELPICON] = 0;

	//	gi.dprintf("pers.weapon is %s\n", ent->client->pers.weapon->classname);

	//ZOID
	if (ctf->value)
		SetCTFStats(ent);
	//ZOID
}

