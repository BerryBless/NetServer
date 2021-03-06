#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <conio.h>
#include <time.h>

#define dfLOGFILENAME "echoserver.log"

#define dfLOG_LEVEL_DEBUG 0
#define dfLOG_LEVEL_NOTICE 1
#define dfLOG_LEVEL_WARNING 2
#define dfLOG_LEVEL_ERROR 3

#define _LOG(LogLevel, fmt, ...)	\
	do{	\
		if(g_LogLevel <= LogLevel) { \
			swprintf_s(g_wsLogBuf,fmt,##__VA_ARGS__);\
			Log(g_wsLogBuf,LogLevel);\
		}\
	} while (0) 

#define _LOG_NONTIMESTAMP(LogLevel, fmt, ...)\
	do{	\
		if(g_LogLevel <= LogLevel) { \
			swprintf_s(g_wsLogBuf,fmt,##__VA_ARGS__);\
			Log_nontimestamp(g_wsLogBuf,LogLevel);\
		}\
	} while (0) 

#define CRASH() do{\
		_LOG(dfLOG_LEVEL_ERROR, L"///////CRASH : %s [%d]",__FILEW__,__LINE__);\
		while(true);\
		int *nptr = nullptr; *nptr = 1;\
	}while(0)


extern int g_LogLevel;
extern wchar_t g_wsLogBuf[256];
extern char g_sFilename[256];


void Log(wchar_t *wsString, int iLogLevel);
void Log_nontimestamp(wchar_t *wsString, int iLogLevel);