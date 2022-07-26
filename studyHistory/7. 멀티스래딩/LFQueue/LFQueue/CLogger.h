#pragma once
#include <wtypes.h>

#define dfLOG_LEVEL_DEBUG 0
#define dfLOG_LEVEL_ERROR 1
#define dfLOG_LEVEL_NOTICE 2
#define dfLOG_LEVEL_OFF 3

class CLogger {
public:
	static void _Log(int logLevel, const WCHAR *format, ...);
	static void Initialize();
	static void SetDirectory(const WCHAR *path);
	static void SetLogLevel(int level) { _logLevel = level; }
	static int getLogLevel() { return _logLevel; }
private:
	static void FileLock();
	static void FileUnlock();

private:
	static int _logLevel;
	static SRWLOCK _lock;
	static WCHAR _filePath[MAX_PATH];
	static DWORD _logCount;

};

#define CRASH() do{\
					CLogger::_Log(dfLOG_LEVEL_ERROR, L"///////CRASH : FILE[%s] Line[%d]",__FILEW__,__LINE__);\
					int *nptr = nullptr; *nptr = 1;\
				}while(0)