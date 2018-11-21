//
// Statistics logging functions for TMG_MOD
//
#include "g_local.h"
#include "statslog.h"
#include "maplist.h"	// for gamedir

cvar_t	*statslog;			// Enable player stats summary to stats log (default = 1)
cvar_t	*statsfile;			// the name of the stats log file ("stats.log" by default)
cvar_t	*statsfile_rename;	// how often we rename the file
cvar_t	*statslog_tmp;		// temp storage for statslog value during rename

void StatsInitVars(void)
{
	//cvars pertaining to stats logs
	statslog = gi.cvar ("statslog", "1", 0);		//QW// allow logging player statistics
	statsfile = gi.cvar ("statsfile", "stats.log", 0); //QW// the name of the stats file
	statsfile_rename = gi.cvar ("statsfile_rename", "1", 0); //QW// 0 = never, 1 = daily, 2 = weekly, 3 = monthly
	statslog_tmp = gi.cvar ("statslog_tmp", "0", 0);
}

// Output the statistics strings to a file in the gamedir
// Inputs: 
//			pointer to the string to be logged.
//
void StatsLog(const char *fmt, ... ) 
{ 

	va_list	argptr; 
	char	ar_string[1024]; 
	char	ar_tmp[1024]; 
	time_t	long_time;
	struct	tm	*ar_st; 
	char	logpath [MAX_QPATH];

	va_start(argptr, fmt); 
	vsprintf(ar_tmp, fmt, argptr); 
	va_end(argptr); 

	time(&long_time);
	ar_st = localtime(&long_time); 

	Com_sprintf( ar_string, sizeof(ar_string), "%02i/%02i/%02i %02i:%02i:%02i : %s", 
		ar_st->tm_mon + 1, 
		ar_st->tm_mday, 
		ar_st->tm_year + 1900, 
		ar_st->tm_hour, 
		ar_st->tm_min, 
		ar_st->tm_sec, 
		ar_tmp); 

	Com_sprintf(logpath, sizeof logpath, "%s/stats/%s", game_dir->string, statsfile->string);

	// if not open and we want logging, open the log
	if (statslog->value)
	{
		if (!pStatsfile) 
		{
			pStatsfile = fopen(logpath, "a");
			if(pStatsfile) 
			{
				fprintf(pStatsfile, "%s", ar_string); 
			}
			else
				gi.dprintf("Error writing to %s\n", logpath);
		}
		else
		{
			fprintf(pStatsfile, "%s", ar_string);
		}
	}
	else // not logging, flush and close
	{
		if (pStatsfile)
		{
			fflush(pStatsfile);
			fclose(pStatsfile);
			pStatsfile = NULL;
		}
	}
} 

// this renames the stats file from lox/stats.log (or whatever)
// to lox/stats/YYYYMMDD-stats.log. The lox/stats/ folder MUST exist.
void StatsLogRename(void)
{
	char	oldname[MAX_QPATH];
	char	newname[MAX_QPATH];
	time_t	long_time;
	struct	tm	*ltime; 

	time(&long_time);
	ltime = localtime(&long_time); 

	if ((statsfile_rename->value == 1 && statslog->value == 1)	// every day
		|| (statsfile_rename->value == 2 && ltime->tm_wday == 0)	// sundays
		|| (statsfile_rename->value == 3 && ltime->tm_mday == 1))	// 1st of month
	{
		gi.dprintf("Backing up statsfile\n");
		gi.cvar_set("statslog_tmp", statslog->string);	// save current logging mode
		gi.cvar_forceset("statslog", "0");	// turn off logging
		StatsLog("BACK: %s\\%.1f\n", level.mapname, level.time);	// post message to force closure

		Com_sprintf(oldname, sizeof oldname, "%s/stats/%s", 
			game_dir->string, statsfile->string);
		Com_sprintf(newname, sizeof newname, "%s/logs/%02i%02i%02i-%s", 
			game_dir->string,
			ltime->tm_year + 1900, 
			ltime->tm_mon + 1, 
			ltime->tm_mday, 
			statsfile->string); 

		if (rename(oldname, newname))
		{
			if(errno == EACCES)
			{
				gi.dprintf("Error renaming %s to %s, make sure target folder exists.\n", oldname, newname);
			}
			else if (errno == ENOENT)
			{
				gi.dprintf("Error renaming %s, it doesn't exist.\n", oldname);
			}
		}
		gi.cvar_forceset("statslog", statslog_tmp->string);	// restore previous mode
		gi.dprintf("Statsfile backup complete.\n");
		StatsLog("CONT: %s\\%.1f\n", level.mapname, level.time);
		if(ctf->value)
			StatsLog("MODE: %s %s\n", "TMG_MOD CTF", MOD_VERSION );
		else
			StatsLog("MODE: %s %s\n", "TMG_MOD Deathmatch", MOD_VERSION );
	}
}

/**
 Force flush the log file if it's open.
 */
void StatsLogFlush(void)
{
	if (pStatsfile && statslog->value)
		fflush(pStatsfile);
}

/*
QwazyWabbit added 09/16/2016.
This function writes backslash-delimited hook stats to the log file.
For performance reasons, counters are incremented in the hook_think function 
rather than calling a logging function every time a player launches a hook. 
Calling this function in BeginIntermission grabs the counter values before
they're cleared by PutClientInServer on the next level change.
*/
void StatsLogHooks(edict_t *ent)
{
	StatsLog("HOOKS: %s\\%ld\\%ld\\\\%s\\%.1f\n", 
		ent->client->pers.netname,
		ent->client->resp.hooks_landed_count,
		ent->client->resp.hooks_deployed_count,
		level.mapname, level.time);
	
	StatsLogFlush();
}

/**
Output stats to log if one is open
flush it at this time.
Called by EndDMLevel.
*/
void StatsLogPlayerStats(void)
{
	edict_t	*player;
	int i;

	player = g_edicts;
	for (i = 0; i < game.maxclients; i++)
	{
		player++;
		if (player->inuse)
			StatsLog("STAT: %s\\%d\\%d\\%d\\%d\\%d\\%d\\%s\\%.1f\n",
			player->client->pers.netname,
			player->client->resp.score,
			player->client->resp.eff,
			player->client->resp.captures,
			player->client->resp.deaths,
			player->client->resp.fph,
			player->client->resp.ctf_team,
			level.mapname,
			level.time);
	}
	StatsLogFlush();
}