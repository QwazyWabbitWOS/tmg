
// **********************************************
//        Windows high performance counter
// **********************************************
//lifted from r1q2 :)

// this version outputs to the debugger window
// instead of to the console log.

#include "g_local.h"
#include "performance.h"

#ifdef _WIN32
#ifdef _DEBUG

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <stdio.h>

LARGE_INTEGER start;
double totalTime = 0;

void _START_PERFORMANCE_TIMER (void)
{
	QueryPerformanceCounter (&start);
}

void _STOP_PERFORMANCE_TIMER (char* str)
{
	double res;
	LARGE_INTEGER stop;
	__int64 diff;
	LARGE_INTEGER freq;
	char string[64];

	QueryPerformanceCounter (&stop);
	QueryPerformanceFrequency (&freq);
	diff = stop.QuadPart - start.QuadPart;
	res = ((double)((double)diff / (double)freq.QuadPart));
	Com_sprintf(string, sizeof string,
		"%s executed in %.9f secs.\n", str, res);
	OutputDebugString(string);
	//	Com_Printf (string);
	totalTime += res;
}
#endif
#endif


//QW//
/* Use this function to trace execution or whatever.
This improves upon OutputDebugString a bit
to allow var_args instead of static text.
Outputs to the debugger and allows
us to write: DbgPrintf("%s was called.\n", __func__);
In Non-Windows, this function becomes a call to gi.dprintf but
it outputs only if gamedebug cvar is set.
*/
void DbgPrintf (char *msg, ...)
{
	va_list	argptr;
	char	text[1024];

	va_start (argptr, msg);
	vsnprintf (text, sizeof(text), msg, argptr);
	va_end (argptr);

#if defined _DEBUG
#if defined _WIN32
	OutputDebugString(text);
#endif /* _WIN32 */
#else // not _WIN32 or not _DEBUG
	if(gamedebug->value)
		gi.dprintf(text);
#endif /* DEBUG */
}
