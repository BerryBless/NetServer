#include "CRingbuffer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <Windows.h>
#include <process.h>
#include <conio.h>
#include <time.h>
#include "Profiler.h"
#define BUFFERSIZE 111
CRingBuffer g_buffer(1000000);
#define dfFRCHECK

char g_dataSet[] = {"dialgapalkiagiratinaheatrancresaeliaregigiasuxiemespritazelftornadusthundurslandorusenamorusarceuswyrderkleavorursalumacdialgapalkiagiratinaheatrancresaeliaregigiasuxiemespritazelftornadusthundurslandorusenamorusarceuswyrderkleavorursalumacdialgapalkiagiratinaheatrancresaeliaregigiasuxiemespritazelftornadusthundurslandorusenamorusarceuswyrderkleavorursalumacdialgapalkiagiratinaheatrancresaeliaregigiasuxiemespritazelftornadusthundurslandorusenamorusarceuswyrderkleavorursalumacdialgapalkiagiratinaheatrancresaeliaregigiasuxiemespritazelftornadusthundurslandorusenamorusarceuswyrderkleavorursalumacdialgapalkiagiratinaheatrancresaeliaregigiasuxiemespritazelftornadusthundurslandorusenamorusarceuswyrderkleavorursalumacdialgapalkiagiratinaheatrancresaeliaregigiasuxiemespritazelftornadusthundurslandorusenamorusarceuswyrderkleavorursalumacdialgapalkiagiratinaheatrancresaeliaregigiasuxiemespritazelftornadusthundurslandorusenamorusarceuswyrderkleavorursalumacdialgapalkiagiratinaheatrancresaeliaregigiasuxiemespritazelftornadusthundurslandorusenamorusarceuswyrderkleavorursalumacdialgapalkiagiratinaheatrancresaeliaregigiasuxiemespritazelftornadusthundurslandorusenamorusarceuswyrderkleavorursalumacdialgapalkiagiratinaheatrancresaeliaregigiasuxiemespritazelftornadusthundurslandorusenamorusarceuswyrderkleavorursalumacdialgapalkiagiratinaheatrancresaeliaregigiasuxiemespritazelftornadusthundurslandorusenamorusarceuswyrderkleavorursalumacdialgapalkiagiratinaheatrancresaeliaregigiasuxiemespritazelftornadusthundurslandorusenamorusarceuswyrderkleavorursalumacdialgapalkiagiratinaheatrancresaeliaregigiasuxiemespritazelftornadusthundurslandorusenamorusarceuswyrderkleavorursalumacdialgapalkiagiratinaheatrancresaeliaregigiasuxiemespritazelftornadusthundurslandorusenamorusarceuswyrderkleavorursalumacdialgapalkiagiratinaheatrancresaeliaregigiasuxiemespritazelftornadusthundurslandorusenamorusarceuswyrderkleavorursalumacdialgapalkiagiratinaheatrancresaeliaregigiasuxiemespritazelftornadusthundurslandorusenamorusarceuswyrderkleavorursalumacdialgapalkiagiratinaheatrancresaeliaregigiasuxiemespritazelftornadusthundurslandorusenamorusarceuswyrderkleavorursalumacdialgapalkiagiratinaheatrancresaeliaregigiasuxiemespritazelftornadusthundurslandorusenamorusarceuswyrderkleavorursalumacdialgapalkiagiratinaheatrancresaeliaregigiasuxiemespritazelftornadusthundurslandorusenamorusarceuswyrderkleavorursalumacdialgapalkiagiratinaheatrancresaeliaregigiasuxiemespritazelftornadusthundurslandorusenamorusarceuswyrderkleavorursalumacdialgapalkiagiratinaheatrancresaeliaregigiasuxiemespritazelftornadusthundurslandorusenamorusarceuswyrderkleavorursalumacdialgapalkiagiratinaheatrancresaeliaregigiasuxiemespritazelftornadusthundurslandorusenamorusarceuswyrderkleavorursalumacdialgapalkiagiratinaheatrancresaeliaregigiasuxiemespritazelftornadusthundurslandorusenamorusarceuswyrderkleavorursalumacdialgapalkiagiratinaheatrancresaeliaregigiasuxiemespritazelftornadusthundurslandorusenamorusarceuswyrderkleavorursalumacdialgapalkiagiratinaheatrancresaeliaregigiasuxiemespritazelftornadusthundurslandorusenamorusarceuswyrderkleavorursalumacdialgapalkiagiratinaheatrancresaeliaregigiasuxiemespritazelftornadusthundurslandorusenamorusarceuswyrderkleavorursalumacdialgapalkiagiratinaheatrancresaeliaregigiasuxiemespritazelftornadusthundurslandorusenamorusarceuswyrderkleavorursalumacdialgapalkiagiratinaheatrancresaeliaregigiasuxiemespritazelftornadusthundurslandorusenamorusarceuswyrderkleavorursalumacdialgapalkiagiratinaheatrancresaeliaregigiasuxiemespritazelftornadusthundurslandorusenamorusarceuswyrderkleavorursalumacdialgapalkiagiratinaheatrancresaeliaregigiasuxiemespritazelftornadusthundurslandorusenamorusarceuswyrderkleavorursalumacdialgapalkiagiratinaheatrancresaeliaregigiasuxiemespritazelftornadusthundurslandorusenamorusarceuswyrderkleavorursalumacdialgapalkiagiratinaheatrancresaeliaregigiasuxiemespritazelftornadusthundurslandorusenamorusarceuswyrderkleavorursalumacdialgapalkiagiratinaheatrancresaeliaregigiasuxiemespritazelftornadusthundurslandorusenamorusarceuswyrderkleavorursalumacdialgapalkiagiratinaheatrancresaeliaregigiasuxiemespritazelftornadusthundurslandorusenamorusarceuswyrderkleavorursalumacdialgapalkiagiratinaheatrancresaeliaregigiasuxiemespritazelftornadusthundurslandorusenamorusarceuswyrderkleavorursalumacdialgapalkiagiratinaheatrancresaeliaregigiasuxiemespritazelftornadusthundurslandorusenamorusarceuswyrderkleavorursalumacdialgapalkiagiratinaheatrancresaeliaregigiasuxiemespritazelftornadusthundurslandorusenamorusarceuswyrderkleavorursalumacdialgapalkiagiratinaheatrancresaeliaregigiasuxiemespritazelftornadusthundurslandorusenamorusarceuswyrderkleavorursalumacdialgapalkiagiratinaheatrancresaeliaregigiasuxiemespritazelftornadusthundurslandorusenamorusarceuswyrderkleavorursalumacdialgapalkiagiratinaheatrancresaeliaregigiasuxiemespritazelftornadusthundurslandorusenamorusarceuswyrderkleavorursalumacdialgapalkiagiratinaheatrancresaeliaregigiasuxiemespritazelftornadusthundurslandorusenamorusarceuswyrderkleavorursalumacdialgapalkiagiratinaheatrancresaeliaregigiasuxiemespritazelftornadusthundurslandorusenamorusarceuswyrderkleavorursalumacdialgapalkiagiratinaheatrancresaeliaregigiasuxiemespritazelftornadusthundurslandorusenamorusarceuswyrderkleavorursalumacdialgapalkiagiratinaheatrancresaeliaregigiasuxiemespritazelftornadusthundurslandorusenamorusarceuswyrderkleavorursalumacdialgapalkiagiratinaheatrancresaeliaregigiasuxiemespritazelftornadusthundurslandorusenamorusarceuswyrderkleavorursalumacdialgapalkiagiratinaheatrancresaeliaregigiasuxiemespritazelftornadusthundurslandorusenamorusarceuswyrderkleavorursalumacdialgapalkiagiratinaheatrancresaeliaregigiasuxiemespritazelftornadusthundurslandorusenamorusarceuswyrderkleavorursalumacdialgapalkiagiratinaheatrancresaeliaregigiasuxiemespritazelftornadusthundurslandorusenamorusarceuswyrderkleavorursalumacdialgapalkiagiratinaheatrancresaeliaregigiasuxiemespritazelftornadusthundurslandorusenamorusarceuswyrderkleavorursalumacdialgapalkiagiratinaheatrancresaeliaregigiasuxiemespritazelftornadusthundurslandorusenamorusarceuswyrderkleavorursalumacdialgapalkiagiratinaheatrancresaeliaregigiasuxiemespritazelftornadusthundurslandorusenamorusarceuswyrderkleavorursalumacdialgapalkiagiratinaheatrancresaeliaregigiasuxiemespritazelftornadusthundurslandorusenamorusarceuswyrderkleavorursalumacdialgapalkiagiratinaheatrancresaeliaregigiasuxiemespritazelftornadusthundurslandorusenamorusarceuswyrderkleavorursalumacdialgapalkiagiratinaheatrancresaeliaregigiasuxiemespritazelftornadusthundurslandorusenamorusarceuswyrderkleavorursalumac"};


