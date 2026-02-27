#if defined(DEBUG)

	#if(__amigaos4__)
	#include <interfaces/exec.h>
	#define DBUG(x,...)  DebugPrintF("[%s:%-4ld] "x ,__FILE__,__LINE__, __VA_ARGS__)
	#define DBUGN(x,...) DebugPrintF(""x ,__VA_ARGS__)
	#else
	#include <proto/debug.h>
	#define DBUG(x,...)  KPrintF("[%s:%-4ld] "x ,__FILE__,__LINE__, __VA_ARGS__)
	#define DBUGN(x,...) KPrintF(""x ,__VA_ARGS__)
	#define __FUNCTION__ __func__
	#endif

#else

	#define DBUG(x,...)
	#define DBUGN(x,...)

#endif
