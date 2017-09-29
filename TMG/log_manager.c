#include "g_local.h"
#include "log_manager.h"
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

cvar_t	*logfile;           // the logfile mode control
cvar_t	*logfile_name;      // the server log file
cvar_t	*logfile_rename;    // how often we rename the server log file
cvar_t	*logfile_tmp;       // temp storage of logfile mode

/**
Initialize the default values for the cvars we use.
 logfile_name is the name of the server log file (clients use qconsole.log)
 this variable is owned by server engine, so it's read-only to us.
 logfile_rename is the frequency of renaming:
 0 = never, 1 = daily, 2 = weekly, 3 = monthly
 default is weekly.
*/
void Log_Init_vars (void)
{
	logfile_name = gi.cvar ("logfile_name", "server.log", CVAR_NOSET);
	logfile_rename = gi.cvar ("logfile_rename", "1", 0);
	logfile_tmp = gi.cvar ("logfile_tmp", "0", 0);
}

// Call this once per second or once per minute
// from within G_RunFrame.
// Check local time and return true if it's midnight
int	Log_CheckLocalMidnight(void)
{
    time_t	long_time;
	struct	tm	*ltime; 
	
	time(&long_time);
	ltime = localtime(&long_time); 
	
	if (ltime->tm_hour == 0 && ltime->tm_min == 0 && ltime->tm_sec == 0)
		return true;
	else 
		return false;
}

//QW// rename server.log once per day | week | month
// depending on value of logfile_rename, default is daily.
void Log_RenameConsoleLog(void)
{
	char	logpath [MAX_QPATH], newname[MAX_QPATH];
    time_t	long_time;
	struct	tm	*ltime; 
	
	time(&long_time);
	ltime = localtime(&long_time);
	
	if ((logfile_rename->value == 1)	// every day
		|| (logfile_rename->value == 2 && ltime->tm_wday == 0)	// sundays
		|| (logfile_rename->value == 3 && ltime->tm_mday == 1))	// 1st of month
	{
		logfile = gi.cvar("logfile", "", CVAR_NOSET);	//expose the variables
		logfile_name = gi.cvar("logfile_name", "", CVAR_NOSET);
		gi.cvar_set("logfile_tmp", logfile->string);			// save current mode
		gi.cvar_forceset("logfile", "0");	// turn off logging
		gi.dprintf("Backing up logfile\n");	// post a message to force log closure
		Com_sprintf(logpath, sizeof logpath, "%s/%s", game_dir->string, logfile_name->string);
		Com_sprintf(newname, sizeof newname, "%s/logs/%02i%02i%02i-%s", 
			game_dir->string,
			ltime->tm_year + 1900, 
			ltime->tm_mon + 1, 
			ltime->tm_mday, 
			logfile_name->string); 
		
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
		
		gi.cvar_forceset("logfile", logfile_tmp->string);	// restore previous mode
		gi.dprintf("Logfile backup complete.\n"); //announce and reopen
	}
}
