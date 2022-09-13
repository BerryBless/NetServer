#include "pch.h"
#include "CThread.h"


CThread::CThread() : _hLaunchEvent{ nullptr }, _isLaunching{ false }, _isRunning{ false }, _hThread{ nullptr }
{
	_hLaunchEvent = CreateEvent(nullptr, false, false, nullptr);
	_hJoinEvent = CreateEvent(nullptr, true, false, nullptr);
	this->SetThreadName(L"Default Thread");
	this->BeginThread();
}

CThread::CThread(const WCHAR *tName) :_hLaunchEvent{ nullptr }, _isLaunching{ false }, _isRunning{ false }, _hThread{ nullptr }{
	_hLaunchEvent = CreateEvent(nullptr, false, false, nullptr);
	_hJoinEvent = CreateEvent(nullptr, true, false, nullptr);
	this->SetThreadName(tName);
	this->BeginThread();
}

CThread::~CThread() {
	this->EndThread();
	CloseHandle(_hLaunchEvent);
	CloseHandle(_hJoinEvent);
}

unsigned int __stdcall CThread::Thread(LPVOID arg) {
	CThread *pObject = (CThread *) arg;
	bool ret;
	SetEvent(pObject->_hLaunchEvent);


	CLogger::_Log(dfLOG_LEVEL_ERROR, L"// %s::TID[%d] START", pObject->_name, pObject->_tid);
	while (pObject->_isRunning) {
		pObject->_isLaunching = false;

		WaitForSingleObject(pObject->_hLaunchEvent, INFINITE);

		CLogger::_Log(dfLOG_LEVEL_DEBUG, L"// %s::TID[%d] BEGIN Execute ", pObject->_name, pObject->_tid);
		ret = pObject->_task.Execute();
		CLogger::_Log(dfLOG_LEVEL_DEBUG, L"// %s::TID[%d] END Execute ", pObject->_name, pObject->_tid);

		SetEvent(pObject->_hJoinEvent);
		ResetEvent(pObject->_hJoinEvent);
		pObject->_task.Clear();
	}
	CLogger::_Log(dfLOG_LEVEL_ERROR, L"// %s::TID[%d] CLOSED", pObject->_name, pObject->_tid);
	return 0;
}
bool CThread::Launch(void(*pTask)(LPVOID), LPVOID arg) {
	if (_isLaunching == true) {
		CLogger::_Log(dfLOG_LEVEL_DEBUG, L"// TID[%d] Launch() :: %s is already running", this->_tid, _name);
		return false;
	}
	_isLaunching = true;
	_task.SetTask(pTask, arg);
	SetEvent(_hLaunchEvent);
	return true;
}
void CThread::Join() {
	if (_isLaunching == false) {
		CLogger::_Log(dfLOG_LEVEL_ERROR, L"// TID[%d] Join() :: %s is not working on it", this->_tid, _name);
		return;
	}
	DWORD retval = WaitForSingleObject(_hJoinEvent, INFINITE);
}


bool CThread::BeginThread() {
	if (_isRunning == true) {
		CLogger::_Log(dfLOG_LEVEL_ERROR, L"// TID[%d] BeginThread() :: %s is already running", this->_tid, _name);
		return false;
	}
	_isRunning = true;

	static LONG userThreadID = 0;
	_tid = InterlockedIncrement(&userThreadID);
	_hThread = (HANDLE) _beginthreadex(nullptr, 0, Thread, this, 0, nullptr);
	WaitForSingleObject(_hLaunchEvent, INFINITE);
	return true;
}

void CThread::EndThread() {
	if (_isRunning == false) {
		CLogger::_Log(dfLOG_LEVEL_ERROR, L"// TID[%d] BeginThread() :: %s is not running", this->_tid, _name);
		return;
	}
	_isRunning = false;
	SetEvent(_hLaunchEvent);
	DWORD retval = WaitForSingleObject(_hThread, INFINITE);
}

void CThread::SetThreadName(const WCHAR *name) {
	wsprintf(_name, L"%s", name);
	CLogger::_Log(dfLOG_LEVEL_DEBUG, L"// Changed Thread Name %s::TID[%d] ", _name, _tid);
}
