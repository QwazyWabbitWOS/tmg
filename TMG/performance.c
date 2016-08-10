
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
	sprintf(string, "%s executed in %.6f secs.\n", str, res);
	OutputDebugString(string);
	//	Com_Printf (string);
	totalTime += res;
}
#endif
#endif

#ifdef _DEBUG
#define DEBUG
#endif

//QW//
/* Use this function to trace execution or whatever.
This improves upon OutputDebugString a bit
to allow var_args instead of static text.
Outputs to the debugger and allows
us to write: DbgPrintf("%s was called.\n", __func__);
Use Quake 2's gi.dprintf to output to the Quake 2 console.
In Linux, this function becomes a call to gi.dprintf but
it outputs only if developer cvar is set.
*/
void DbgPrintf (char *msg, ...)
{
	va_list	argptr;
	char	text[1024];

	va_start (argptr, msg);
	vsnprintf (text, sizeof(text), msg, argptr);
	va_end (argptr);
#ifdef _WIN32
#ifdef DEBUG
	OutputDebugString(text);
#endif /* _DEBUG */
#else // Not WIN32
	if(developer->value)
		gi.dprintf(text);
#endif /* _WIN32 */
}
