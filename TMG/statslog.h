//
// Statistics logging functions for TMG_MOD
//

#ifndef STATSLOG_H
#define STATSLOG_H

extern cvar_t	*statslog;			// Enable player stats summary to qconsole.log (default = 1)
extern cvar_t	*statsfile;			// the name of the stats log file ("stats.log")
extern cvar_t	*statsfile_rename;	// how often we rename the file

// global pointer to the stats file
static FILE	*pStatsfile = NULL; 

void StatsLog(const char *fmt, ... ) ;
void StatsLogRename(void);
void StatsInitVars(void);
void StatsLogHooks(edict_t *ent);
void StatsLogPlayerStats(void);

#endif //STATSLOG_H
