#pragma once
#include <process.h>
#include <Windows.h>

#define dfRINGBUFFERSIZE 10000

class RingBuffer {
private: // 큐
	char *_buffer;	// 큐
	int _front;		// 앞 (당장 빼낼곳)
	int _rear;		// 뒤 (당장 들어갈곳)


private: // 링버퍼 관리 변수
	int _iBufferSize; // 현재 버퍼 사이즈

private:
	SRWLOCK _srwlock;

public:
	RingBuffer(void); // 디폴트 : 10000byte만큼 사이즈 잡음
	RingBuffer(int iSize); // size만큼 버퍼잡기
	~RingBuffer(void);// 버퍼 소멸

public:
	/////////////////////////////////////////////////////////////////////////
	// 현재 버퍼의 크기
	// 
	// Parameters: 없음.
	// Return: (int)버퍼의 용량.
	/////////////////////////////////////////////////////////////////////////
	int GetBufferSize(void);

	/////////////////////////////////////////////////////////////////////////
	// 현재 사용중인 용량 얻기.
	//
	// Parameters: 없음.
	// Return: (int)사용중인 용량.
	/////////////////////////////////////////////////////////////////////////
	int GetUseSize(void);

	/////////////////////////////////////////////////////////////////////////
	// 현재 버퍼에 남은 용량 얻기.
	//
	// Parameters: 없음.
	// Return: (int)남은용량.
	/////////////////////////////////////////////////////////////////////////
	int GetFreeSize(void);

	/////////////////////////////////////////////////////////////////////////
	// 버퍼 포인터로 외부에서 한방에 읽고, 쓸 수 있는 길이.
	// (끊기지 않은 길이)
	//
	// 원형 큐의 구조상 버퍼의 끝단에 있는 데이터는 끝 -> 처음으로 돌아가서
	// 2번에 데이터를 얻거나 넣을 수 있음. 이 부분에서 끊어지지 않은 길이를 의미
	//
	// Parameters: 없음.
	// Return: (int)사용가능 용량.
	////////////////////////////////////////////////////////////////////////
	int DirectEnqueueSize(void);
	int DirectDequeueSize(void);


	/////////////////////////////////////////////////////////////////////////
	// WritePos 에 데이타 넣음.
	//
	// Parameters: (char *)데이타 포인터. (int)크기.
	// Return: (int)넣은 크기.
	/////////////////////////////////////////////////////////////////////////
	int Enqueue(const unsigned char *pData, int iSize);

	/////////////////////////////////////////////////////////////////////////
	// ReadPos 에서 데이타 가져옴. ReadPos 이동.
	//
	// Parameters: (char *)데이타 포인터. (int)크기.
	// Return: (int)가져온 크기.
	/////////////////////////////////////////////////////////////////////////
	int Dequeue(unsigned char *pDest, int iSize);

	/////////////////////////////////////////////////////////////////////////
	// ReadPos 에서 데이타 읽어옴. ReadPos 고정.
	//
	// Parameters: (char *)데이타 포인터. (int)크기.
	// Return: (int)가져온 크기.
	/////////////////////////////////////////////////////////////////////////
	int Peek(unsigned char *pDest, int iSize);


	/////////////////////////////////////////////////////////////////////////
	// 원하는 길이만큼 읽기위치 에서 삭제 / 쓰기 위치 이동
	//
	// Parameters: 없음.
	// Return: 이동한 값.
	/////////////////////////////////////////////////////////////////////////
	int MoveRear(int iSize);
	int MoveFront(int iSize);

	/////////////////////////////////////////////////////////////////////////
	// 버퍼의 모든 데이타 삭제.
	//
	// Parameters: 없음.
	// Return: 없음.
	/////////////////////////////////////////////////////////////////////////
	void ClearBuffer(void);

	/////////////////////////////////////////////////////////////////////////
	// 버퍼의 포인터 얻음.
	//
	// Parameters: 없음.
	// Return: (char *) 버퍼 포인터.
	/////////////////////////////////////////////////////////////////////////
	char *GetBufferPtr(void);

	/////////////////////////////////////////////////////////////////////////
	// 버퍼의 Front 포인터 얻음.
	//
	// Parameters: 없음.
	// Return: (char *) 버퍼 포인터.
	/////////////////////////////////////////////////////////////////////////
	char *GetFrontBufferPtr(void);


	/////////////////////////////////////////////////////////////////////////
	// 버퍼의 RearPos 포인터 얻음.
	//
	// Parameters: 없음.
	// Return: (char *) 버퍼 포인터.
	/////////////////////////////////////////////////////////////////////////
	char *GetRearBufferPtr(void);


	/////////////////////////////////////////////////////////////////////////
	// 링버퍼의 현재 정보 화면에 출력
	// 	   * 테스트 전용 
	// 	   * 알아서 바꿔서 사용하기
	//
	// Parameters: 없음.
	// Return: 없음.
	/////////////////////////////////////////////////////////////////////////
	void PrintfInfo(void);

public:
	/////////////////////////////////////////////////////////////////////////
	// 링버퍼의 Lock
	//
	// Parameters: bool (읽기 전용 : true).
	// Return: 없음.
	/////////////////////////////////////////////////////////////////////////
	void Lock(bool bReadonly = false);

	/////////////////////////////////////////////////////////////////////////
	// 링버퍼의 Unlock
	//
	// Parameters: bool (읽기 전용 : true).
	// Return: 없음.
	/////////////////////////////////////////////////////////////////////////
	void Unlock(bool bReadonly = false);

};

