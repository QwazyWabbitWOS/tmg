
#include "g_local.h"
#include "performance.h"
#include "g_items.h"
#include "anticheat.h"
#include "g_cmds.h"
#include "s_map.h"
#include "p_client.h"
#include "vote.h"
#include "e_hook.h"
#include "timer.h"
#include "p_hud.h"
#include "bot.h"
#include "runes.h"
#include "highscore.h"
#include "hud.h"
#include "maplist.h"
#include "log_manager.h"
#include "statslog.h"

game_locals_t	game;
level_locals_t	level;
game_import_t	gi;
game_export_t	globals;
spawn_temp_t	st;

int	sm_meat_index;
int	snd_fry;
int meansOfDeath;
int dmflag=0;	//JSW

edict_t		*g_edicts;

/* console variables */
cvar_t	*deathmatch;
cvar_t	*coop;
cvar_t	*dmflags;
cvar_t	*skill;
cvar_t	*fraglimit;
cvar_t	*timelimit;
cvar_t	*scoreboardtime;
cvar_t	*cycle_always;
//ZOID
cvar_t	*capturelimit;
//ZOID
cvar_t	*passwd;
cvar_t	*maxclients;
cvar_t	*maxentities;
cvar_t	*g_select_empty;

cvar_t	*dedicated;

cvar_t	*sv_maxvelocity;
cvar_t	*sv_gravity;

cvar_t	*sv_rollspeed;
cvar_t	*sv_rollangle;
cvar_t	*gun_x;
cvar_t	*gun_y;
cvar_t	*gun_z;

cvar_t	*run_pitch;
cvar_t	*run_roll;
cvar_t	*bob_up;
cvar_t	*bob_pitch;
cvar_t	*bob_roll;

cvar_t	*sv_cheats;
cvar_t	*view_weapons;       // control weapon visibility

//RAV
cvar_t	*use_hook;
cvar_t	*hook_speed;         // sets how fast the hook shoots out
cvar_t	*hook_pullspeed;     // sets how fast the hook pulls a player
cvar_t	*hook_sky;           // enables hooking to the sky
cvar_t	*hook_maxtime;       // sets max time you can stay hooked
cvar_t	*hook_damage;        // sets damage hook does to other players
cvar_t	*hook_reset;         // 1 = hook will not inflict damage on opponents
cvar_t  *hook_color;         // global selection of hook color in DM
cvar_t  *hook_offhand;       //QW// UNUSED


cvar_t	*game_dir;

cvar_t *sv_botdetection;
cvar_t  *server_ip;

cvar_t  *warmup_time;
cvar_t  *g_filter;

cvar_t  *flood_msgs;
cvar_t  *flood_persecond;
cvar_t  *flood_waitdelay;
cvar_t	*flood_team;

cvar_t  *maxfps;
cvar_t  *maxrate;

cvar_t  *voosh;

cvar_t  *max_hud_dist;
cvar_t  *light_comp;
cvar_t  *id_x;
cvar_t  *id_y;
cvar_t  *hud_freq;
cvar_t  *motd_time;

cvar_t  *hostname;


cvar_t  *speed_msec;
cvar_t  *speed_set;
cvar_t  *speed_bust;


cvar_t  *railwait;
cvar_t  *raildamage;
cvar_t  *railkick;

cvar_t  *flashlight;
cvar_t  *lights;//do not document this one
cvar_t  *lights_out;

cvar_t	*mapvote;

cvar_t *rcon;

cvar_t  *banned_items;
cvar_t  *weapflags;
cvar_t  *ammoflags;
cvar_t  *ban_blaster;
cvar_t  *start_weapons;
cvar_t  *start_items;
cvar_t  *armor_spawn;
cvar_t  *ammo_spawn;
cvar_t  *mega_health_spawn;
cvar_t  *health_spawn;
cvar_t  *spawn_time;
cvar_t	*runes;
cvar_t  *runeflags;
cvar_t  *max_Vhealth;
cvar_t  *pickup_tech;
cvar_t  *hide_spawn;
cvar_t  *camper_check;
cvar_t  *camp_time;
cvar_t  *camp_distance;
cvar_t  *resp_protect;
cvar_t  *motd_line;
cvar_t	*prox;
cvar_t  *check_models;

