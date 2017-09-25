//
// Statistics logging functions for TMG_MOD
//

#ifndef STATSLOG_H
#define STATSLOG_H

extern	cvar_t	*statslog;			// Enable player stats summary to stats log (default = 1)
extern	cvar_t	*statsfile;			// the name of the stats log file ("stats.log" by default)
extern	cvar_t	*statsfile_rename;	// how often we rename the file
extern	cvar_t	*statslog_tmp;		// temp storage for statslog value during rename

// global pointer to the stats file
static FILE	*pStatsfile = NULL; 

void StatsLog(const char *fmt, ... ) ;
void StatsLogRename(void);
void StatsInitVars(void);
void StatsLogHooks(edict_t *ent);
void StatsLogPlayerStats(void);

#endif //STATSLOG_H
