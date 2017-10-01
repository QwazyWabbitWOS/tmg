#ifndef PERFORMANCE_H
#define PERFORMANCE_H

// **********************************************
//        Windows high performance counter
// **********************************************

#ifdef _WIN32
#ifdef _DEBUG
void _STOP_PERFORMANCE_TIMER (char* str);
void _START_PERFORMANCE_TIMER (void);
#define START_PERFORMANCE_TIMER _START_PERFORMANCE_TIMER()
#define STOP_PERFORMANCE_TIMER _STOP_PERFORMANCE_TIMER(char* str)
#else
#define START_PERFORMANCE_TIMER
#define STOP_PERFORMANCE_TIMER
#endif
#endif

/**
 * Send message to debug console on Windows
 * or game server console via gi.dprintf on Linux.
 */
void DbgPrintf (char *msg, ...);

/* Select debug features */
/* 
Turn these on to debug areas of interest.
*/
#define DEBUG_HUD 0
#define DEBUG_PHYSICS 0

#endif /* PERFORMANCE_H */
