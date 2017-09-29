#ifndef LOG_MANAGER_H
#define LOG_MANAGER_H

extern cvar_t *logfile;         // the logfile mode control
extern cvar_t *logfile_name;    // the server log file
extern cvar_t *logfile_rename;  // how often we rename the server log file
extern cvar_t *logfile_tmp;     // temp storage of logfile mode

void Log_Init_vars (void);
int Log_CheckLocalMidnight(void);
void Log_RenameConsoleLog(void);

#endif
