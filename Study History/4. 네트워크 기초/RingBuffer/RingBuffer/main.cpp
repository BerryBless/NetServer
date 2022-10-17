#include "CRingBuffer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <Windows.h>
#include <time.h>
#include "Profiler.h"
#define BUFFERSIZE 70
CRingBuffer buffer (BUFFERSIZE);
char g_dataSet[] = {"Lorem Ipsum is simply dummy text of the printing and typesetting industry. Lorem Ipsum has been the industry's standard "};



void main() {
	srand(50);

	int enqRandom; // 들어가는거 랜덤
	int deqRandom; // 나오는거 랜덤
	int enqRet;
	int deqRet;
	char *ptr = g_dataSet;	// 현재 데이터셋
	char *deqCopy = new char[sizeof(g_dataSet)]; // 쓸 버퍼
	int leftSize = sizeof(g_dataSet) - 1; // rend에서 나올 최대크기
	DWORD old = clock();

	int iCount = 0;
	while (true) {
		if (leftSize == 0) {
			// 초기화
			ptr = g_dataSet;
			leftSize = sizeof(g_dataSet) - 1; // 맨뒤 널빼고
		}

		enqRandom = rand() % leftSize + 1; // 1~leftSize
		enqRet = buffer.Enqueue(ptr, enqRandom); //랜덤한 길이만큼 넣기 
		ptr += enqRet; //enq된 사이즈만큼 증가.

		deqRandom = rand() % leftSize + 1; // 1~leftSize

		leftSize -= enqRet; // enq된 사이즈만큼 다음 후보에서 빼주기

		memset(deqCopy, 0, sizeof(g_dataSet)); // 널문자 대신 0찍어주기

		if (iCount % 2 == 0) {
			deqRet = buffer.Dequeue(deqCopy, deqRandom); // deq
		}
		else {
			// peek
			deqRet = buffer.Peek(deqCopy, deqRandom);
			buffer.MoveFront(deqRet);
		}
		if (deqRet > 0)
			printf("%s", deqCopy);//deq한거 있으면 출력
		
		iCount++;
		//Sleep(5);

		DWORD cur = clock();
		if (cur - old > 3000) {
			old = cur;
			PRO_PRINT((CHAR *) "ringbuffer.profile");
		}
		
	}
}