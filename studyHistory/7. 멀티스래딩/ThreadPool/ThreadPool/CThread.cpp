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
	wprintf_s(L"%s::TID[%d] START \n", pObject->_name, GetCurrentThreadId());
	while (pObject->_isRunning) {
		pObject->_task.Clear();
		pObject->_isLaunching = false;

		WaitForSingleObject(pObject->_hLaunchEvent, INFINITE);
		
		ret = pObject->_task.Execute();
		SetEvent(pObject->_hJoinEvent);
		ResetEvent(pObject->_hJoinEvent);
	}
	wprintf_s(L"%s::TID[%d] Closed \n", pObject->_name,GetCurrentThreadId());
	return 0;
}

bool CThread::BeginThread() {
	if (_isRunning == true) return false;
	_isRunning = true;
	_hThread = (HANDLE) _beginthreadex(nullptr, 0, Thread, this, 0, nullptr);
	WaitForSingleObject(_hLaunchEvent, INFINITE);
	return true;
}

bool CThread::Launch(void(*pTask)(LPVOID), LPVOID arg) {
	if (_isLaunching == true) return false;
	_isLaunching = true;
	_task.SetTask(pTask, arg);
	SetEvent(_hLaunchEvent);
	return true;
}

void CThread::Join() {
	if (_isLaunching == false) return;
	DWORD retval = WaitForSingleObject(_hJoinEvent, INFINITE);
}

void CThread::EndThread() {
	if (_isRunning == false) return;
	_isRunning = false;
	SetEvent(_hLaunchEvent);
	DWORD retval = WaitForSingleObject(_hThread, INFINITE);
}

void CThread::SetThreadName(const WCHAR *name) {
	wsprintf(_name, L"%s", name);
}
