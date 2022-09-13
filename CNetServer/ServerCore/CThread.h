#pragma once


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
	CThread(const WCHAR *tName);
	~CThread();

	static unsigned int __stdcall Thread(LPVOID arg);
public:
	bool Launch(void (*pTask)(LPVOID), LPVOID arg);
	void Join();

	void SetThreadName(const WCHAR *name);
private:
	bool BeginThread();
	void EndThread();

private:
	bool		_isLaunching;
	bool		_isRunning;
	CTask		_task;
	HANDLE		_hThread;
	HANDLE		_hLaunchEvent; 
	HANDLE		_hJoinEvent;

	int _tid;
	WCHAR _name[64] = L"";
};
