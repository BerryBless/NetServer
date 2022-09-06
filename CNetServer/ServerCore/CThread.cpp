#include "pch.h"
#include "CThread.h"


CThread::CThread() : _isLaunching{ false }, _isRunning{ false }, _hThread{ nullptr }
{
	_hLaunchEvent = nullptr;
	//if (_hLaunchEvent == nullptr) {
	_hLaunchEvent = CreateEvent(nullptr, false, false, nullptr);
	//}
	_hJoinEvent = CreateEvent(nullptr, true, false, nullptr);
}

CThread::~CThread() {
	CloseHandle(_hLaunchEvent);
	CloseHandle(_hJoinEvent);
}

unsigned int __stdcall CThread::Thread(LPVOID arg) {
	CThread *pObject = (CThread *) arg;
	bool ret;
	SetEvent(pObject->_hLaunchEvent);


	CLogger::_Log(dfLOG_LEVEL_NOTICE, L"// %s::TID[%d] START", pObject->_name, GetCurrentThreadId());
	while (pObject->_isRunning) {
		pObject->_isLaunching = false;

		WaitForSingleObject(pObject->_hLaunchEvent, INFINITE);

		CLogger::_Log(dfLOG_LEVEL_DEBUG, L"// %s::TID[%d] BEGIN Execute ", pObject->_name, GetCurrentThreadId());
		ret = pObject->_task.Execute();
		CLogger::_Log(dfLOG_LEVEL_DEBUG, L"// %s::TID[%d] END Execute ", pObject->_name, GetCurrentThreadId());

		SetEvent(pObject->_hJoinEvent);
		ResetEvent(pObject->_hJoinEvent);
		pObject->_task.Clear();
	}
	CLogger::_Log(dfLOG_LEVEL_NOTICE, L"// %s::TID[%d] CLOSED", pObject->_name, GetCurrentThreadId());
	return 0;
}

bool CThread::BeginThread() {
	if (_isRunning == true) {
		CLogger::_Log(dfLOG_LEVEL_ERROR, L"BeginThread() :: %s is already running", _name);
		return false;
	}
	_isRunning = true;
	_hThread = (HANDLE) _beginthreadex(nullptr, 0, Thread, this, 0, nullptr);
	WaitForSingleObject(_hLaunchEvent, INFINITE);
	return true;
}

bool CThread::Launch(void(*pTask)(LPVOID), LPVOID arg) {
	if (_isLaunching == true) {
		CLogger::_Log(dfLOG_LEVEL_ERROR, L"Launch() :: %s is already running", _name);
		return false;
	}
	_isLaunching = true;
	_task.SetTask(pTask, arg);
	SetEvent(_hLaunchEvent);
	return true;
}

void CThread::Join() {
	if (_isLaunching == false) {
		CLogger::_Log(dfLOG_LEVEL_ERROR, L"Join() :: %s is not working on it", _name);
		return;
	}
	DWORD retval = WaitForSingleObject(_hJoinEvent, INFINITE);
}

void CThread::EndThread() {
	if (_isRunning == false) {
		CLogger::_Log(dfLOG_LEVEL_ERROR, L"BeginThread() :: %s is not running", _name);
		return;
	}
	_isRunning = false;
	SetEvent(_hLaunchEvent);
	DWORD retval = WaitForSingleObject(_hThread, INFINITE);
}

void CThread::SetThreadName(const WCHAR *name) {
	wsprintf(_name, L"%s", name);
}
