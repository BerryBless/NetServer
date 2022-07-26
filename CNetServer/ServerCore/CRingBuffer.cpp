#include "pch.h"
#include "CRingBuffer.h"
#include <stdio.h>
#define BLINK 1 // 큐가 다찼는지 확인할 공간 (실제 큐사이즈 = _iBufferSize - BLINK)


CRingBuffer::CRingBuffer() {
	// 기본 생성자
	// 기본크기 10000 byte
	this->ClearBuffer();
	_iBufferSize = dfRINGBUFFERSIZE;
	_buffer = new char[_iBufferSize];

	// SRWLock 초기화
	InitializeSRWLock(&_srwlock);
}

CRingBuffer::CRingBuffer(int iSize) {
	// 버퍼 크기 지정 생성자
	this->ClearBuffer();
	_iBufferSize = iSize;
	_buffer = new char[_iBufferSize];

	// SRWLock 초기화
	InitializeSRWLock(&_srwlock);
}

CRingBuffer::~CRingBuffer() {
	// 버퍼 소멸자
	delete[] _buffer;
}

int CRingBuffer::GetBufferSize(void) {
	// 버퍼 실제 크기 리턴
	return _iBufferSize - BLINK;
}

int CRingBuffer::GetUseSize(void) {
	if (_front <= _rear) {
		// □□□□□■■■■■■■■□□□□□
		// 0    F^^^^^^^r
		return  _rear - _front;
	}
	// ■■■■□□□□□□□□□□■■■■
	// 0^^^r         F^^^
	return (_iBufferSize - _front) + _rear;
}

int CRingBuffer::GetFreeSize(void) {
	// 넣을 수 있는 공간
	return _iBufferSize - GetUseSize() - BLINK;
}

int CRingBuffer::DirectEnqueueSize(void) {
	// 끊기지 않고 넣을 수 있는 공간
	if (_front <= _rear) {
		if (_front < BLINK) {
			// ■■■■■■■■□□□□□□□□□□
			// f       R^^^^^^^^  <- 마지막 한칸은 BLINK
			return (_iBufferSize - _rear) - (BLINK - _front);
		} else {
			// □□□□□■■■■■■■■□□□□□
			// 0    f       R^^^^
			return _iBufferSize - _rear;
		}
	}
	// ■■■■□□□□□□□□□□■■■■
	// 0   R^^^^^^^^ f		
	return   _front - _rear - BLINK;
}

int CRingBuffer::DirectDequeueSize(void) {
	// 끊기지 않고 뺄 수 있는 데이터
	if (_front <= _rear) {
		// □□□□□■■■■■■■■□□□□□
		// 0    F^^^^^^^r
		return _rear - _front;
	}
	// ■■■■□□□□□□□□□□■■■■
	// 0   r         F^^^
	return _iBufferSize - _front;
}

int CRingBuffer::Enqueue(const unsigned char *pData, int iSize) {
	int iEnq; // 끊기지 않고 한번에 넣을 수 있는 사이즈

	if (pData == nullptr) {
		// 널포인터 들어왔으면 결함
		CRASH();
	}

	if (iSize <= 0) {
		// 이상한값!
		// iSize는 절대 0이하가 되면 안됨!
		return 0;
	}

	if (GetFreeSize() < iSize) {
		// iSize만큼 Enqueue 할 수 없는 상황
		//iSize = GetFreeSize();
		return 0;
	}

	if (_front <= _rear) {
		// □□□□□■■■■■■■■□□□□□
		// 0    f       r

		iEnq = DirectEnqueueSize(); // 한번에 넣을 사이즈 확인
		if (iEnq >= iSize) {
			// □□□□□■■■■■■■■□□□□□
			// 0    f       R^^^^  한번에 넣기 충분할경우
			memcpy_s(_buffer + _rear, _iBufferSize, pData, iSize);
			_rear += iSize;
		} else {
			// □□□□□■■■■■■■■□□□□□
			// 0^^  f       R^^^^  두번에 나눠서 넣어야 할경우
			memcpy_s(_buffer + _rear, _iBufferSize, pData, iEnq);			// 일단 끝까지
			memcpy_s(_buffer, _iBufferSize, pData + iEnq, iSize - iEnq);	// 남는부분까지
			_rear = iSize - iEnq;
		}
	} else {
		// ■■■■□□□□□□□□□□■■■■
		// 0   R^^^^^^   f		
		memcpy_s(_buffer + _rear, _iBufferSize, pData, iSize);
		_rear += iSize;
	}

	if (_rear == _iBufferSize) {
		// 끝에 걸치면 맨앞으로
		_rear = 0;
	}

	return iSize;
}

int CRingBuffer::Dequeue(unsigned char *pDest, int iSize) {
	int iDeq;	// 끊기지 않고 한번에 뺄 수 있는 크기

	if (pDest == nullptr) {
		// 널포인터 들어왔으면 결함
		CRASH();
	}

	if (iSize <= 0) {
		// 이상한값!
		// iSize는 절대 0이하가 되면 안됨!
		return 0;
	}

	if (GetUseSize() < iSize) {
		// iSize만큼 Dequeue 할 수 없는 상황
		return 0;
	}

	if (_front <= _rear) {
		// □□□□□■■■■■■■■□□□□□
		// 0    F^^^^^  r
		memcpy_s(pDest, iSize, _buffer + _front, iSize);
		_front += iSize;
	} else {
		iDeq = DirectDequeueSize(); // 한번에 뺄 사이즈 확인

		if (iDeq >= iSize) {
			// ■■■■□□□□□□□□□□■■■■
			// 0   r         F^^	// 한번에 빼기 가능할 경우
			memcpy_s(pDest, iSize, _buffer + _front, iSize);
			_front += iSize;
		} else {
			// ■■■■□□□□□□□□□□■■■■
			// 0^  r         F^^^	// 두번에 나눠서 뺄경우
			memcpy_s(pDest, iSize, _buffer + _front, iDeq);
			memcpy_s(pDest + iDeq, iSize, _buffer, iSize - iDeq);
			_front = iSize - iDeq;
		}
	}

	if (_front == _iBufferSize) {
		// 끝에 걸치면 맨앞으로
		_front = 0;
	}

	return iSize;
}