cvar_t	*lag_limit;
cvar_t  *menutime;
cvar_t  *clear_teams;
cvar_t  *even_teams;
cvar_t  *reserved_slots;
cvar_t	*reserved_password;
cvar_t	*max_specs;
cvar_t	*op_specs;
cvar_t  *speed_check;

//DM ADDITIONS
cvar_t	*sa_shells;
cvar_t	*sa_bullets;
cvar_t	*sa_grenades;
cvar_t	*sa_rockets;
cvar_t	*sa_cells;
cvar_t	*sa_slugs;

cvar_t  *sg_damage;
cvar_t  *sg_kick;
cvar_t  *ssg_damage;
cvar_t  *ssg_kick;
cvar_t  *rg_damage;
cvar_t  *rg_kick;
cvar_t  *bfg_damage;
cvar_t  *g_damage;
cvar_t  *gl_damage;
cvar_t  *rl_damage;
cvar_t  *b_damage;
cvar_t  *hb_damage;
cvar_t  *mg_damage;
cvar_t  *mg_kick;
cvar_t  *cg_damage;
cvar_t  *cg_kick;

cvar_t  *rocket_wait;
cvar_t  *railgun_wait;
cvar_t  *shotgun_wait;
cvar_t  *sshotgun_wait;
cvar_t  *bfg_wait;
cvar_t  *rocket_speed;
cvar_t  *rl_radius_damage;
cvar_t  *rl_radius;
cvar_t  *lag_ping;
//ctf scoring
cvar_t  *cap_point;
cvar_t  *cap_team;
cvar_t  *recover_flag;
cvar_t  *flag_bonus;
cvar_t  *frag_carrier;
cvar_t  *carrier_save;
cvar_t  *carrier_protect;
cvar_t  *flag_defend;
cvar_t  *flag_assist;
cvar_t  *frag_carrier_assist;
cvar_t  *flag_return_time;
//
cvar_t  *server_time;
cvar_t  *show_carrier;

cvar_t  *clan_name;
cvar_t  *clan_pass;
cvar_t  *cl_check;
//

cvar_t	*ctf;
cvar_t	*ctf_forcejoin;

//ponpoko
cvar_t	*chedit;
cvar_t	*vwep;
cvar_t	*botlist;
cvar_t	*autospawn;
cvar_t	*zigmode;
float	spawncycle;
cvar_t	*use_navfiles;
float	ctfjob_update;

cvar_t  *spec_check;
cvar_t	*lan;
cvar_t  *damage_locate;
cvar_t  *damage_display;

//JSW
cvar_t	*nokill;
cvar_t	*punish_suicide;
cvar_t	*console_chat;
cvar_t	*no_hum;
cvar_t	*developer;
cvar_t	*gamedebug;
qboolean	serverlocked;
cvar_t	*basedir;
cvar_t	*hook_carrierspeed;
cvar_t	*mercylimit;
cvar_t	*randomrcon;
cvar_t	*defaultoplevel;
cvar_t	*minfps;
cvar_t	*use_grapple;
cvar_t  *log_chat;
cvar_t	*log_connect;
cvar_t	*grapple_speed;
cvar_t	*grapple_pullspeed;
cvar_t	*grapple_damage;

/* Debugging options */
cvar_t	*debug_spawn;
cvar_t	*debug_bots;
cvar_t	*debug_botspawn;
cvar_t	*debug_ops;


cvar_t	*runes4all;
cvar_t	*quad_notify;
cvar_t	*teamtechs;
cvar_t	*auto_flag_return;
cvar_t	*tmgclock;
cvar_t	*allow_flagdrop;
cvar_t	*extrasounds;
cvar_t	*allow_vote;
cvar_t	*vote_timeout;
cvar_t	*vote_percentage;
cvar_t	*cfgdir;
cvar_t	*modversion;
cvar_t	*doors_stay_open;
//end