bool g_start = false;
////////////////////////
// EnqThread	: 3, 4,
// DeqThread	:30,40,
// time			:9m,2m,
////////////////////////

void Log(char *pString) {
	FILE *fp;

	// 뒤에 추가
	fopen_s(&fp, "ringbuffer.log", "a");
	if (fp == NULL) {
		// 없음 생성
		fopen_s(&fp, "ringbuffer.log", "w");
		if (fp == NULL) {
			// crash
			int *p = nullptr; *p = 10;
			return;
		}
	} else {
		fseek(fp, 0, SEEK_END);
	}

	//fprintf_s(fp, "%s", pString);
	fprintf_s(stdout, "%s", pString);

	fclose(fp);
}

unsigned int __stdcall EnqThread(PVOID pData) {
	while (!g_start);// 동시시작 스핀락

	srand(4);
	int enqRandom; // 들어가는거 랜덤
	int enqRet;
	char *ptr = g_dataSet;	// 현재 데이터셋
	int leftSize = sizeof(g_dataSet) - 1; // rend에서 나올 최대크기
	while (g_start) {
		if (leftSize == 0) {
			// 초기화
			ptr = g_dataSet;
			leftSize = sizeof(g_dataSet) - 1; // 맨뒤 널빼고
		}
		enqRandom = rand() % leftSize + 1; // 1~leftSize
		enqRet = g_buffer.Enqueue(ptr, enqRandom); //랜덤한 길이만큼 넣기 
		if (leftSize - enqRet < 0) {
			return 0;
		}
		ptr += enqRet; //enq된 사이즈만큼 증가.
		leftSize -= enqRet; // enq된 사이즈만큼 다음 후보에서 빼주기
		//Sleep(1);
	}
	return 0;
}
unsigned int __stdcall DeqThread(PVOID pData) {
	while (!g_start);// 동시시작 스핀락

	srand(40);
	int deqRandom; // 나오는거 랜덤
	int deqRet;
	char *deqCopy = new char[sizeof(g_dataSet)]; // 쓸 버퍼
	int iCount = 0;
	int randMax = sizeof(g_dataSet) - 2;
	while (g_start) {
		deqRandom = rand() % randMax;
		memset(deqCopy, 0, sizeof(g_dataSet)); // 널문자 대신 0찍어주기

		if (iCount % 2 == 0) {
			deqRet = g_buffer.Dequeue(deqCopy, deqRandom); // deq
		} else {
			// peek
			deqRet = g_buffer.Peek(deqCopy, deqRandom);
			g_buffer.MoveFront(deqRet);
		}
#ifndef dfFRCHECK
		if (deqRet > 0)
			printf("%s", deqCopy);//deq한거 있으면 출력
#else
		if (deqRet > 0) {
			char atoA = 'a' - 'A';
			*deqCopy -= atoA;
			*(deqCopy + deqRet - 1) -= atoA;
			//printf("%s", deqCopy);//deq한거 있으면 출력
			//Log(deqCopy);
		}

#endif // !dfFRCHECK
		iCount++;
		//Sleep(1);
	}
	return 0;
}





int main() {
	HANDLE hThread[2];
	hThread[0] = (HANDLE) _beginthreadex(NULL, 0, &EnqThread, 0, 0, NULL);
	hThread[1] = (HANDLE) _beginthreadex(NULL, 0, &DeqThread, 0, 0, NULL);

	Sleep(100);
	g_start = true;

	DWORD old = clock();
	while (1) {
		DWORD cur = clock();
		if (cur - old > 3000) {
			old = cur;
			PRO_PRINT((CHAR *) "ringbuffer.profile");
		}
	}

	WaitForMultipleObjects(2, hThread, TRUE, INFINITE);

	return 0;
}
