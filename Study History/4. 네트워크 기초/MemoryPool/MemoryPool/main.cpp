#include <stdio.h>
#include <stdlib.h>
#include "Profiler.h"
#include "CMemory.h"
#include "CObjectPool.hpp"
#include "CList.h"
#include <iostream>
#include <cstring>
#include <ctime>
#include "OtherClass.h"
#define dfMAXSIZE 10000
#define dfMAXCOUNT 1000

unsigned int Erand(int byteSize, bool bPositive = false) {
	unsigned int r = 0;		// 0000 0000
	int mask = 0xFF;		// 1111 1111
	for (int i = 0; i < byteSize; i++) {
		int t = rand();
		t &= mask;
		r = (r << 8); // 1byte 만큼 이동
		r |= t;
		//printf_s("%02d : 0x%08x\n", i, r);
	}
	if (bPositive) r &= ~(1<<byteSize);
	return r;
}



class CObject {
public:
	int _ID;
	int _HP;
	int _attack;
public:
	virtual void SetVal(int a) {
		_ID = a;
		_HP = a;
		_attack = a;
	}
};

class CPlayer : public CObject {
public:
	char _name[30];
public:
	CPlayer() {
		//		PRO_BEGIN(L"CPlayer()");
		SetVal(0x79797979);
		//		PRO_END(L"CPlayer()");
	}
	~CPlayer() {
		//		PRO_BEGIN(L"~CPlayer()");
		SetVal(0x97979797);
		//		PRO_END(L"~CPlayer()");
	}
	int GetID() { return _ID; };
	int GetHP() { return _HP; };
	void SetHP(int iHP) { _HP = iHP; };
	virtual void SetVal(int a) {
		_ID = a;
		_HP = a;
		_attack = a;
		memset(_name, 0xED, 30);
	}
};

class CEnemy : public CObject {
private:
	unsigned char _dummy[5000];
public:
	void Attack() {
		printf_s("ATTACK!");
	}
	virtual void SetVal(int a) {
		_ID = -a;
		_HP = -a;
		_attack = -a;
	}
};


#pragma region TEMP
// 테스트용으로 만든 임시 new/delete

CPlayer *playernew() {
	CPlayer *pData = (CPlayer *) g_memoryPool.Alloc(sizeof(CPlayer));
	new(pData)CPlayer(); // placement new
	return pData;
}

void playerdelete(CPlayer *obj) {
	obj->~CPlayer();
	g_memoryPool.Free(obj);
}

CEnemy *enemynew() {
	CEnemy *pData = (CEnemy *) g_memoryPool.Alloc(sizeof(CEnemy));
	new(pData)CEnemy(); // placement new
	return pData;

}
void enemydelete(CEnemy *obj) {
	obj->~CEnemy();
	g_memoryPool.Free(obj);
}

#pragma endregion
bool comp(char *a, char *b) {
	return *a > *b;
}
int main() {
	srand(40);
	long long testcase;
	int randsize;	// 랜덤 크기
	int randtime;	// 랜덤 반복 횟수
	int i;
	CList<char *> allocPtr;
	CList<char *> poolPtr;
	char *buffer;
	time_t timer = clock();
	time_t tick;
	for (testcase = 1;; testcase++) {
		printf_s("\n ========================== \n");
		tick = clock();
		if (tick - timer >= 30000) {
			timer = tick;
			// Alloc_YYYYMMDD_HHMMSS.log
			char filename[50];	// 저장할파일 이름

			// 시간구하기
			time_t now = time(0);
			struct tm tstruct;

			localtime_s(&tstruct, &now);
			strftime(filename, sizeof(filename), "log/memorypool_%Y%m%e_%H%M%S.log", &tstruct); // (mempool_20210828_173840.log)
			PRO_PRINT(filename);
			g_memoryPool.Monitoring(filename);
		}

		randtime = Erand(3, true) % dfMAXCOUNT + 1;
		for (int i = 0; i < randtime; i++) {
			randsize = rand() % dfMAXSIZE + 1; // 1~4096
			PRO_BEGIN(L"::malloc");
			buffer = (char *) malloc(randsize);
			PRO_END(L"::malloc");
			 allocPtr.push_back (buffer);

			PRO_BEGIN(L"Pool::Alloc");
			buffer = (char *) g_memoryPool.Alloc(randsize);
			PRO_END(L"Pool::Alloc");
			poolPtr.push_back(buffer);
			memset(buffer, Erand(1), randsize);

			if (i % (dfMAXCOUNT/10) == 0) {
				printf_s("[%lld] Allocate.. %d / %d\n", testcase, i, randtime);
			}

		}
		OtherClass other;

		printf_s("\n ------- \n");
		printf_s("Sorting \n");
		allocPtr.sort(comp);
		poolPtr.sort(comp);
		printf_s("\n ------- \n");

		// TODO 오버플로 체크
		// 일부러 조금 남기고 나중에 free하기
		for (int i = 0; i < randtime; i++) {

			buffer = allocPtr.pop_front();
			PRO_BEGIN(L"::free");
			free(buffer);
			PRO_END(L"::free");

			buffer = poolPtr.pop_front();
			PRO_BEGIN(L"Pool::Free");
			g_memoryPool.Free(buffer);
			PRO_END(L"Pool::Free");
			if (i % (dfMAXCOUNT / 10) == 0) {
					printf_s("[%lld] Release.. %d / %d\n", testcase, i, randtime);
			}
		}
		printf_s("\n ========================== \n");
	}


	return 0;
}