int CRingBuffer::Peek(unsigned char *pDest, int iSize) {
	// Dequeue와 알고리즘은 같지만 _front가 움직이지는 않는다!

	int iDeq;	// 끊기지 않고 한번에 뺄 수 있는 크기

	if (pDest == nullptr) {
		// 널포인터 들어왔으면 결함
		CRASH();
	}

	if (iSize <= 0) {
		// 이상한값!
		// iSize는 절대 0이하가 되면 안됨!
		return 0;
	}

	if (GetUseSize() < iSize) {
		// iSize만큼 Peek 할 수 없는 상황
		// 최대 사이즈만큼 Peek
		//iSize = GetUseSize();
		return 0;
	}

	if (_front <= _rear) {
		// □□□□□■■■■■■■■□□□□□
		// 0    F^^^^^  r
		memcpy_s(pDest, iSize, _buffer + _front, iSize);
	} else {
		iDeq = DirectDequeueSize(); // 한번에 뺄 사이즈 확인

		if (iDeq >= iSize) {
			// ■■■■□□□□□□□□□□■■■■
			// 0   r         F^^	// 한번에 빼기 가능할 경우
			memcpy_s(pDest, iSize, _buffer + _front, iSize);
		} else {
			// ■■■■□□□□□□□□□□■■■■
			// 0^  r         F^^^	// 두번에 나눠서 뺄경우
			memcpy_s(pDest, iSize, _buffer + _front, iDeq);
			memcpy_s(pDest + iDeq, iSize, _buffer, iSize - iDeq);
		}
	}

	return iSize;
}

int CRingBuffer::MoveRear(int iSize) {
	int iEnq;
	if (iSize <= 0) {
		// 이상한값?
		// UNDO를 할정도면 애초에 큐에 넣지 말아야함
		return 0;
	}
	if (_front <= _rear) {
		// □□□□□■■■■■■■■□□□□□
		// 0    f       r

		iEnq = DirectEnqueueSize(); // 한번에 넣을 사이즈 확인
		if (iEnq >= iSize) {
			// □□□□□■■■■■■■■□□□□□
			// 0    f       R^^^^  한번에 넣기 충분할경우
			_rear += iSize;
		} else {
			// □□□□□■■■■■■■■□□□□□
			// 0^^  f       R^^^^  두번에 나눠서 넣어야 할경우
			_rear = iSize - iEnq;
		}
	} else {
		// ■■■■□□□□□□□□□□■■■■
		// 0   R^^^^^^   f		
		_rear += iSize;
	}

	if (_rear == _iBufferSize) {
		// 끝에 걸치면 맨앞으로
		_rear = 0;
	}
	return iSize;
}

int CRingBuffer::MoveFront(int iSize) {
	int iDeq;

	if (iSize <= 0) {
		// 이상한값?
		return 0;
	}

	if (_front <= _rear) {
		// □□□□□■■■■■■■■□□□□□
		// 0    F^^^^^  r
		_front += iSize;
	} else {
		iDeq = DirectDequeueSize(); // 한번에 뺄 사이즈 확인
		if (iDeq >= iSize) {
			// ■■■■□□□□□□□□□□■■■■
			// 0   r         F^^^	
			_front += iSize;
		} else {
			// ■■■■□□□□□□□□□□■■■■
			// 0^  r         F^^^	
			_front = iSize - iDeq;
		}
	}

	if (_front == _iBufferSize) {
		// 끝에 걸치면 맨앞으로
		_front = 0;
	}
	return iSize;
}

void CRingBuffer::ClearBuffer(void) {
	// 초기화
	_front = 0;
	_rear = 0;
}

char *CRingBuffer::GetBufferPtr(void) {
	return _buffer;
}

char *CRingBuffer::GetFrontBufferPtr(void) {
	return _buffer + _front;
}

char *CRingBuffer::GetRearBufferPtr(void) {
	return _buffer + _rear;
}

void CRingBuffer::PrintfInfo(void) {
	char buffer[dfRINGBUFFERSIZE] = { 0 };
	if (_rear >= _front) {
		memcpy_s(buffer, dfRINGBUFFERSIZE, _buffer + _front, _rear - _front);
	} else {
		int dSize = DirectDequeueSize();
		memcpy_s(buffer, dfRINGBUFFERSIZE, _buffer + _front, dSize);
		memcpy_s(buffer, dfRINGBUFFERSIZE, _buffer, _rear);
	}

	printf_s("\n\nsize [%d], front [%d], rear [%d], buffer[%s]\n\n", GetUseSize(), _front, _rear, buffer);
}

void CRingBuffer::Lock(bool bReadonly) {
	if (bReadonly == true) {
		// 읽기 전용
		AcquireSRWLockShared(&_srwlock);
	} else {
		// 쓰기 전용
		AcquireSRWLockExclusive(&_srwlock);
	}
}

void CRingBuffer::Unlock(bool bReadonly) {
	if (bReadonly == true) {
		// 읽기 전용
		ReleaseSRWLockShared(&_srwlock);
	} else {
		// 쓰기 전용
		ReleaseSRWLockExclusive(&_srwlock);
	}
}

