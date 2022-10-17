#pragma once
#include <queue>
#include <process.h>
#include <Windows.h>


class CThread {
private:
	class CTask {
	public:
		CTask() : _task{ nullptr }, _arg{ nullptr }{}
		~CTask() {}
		inline void SetTask(void (*pTask)(LPVOID), LPVOID arg) {
			_task = pTask;
			_arg = arg;
		}
		inline bool Execute() {
			if (_task == nullptr) return false;
			(*_task)(_arg);
			return true;
		}
		inline void Clear() {
			_task = nullptr;
		}
	private:
		void (*_task)(LPVOID);
		LPVOID _arg;
	};
public:
	CThread();
	~CThread();

	static unsigned int __stdcall Thread(LPVOID arg);

public:
	bool BeginThread();
	bool Launch(void (*pTask)(LPVOID), LPVOID arg);
	void Join();
	void EndThread();

	void SetThreadName(const WCHAR *name);
private:
	bool _isLaunching;
	bool _isRunning;
	CTask _task;
	HANDLE _hThread;
	HANDLE _hLaunchEvent; // 스레드당 하나냐 풀당 하나냐...
	HANDLE _hJoinEvent;

	WCHAR _name[50] = L"";
};