//===================================================================
void ShutdownGame (void)
{
	int i;
	edict_t	*ent;
	gi.dprintf ("==== ShutdownGame (TMG "MOD_VERSION") ====\n");

	//QW// logging of hook stats per player
	if (g_edicts != NULL) // in case we bomb before they're allocated
	{
		for (i = 0; i < maxclients->value; i++)
		{
			ent = g_edicts + 1 + i;
			if (ent->inuse && !ent->bot_client)
				StatsLogHooks(ent);
		}
	}

	if (statsfile) {// guard against a failed initialization
		StatsLog("DOWN: %s\\%.1f\n", level.mapname, level.time);
		StatsLogFlush();
	}

	if (mdsoft_map)
		free(mdsoft_map);

	gi.FreeTags (TAG_LEVEL);
	gi.FreeTags (TAG_GAME);

#ifdef _WIN32
	OutputDebugString("ShutdownGame() was called.\n");
	OutputDebugString("Dump objects since startup.\n");
	_CrtMemDumpAllObjectsSince(&startup1);
	OutputDebugString("Memory stats since startup.\n");
	_CrtMemDumpStatistics (&startup1);
	_CrtDumpMemoryLeaks();
#endif
}

/*
=================
GetGameAPI

Returns a pointer to the structure with all entry points
and global variables
=================
*/
game_export_t *GetGameAPI (game_import_t *import)
{
	gi = *import;

	globals.apiversion = GAME_API_VERSION;
	globals.Init = InitGame;
	globals.Shutdown = ShutdownGame;
	globals.SpawnEntities = SpawnEntities;

	globals.WriteGame = WriteGame;
	globals.ReadGame = ReadGame;
	globals.WriteLevel = WriteLevel;
	globals.ReadLevel = ReadLevel;

	globals.ClientThink = ClientThink;
	globals.ClientConnect = ClientConnect;
	globals.ClientUserinfoChanged = ClientUserinfoChanged;
	globals.ClientDisconnect = ClientDisconnect;
	globals.ClientBegin = ClientBegin;
	globals.ClientCommand = ClientCommand;

	globals.RunFrame = G_RunFrame;

	globals.ServerCommand = ServerCommand;

	globals.edict_size = sizeof(edict_t);

	return &globals;
}

#ifndef GAME_HARD_LINKED

// this is only here so the functions in q_shared.c and q_shwin.c can link
void Sys_Error (char *error, ...)
{
	va_list		argptr;
	char		text[1024];

	va_start (argptr, error);
	vsprintf (text, error, argptr);
	va_end (argptr);

	AddLogEntry ("debugger.txt", text);
	
	//gi.dprintf ("DEBUGGER! RaV this is the error %s", text);
	
	gi.error (ERR_FATAL, "%s", text);
}

void Com_Printf (char *msg, ...)
{
	va_list		argptr;
	char		text[1024];

	va_start (argptr, msg);
	vsprintf (text, msg, argptr);
	va_end (argptr);

	gi.dprintf ("%s", text);
}

#endif

//======================================================================


/*
=================
ClientEndServerFrames
=================
*/
void ClientEndServerFrames (void)
{
	int		i;
	edict_t	*ent;

	// calc the player views now that all pushing
	// and damage has been added
	for (i = 0 ; i < maxclients->value ; i++)
	{
		ent = g_edicts + 1 + i;
		if (ent && (!ent->inuse || !ent->client))
			continue;

		// Safety check...
		if (!G_EntExists(ent))
			continue;

		if(ent && !(ent->svflags & SVF_MONSTER))
			ClientEndServerFrame (ent);
	}
}

/*
Returns the created target changelevel
================ =
*/
edict_t * CreateTargetChangeLevel(char* map)
{
	edict_t* ent;

	ent = G_Spawn();
	ent->classname = "target_changelevel";
	Com_sprintf(level.nextmap, sizeof(level.nextmap), "%s", map);
	ent->map = level.nextmap;
	return ent;
}

