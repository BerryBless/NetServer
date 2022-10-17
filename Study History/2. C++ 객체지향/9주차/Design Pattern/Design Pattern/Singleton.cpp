#include <stdio.h>
#include <iostream>
#include <thread>
/* 전역 함수로 구현하는 싱글톤*/
/* 유사 싱글톤
class System {
private :
	int _Data;
public:
	System() :_Data(0) {};
	~System() {};

	int GetData() { return _Data; }
	void SetData(int iData) { _Data = iData; }
};


System *GetInstance() {
	static System _Sys;
	return &_Sys;
}

//int _tmain() {
//	System *pSys = GetInstance();
//
//	printf("%d", pSys->GetData());
//	return 0;
//}

//*/

/* 1. GetInstance 내에 static 지역 변수를 두는 방법
class System {
private:
	int _Data;

private:
	System() :_Data(0) {};
	~System() {};

public:
	static System *GetInstance() {
		static System _Sys;
		return &_Sys;
	}

	int GetData() { return _Data; }
	void SetData(int iData) { _Data = iData; }
};


int main() {
	printf("%d", System::GetInstance()->GetData());
	return 0;
}
//*/


/* 클래스 맴버 변수에 static으로 자기 자신을 가지는 방법
class System {
private:
	int _Data;
	static System _System;
private:
	System() :_Data(0) { printf("생성"); };
	~System() { printf("파괴"); };

public:
	static System *GetInstance() {
		return &_System;
	}

	int GetData() { return _Data; }
	void SetData(int iData) { _Data = iData; }
};

// static 맴버 변수를 사용함으로 외부에서 선언을 꼭 해주어야 함.
System System::_System;

int main() {
	printf("%d", System::GetInstance()->GetData());
	return 0;
}
//*/

/* 동적할당을 통한 싱글톤  (클라 굿, 서버 애매)
class System {
private:
	int _Data;
	static System *_pSystem;
private:
	System() :_Data(0) { printf("생성"); };
	~System() { printf("파괴"); };

public:
	static System *GetInstance() {
		// DCL 패턴 (멀티스레딩)
		//if (_pSystem == NULL) {
		//	 lock
		if (_pSystem == NULL)
			_pSystem = new System;
		//	 unlock
		//}
		return _pSystem;
	}

	int GetData() { return _Data; }
	void SetData(int iData) { _Data = iData; }
};

System *System::_pSystem = NULL;
int main() {
	printf("%d", System::GetInstance()->GetData());
	return 0;
}
//*/

/* 템플릿 클래스를 통한 싱글톤 구현 (왜쓰는지 이해안됨, 나도 강사도)

template<typename T>
class TemplateSingleton {
protected:
	TemplateSingleton() {};
	~TemplateSingleton() {};

private :
	static T *_pInstance;

public:
	static T *GetInstance() {
		if (_pInstance = NULL)
			_pInstance = new T;
		return _pInstance;
	};

	static void DestroyInstance() {
		if (_pInstance) {
			delete _pInstance;
			_pInstance = NULL;
		}
	};
};

template<typename T> T *TemplateSingleton<T> ::_pInstance = 0;

class System : public TemplateSingleton<System> {
public:
	System() {};
	~System() {};

	int _Data;

public:
	int GetData() { return _Data; }
	void SetData(int iData) { _Data = iData; }
};

int main() {
	printf("%d", System::GetInstance()->GetData());
	return 0;
}
//*/


/* 동적할당을 하면서 파괴자를 불러주는 싱글톤
class System {
private:
	int _Data;
	static System *_pSystem;
private:
	System() :_Data(0) { printf("생성"); };
	~System() { printf("파괴"); };

public:
	static System *GetInstance() {
		if (_pSystem == NULL) {
			_pSystem = new System;
			atexit(Destroy);	// main 종료 직전에 호출
		}
		return _pSystem;
	}
	static void Destroy() {
		delete _pSystem;
		//_pSystem = NULL; // 피닉스 싱글톤
	}
	int GetData() { return _Data; }
	void SetData(int iData) { _Data = iData; }
};

System *System::_pSystem = NULL;
int main() {
	printf("%d", System::GetInstance()->GetData());
	return 0;
}
//*/