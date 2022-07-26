#include <iostream>
#include <queue>
class ICommand {
public :
	virtual void Execute() = 0;
};

// 클래스 A,B,C의 작업이 필요하지만 느려서 떠맏길때

// 잡큐에 모아두었다가 한번에 처리
class CJobQueue  {
public:
	void Push(ICommand * pCmd) {
		_commands.push(pCmd);
	}
	void CommandExcute() {
		while (!_commands.empty()) {
			ICommand *pCmd = _commands.front();
			_commands.pop();
			pCmd->Execute();
		}
	}
private:
	std::queue<ICommand *> _commands;
};


class CA : public ICommand {
public:
	virtual void Execute() {
		printf_s("A의 작업 처리..");
	}
};
class CB : public ICommand {
public:
	virtual void Execute() {
		printf_s("B의 작업 처리..");
	}
};
class CC : public ICommand {
public:
	virtual void Execute() {
		printf_s("C의 작업 처리..");
	}
};

int main() {
	CA a;
	CB b;
	CC c;
	CJobQueue jQueue;
	
	jQueue.Push(&a);
	jQueue.Push(&b);
	jQueue.Push(&b);
	jQueue.Push(&a);
	jQueue.Push(&c);
	jQueue.Push(&a);

	jQueue.CommandExcute();
	return 0;
}