/*
=================
The timelimit or fraglimit has been exceeded
=================
*/
void EndDMLevel (void)
{
	edict_t		*ent = NULL;
	int	votedmap = NO_MAPVOTES;

	// stay on same level flag
	if (dmflag & DF_SAME_LEVEL)
		BeginIntermission(CreateTargetChangeLevel(level.mapname));

	if (maplist->nextmap > -1)
	{
		ent = G_Spawn ();
		ent->classname = "target_changelevel";
		ent->map = maplist->mapname[maplist->nextmap];

		if (Maplist_CheckFileExists(ent->map))
			BeginIntermission (ent);
		else
		{
			gi.bprintf(PRINT_CHAT,"Map %s does not exist on server, reverting to last map.\n", ent->map);
			ent->map = level.mapname;
			votetime = 0;
			BeginIntermission (ent);
		}
	}

	if (mapvote->value && !ent)
	{
		ent = G_Spawn ();
		ent->classname = "target_changelevel";
		votedmap = MaplistNextMap(ent);

		if (votedmap != NO_MAPVOTES && !Maplist_CheckFileExists(ent->map))
		{
			gi.bprintf(PRINT_CHAT,"Map %s does not exist on server, reverting to last map.\n", ent->map);
			ent->map = level.mapname;
			votetime = 0;
			BeginIntermission (ent);
		}

		if(mapscrewed)
			ent = NULL;

		votetime = 0;
		if (ent != NULL)
			BeginIntermission (ent);
	}

    /* New code - START - mdavies */
    if( !ent )
    {
        ent = mdsoft_NextMap();
		if (ent != NULL)
			BeginIntermission (ent);
    }
    /* New code - END - mdavies */

    if( !ent )
    {
        DbgPrintf("mdsoft_NextMap returned NULL!\n");
		if (level.nextmap[0])
        {   // go to a specific map
            ent = G_Spawn ();
            ent->classname = "target_changelevel";
            ent->map = level.nextmap;
			BeginIntermission (ent);
        }
        else
        {   // search for a changelevel
            ent = G_Find (NULL, FOFS(classname), "target_changelevel");
            if (!ent)
            {   // the map designer didn't include a changelevel,
                // so create a fake ent that goes back to the same level
                ent = G_Spawn ();
                ent->classname = "target_changelevel";
                ent->map = level.mapname;
				BeginIntermission (ent);
            }
        }
    }
	if(use_bots->value)
		Load_BotInfo();

	StatsLogPlayerStats();

}


//RAV
/**
End the level and go to selected map
*/
void OPEndDMLevel (int mapindex, edict_t *cl)
{
	edict_t		*ent;

	my_bprintf (PRINT_CHAT,
				"OP %s is switching to map '%s' - '%s'\n",
				cl->client->pers.netname,
				maplist->mapname[mapindex],
				maplist->mapnick[mapindex]);

	ent = G_Spawn ();
	ent->classname = "target_changelevel";
	ent->map = maplist->mapname[mapindex];
	BeginIntermission (ent);
}

/*
=================
CheckDMRules
=================
*/
void CheckDMRules (void)
{
	int			i;
	gclient_t	*cl;
	
	if (level.intermissiontime)
		return;

	if (!deathmatch->value)
		return;
	
	if(level.time < votetime)
		return;
		
	if (match_state == STATE_PLAYING && level.time >= match_state_end)
	{
		//if (developer->value)
		//	gi.dprintf("1 match_state_end = %f, level.time = %f, votetime = %f\n", 
		//			match_state_end, level.time, votetime);

		my_bprintf (PRINT_HIGH, "Timelimit hit.\n");
		//start voting
		if(mapvote->value && maplist->nextmap == -1)
		{
			votetime = level.time+(int)menutime->value;
		}
		else
			EndDMLevel ();
		return;
	}
	
	if (ctf->value)
	{
		if (CTFCheckRules())
		{
			//start voting
			if(mapvote->value && maplist->nextmap == -1)
			{
				my_bprintf(PRINT_CHAT, "Use menu to vote for next map!\n");
				FillMapNames();
				votetime = level.time+(int)menutime->value;
			}
			else
				EndDMLevel ();
			return;
		}
	}

	if (fraglimit->value)
	{
		for (i=0 ; i<maxclients->value ; i++)
		{
			cl = game.clients + i;
			if (!g_edicts[i+1].inuse)
				continue;
			if (cl->resp.score >= fraglimit->value)
			{
				my_bprintf (PRINT_HIGH, "Fraglimit hit.\n");
				//start voting
				if(mapvote->value && maplist->nextmap == -1)
				{
					my_bprintf(PRINT_CHAT, "Use menu to vote for next map!\n");
					FillMapNames();
					votetime = level.time+(int)menutime->value;
				}
				else
					EndDMLevel ();
				return;
			}
		}
	}
}

