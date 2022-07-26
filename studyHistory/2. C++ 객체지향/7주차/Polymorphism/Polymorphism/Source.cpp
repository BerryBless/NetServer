#include <iostream>
#include <Windows.h>
#include <conio.h>

#define MAXSTAR 20
#define MAXLEN 80

class BaseObject
{
protected:
	int _X;

public:
	virtual bool Update() = 0;	// 리턴으로 파괴여부 결정
	virtual void Draw() = 0;
};

class OneStar : public BaseObject {
public:
	OneStar() {
		_X = 1;
	}
	virtual bool Update() {
		if (_X > 0 && _X < MAXLEN) {
			_X += 1;
			return true;
		}
		return false;
	}
	virtual void Draw() {
		for (int i = 0; i <= _X; i++) {
			printf_s(" ");
		}
		printf_s("*");
	}
};
class TwoStar : public BaseObject {
public:
	TwoStar() {
		_X = 2;
	}
	virtual bool Update() {
		if (_X > 0 && _X < MAXLEN) {
			_X += 2;
			return true;
		}
		return false;
	}
	virtual void Draw() {
		for (int i = 0; i <= _X; i++) {
			printf_s(" ");
		}
		printf_s("**");
	}
};
class ThreeStar : public BaseObject {
public:
	ThreeStar() {
		_X = 3;
	}
	virtual bool Update() {
		if (_X > 0 && _X < MAXLEN) {
			_X += 3;
			return true;
		}
		return false;
	}
	virtual void Draw() {
		for (int i = 0; i <= _X; i++) {
			printf_s(" ");
		}
		printf_s("***");
	}
};

void KeyProcess();
void Update();
void Draw();

BaseObject* _objs[MAXSTAR];

int main()
{
	for (int i = 0; i < MAXSTAR; i++) {
		_objs[i] = nullptr;
	}

	while (1)
	{
		KeyProcess();

		Update();

		system("cls");
		Draw();

		Sleep(30);
	}
}


void KeyProcess() {
	BaseObject* base = nullptr;
	if (_kbhit()) {
		char in = _getch();
		switch (in)		{
		case '1':
			base = new OneStar();
			break;
		case '2':
			base = new TwoStar();
			break;
		case '3':
			base = new ThreeStar();
			break;
		default:
			break;
		}

		if(base != nullptr){
			for (int i = 0; i < 20; i++) {
				if (_objs[i] == nullptr) {
					_objs[i] = base;
					break;
				}
			}
		}
	}

}
void Update() {
	for (int i = 0; i < MAXSTAR; i++) {
		if (_objs[i] == nullptr) continue;
		if (_objs[i]->Update() == false) {
			delete _objs[i];
			_objs[i] = nullptr;
		}
	}
}
void Draw() {
	for (int i = 0; i < MAXSTAR; i++) {
		if (_objs[i] == nullptr) {
			printf_s("\n");
			continue;
		}
		_objs[i]->Draw();
		printf_s("\n");
	}
}