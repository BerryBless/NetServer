#pragma once
#include <wtypes.h>

#define dfLOG_LEVEL_DEBUG 0
#define dfLOG_LEVEL_ERROR 1
#define dfLOG_LEVEL_NOTICE 2
#define dfLOG_LEVEL_OFF 3
	
#define _LOG(logLevel, format, ...) g_sysLogger._Log(logLevel, format, __VA_ARGS__)
#define _SET_LOG_LEVEL(logLevel) g_sysLogger.SetLogLevel(logLevel)


class CLogger {
public:
	CLogger();
	~CLogger();
public:
	void _Log(int logLevel, const WCHAR *format, ...);
	void Initialize();
	void SetDirectory(const WCHAR *path);
	void SetLogLevel(int level) { _logLevel = level; }
	int  GetLogLevel() { return _logLevel; }
private:
	void FileLock();
	void FileUnlock();

private:
	int _logLevel;
	SRWLOCK _lock;
	WCHAR _filePath[MAX_PATH];
	DWORD _logCount;

};

extern CLogger g_sysLogger;
