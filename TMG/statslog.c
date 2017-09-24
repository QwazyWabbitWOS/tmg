//
// Statistics logging functions for TMG_MOD
//
#include "g_local.h"
#include "statslog.h"
#include "maplist.h"	// for gamedir
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

cvar_t	*statslog;			// Enable player stats summary to qconsole.log (default = 1)
cvar_t	*statsfile;			// the name of the stats log file ("stats.log" by default)
cvar_t	*statsfile_rename;	// how often we rename the file
cvar_t	*logfile_name;		// the server or qconsole.log file
cvar_t	*logfile;			// the logfile mode control
cvar_t	*logfile_rename;	// how often we rename the server log file

void StatsInitVars(void)
{
	//cvars pertaining to stats logs
	statslog = gi.cvar ("statslog", "1", 0);		//QW// allow logging player statistics
	statsfile = gi.cvar ("statsfile", "stats.log", 0); //QW// the name of the stats file
	statsfile_rename = gi.cvar ("statsfile_rename", "1", 0); //QW// 0 = never, 1 = daily, 2 = weekly, 3 = monthly
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

	//START_PERFORMANCE_TIMER;

	if (statslog->value == 0)
	{
		if (pStatsfile)
			fclose(pStatsfile);
		return;
	}

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

	if (!pStatsfile) 
	{
		if((pStatsfile = fopen(logpath, "a")) != NULL) 
		{
			fprintf(pStatsfile, "%s", ar_string); 
		}
		else
			gi.dprintf("Error writing to %s\n", statsfile->string);
	}
	else
		fprintf(pStatsfile, "%s", ar_string);

	//OutputDebugString(ar_string);
	//_STOP_PERFORMANCE_TIMER(__func__);
} 

// this renames the stats file from lox/stats.log (or whatever)
// to lox/stats/YYYYMMDD-stats.log. The lox/stats/ folder MUST exist.
void StatsLogRename(void)
{
	char	logpath[MAX_QPATH];
	char	newname[MAX_QPATH];
	time_t	long_time;
	struct	tm	*ltime; 

	time(&long_time);
	ltime = localtime(&long_time); 

	if ((statsfile_rename->value == 1)	// every day
		|| (statsfile_rename->value == 2 && ltime->tm_wday == 0)	// sundays
		|| (statsfile_rename->value == 3 && ltime->tm_mday == 1))	// 1st of month
	{
		Com_sprintf(logpath, sizeof logpath, "%s/%s", gamedir->string, statsfile->string);
		Com_sprintf(newname, sizeof newname, "%s/stats/%02i%02i%02i-%s", 
			gamedir->string,
			ltime->tm_year + 1900, 
			ltime->tm_mon + 1, 
			ltime->tm_mday, 
			statsfile->string); 

		if (pStatsfile)
		{
			fflush(pStatsfile);
			fclose(pStatsfile);
		}

		if (rename(logpath, newname))
		{
			if(errno == EACCES)
			{
				gi.dprintf("Error renaming %s to %s, make sure target folder exists.\n", logpath, newname);
			}
			else if (errno == ENOENT)
			{
				gi.dprintf("Error renaming %s, it doesn't exist.\n", logpath);
			}
		}
	}
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
	StatsLog("HOOKS: %s\\%ld\\%ld\\%.1f\n", 
		ent->client->pers.netname,
		ent->client->resp.hooks_landed_count,
		ent->client->resp.hooks_deployed_count,
		level.time);
}

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
}