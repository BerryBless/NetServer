#include "pch.h"
#include "CorePch.h"


void HelloWorld() {
	printf_s("HELLO SERVER!");
}

long CCrashDump::_dumpCount = 0;
static CCrashDump g_dump;