/*
=============
ExitLevel
=============
*/
void ExitLevel (void)
{
	int		i;
	edict_t	*ent;
	char	command [256];

	Com_sprintf (command, sizeof(command), "gamemap \"%s\"\n", level.changemap);

	gi.AddCommandString (command);
	level.changemap = NULL;
	level.exitintermission = 0;
	level.intermissiontime = 0;
	ClientEndServerFrames ();

	// clear some things before going to next level
	for (i = 0; i < maxclients->value; i++)
	{
		ent = g_edicts + 1 + i;
		if (ent && ent->inuse)
		{
			if (ent->health > ent->client->pers.max_health)
				ent->health = ent->client->pers.max_health;
		}
	}

	hs_show = true;

	SetBotFlag1(NULL);
	SetBotFlag2(NULL);

//ZOID
	if (ctf->value)
		CTFInit();
//ZOID
}

/*
================
G_RunFrame

Advances the world by 0.1 seconds
================
*/
void G_RunFrame (void)
{
	int		i;
	edict_t	*ent, *e;

	botdetection = (int) sv_botdetection->value;

	// every 10th frame do logfile management
	if (level.framenum % 10 == 0)
	{
		if (Log_CheckLocalMidnight())
		{
			Log_RenameConsoleLog();
			StatsLogRename();
		}
	}

	level.framenum++;
	level.time = level.framenum * FRAMETIME;
	level.newframenum++;

//	gi.dprintf ("L. map to be voted on is %s %s\n", maplist->mapname[maplist->currentmapvote]);
	
	// choose a client for monsters to target this frame
	//	AI_SetSightClient ();
	
	//RAV map voting
	if (level.intermissiontime && (level.intermissiontime < (level.time - scoreboardtime->value)))
	{
		if (CountConnectedClients() || cycle_always->value)
            level.exitintermission = true;
	}
	
	// exit intermissions
	if (level.exitintermission)
	{
		ExitLevel ();
		return;
	}
	//

	if(voosh->value == 0)
	{
		CTFSetupTechSpawn();
		if(runes->value)
			runes_spawn_start();
	}
	
	// Bot Spawning
	//
	if(SpawnWaitingBots && !level.intermissiontime && match_state == STATE_PLAYING )
	{
		if(spawncycle < level.time)
		{
			Bot_SpawnCall();
			spawncycle = level.time + FRAMETIME * 10 + 0.01 * SpawnWaitingBots;
		}
	}
	else
	{
		if(spawncycle < level.time)
			spawncycle = level.time + FRAMETIME * 10;
	}
	
	TimeLeft(); // Maintain the HUD match timer countdown

	//
	// treat each object in turn
	// even the world gets a chance to think
	//
	ent = &g_edicts[0];
	for (i=0 ; i<globals.num_edicts ; i++, ent++)
	{
		if (!ent->inuse)
			continue;
		level.current_entity = ent;
		VectorCopy (ent->s.origin, ent->s.old_origin);
		
		// if the ground entity moved, make sure we are still on it
		if ((ent->groundentity) && (ent->groundentity->linkcount != ent->groundentity_linkcount))
		{
			ent->groundentity = NULL;
			if ( !(ent->flags & (FL_SWIM|FL_FLY)) && (ent->svflags & SVF_MONSTER) )
			{
				M_CheckGround (ent);
			}
		}
		
		if(ctfjob_update < level.time)
		{
			//gi.bprintf(PRINT_HIGH,"Assigned!!!\n");
			CTFJobAssign();
			ctfjob_update = level.time + FRAMETIME * 2;
		}
		
		if (i > 0 && i <= maxclients->value && !(ent->svflags & SVF_MONSTER))
		{
			ClientBeginServerFrame (ent);
			continue;
		}
		G_RunEntity (ent);
	} // End world-think

	if(mapvote->value && maplist->nextmap == -1)
	{
		//if (ceil(match_state_end - level.time) < 11)
		//	DbgPrintf("5 match_state_end = %f, level.time = %f, votetime = %f\n", match_state_end, level.time, votetime);
		if((votetime > 0) && (level.time + 2 > votetime)) //check for votes and exit intermission
		{
			//DbgPrintf("4 match_state_end = %f, level.time = %f, votetime = %f\n", match_state_end, level.time, votetime);
			EndDMLevel ();
		}
	}

	// see if it is time to end a deathmatch
	CheckDMRules ();
	
	//RAV
	TimerThink ();

	if (level.intermissiontime)
	{ //set up highscores display
		//fix for highscores
		if(level.time == level.intermissiontime + scoreboardtime->value/2)
		{
			LoadHighScores();
		}
		if ((level.time >= level.intermissiontime + (scoreboardtime->value/2) + 0.1f) && (show_highscores->value))
		{
			for (i=0; i<=maxclients->value; i++)
			{
				e = &g_edicts[i];
				if (e && e->bot_client)
				{
					ClientDisconnect(e);
				}
			}

			for (i = 1; i <= maxclients->value; i++)
			{
				e = &g_edicts[i];
				if (e && e->inuse && !e->bot_client)
				{
					// display it for all to view!!
					StuffCmd(e, "cmd hscore\n");
				}
			}
		}
		if ((level.time >= level.intermissiontime + scoreboardtime->value - 4.0f) && (hs_show == true))
		{
			hs_show = false;
		}
	}

	if((mapvote->value) && 
		(level.time + menutime->value - 1.0f < votetime) && 
		!locked_teams && (maplist->nextmap == -1))
	{
		//gi.drintf("3 match_state_end = %f, level.time = %f, votetime = %f\n",
		//		  match_state_end, level.time, votetime);

		//match_state = STATE_VOTING;
		level.warmup = true;
		//locked_teams  = true;
//		my_bprintf (PRINT_HIGH, "teams locked\n");//JSWdebug
		for (i = 0; i < maxclients->value; i++)
		{
			// disconnect all bots if any
			ent = g_edicts + 1 + i;
			if (ent && ent->bot_client)
			{
				ClientDisconnect(ent);
			}

			if (ent && !ent->inuse)
				continue;

			if (ent && ent->client)
			{
				ent->client->pers.vote_times = 0;// set all voting to 0
				ent->client->newweapon = NULL;

				ChangeWeapon (ent);
				ent->client->ps.gunindex = 0;
				if(ent->client->hook)
					abandon_hook_reset(ent->client->hook);
				runes_drop(ent);
				if ( ent->flashlight )
				{
					G_FreeEdict(ent->flashlight);
					ent->flashlight = NULL;
				}

				if(ctf->value)
				{
					CTFPlayerResetGrapple(ent);
					CTFDeadDropFlag(ent);
					CTFDeadDropTech(ent);
				}
				//bring up menu for voting
				if(ent->client->pers.pl_state == PL_PLAYING)
					ent->client->pers.pl_state = PL_WARMUP;
				if(!ent->bot_client)
					StuffCmd (ent, va("map_vote\n"));
			}
		}
	}

	// build the playerstate_t structures for all players
	ClientEndServerFrames ();
}

