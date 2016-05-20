
// **********************************************
//        Windows high performance counter
// **********************************************

#ifdef _WIN32
#ifdef _DEBUG
void _STOP_PERFORMANCE_TIMER (char* str);
void _START_PERFORMANCE_TIMER (void);
#define START_PERFORMANCE_TIMER _START_PERFORMANCE_TIMER()
#define STOP_PERFORMANCE_TIMER _STOP_PERFORMANCE_TIMER()
void DbgPrintf (char *msg, ...);
#else
#define START_PERFORMANCE_TIMER
#define STOP_PERFORMANCE_TIMER
#endif
#endif
