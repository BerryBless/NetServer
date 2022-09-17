#include"pch.h"
#include "SerializingBuffer.h"
#include <malloc.h>
#include <string.h>
#include <Windows.h>
#include "ObjectPool_TLS.hpp"
#ifndef CRASH
#define CRASH() do{\
	_LOG(dfLOG_LEVEL_ERROR, L"///////CRASH : FILE[%s] Line[%d]",__FILEW__,__LINE__);\
	int *nptr = nullptr; *nptr = 1;\
}while(0)
#endif // !CRASH

ObjectPool_TLS<Packet> Packet::_packetPool(true);
//ObjectPool<Packet> Packet::_packetPool(0,true);

Packet::Packet() {
	// 기본 사이즈만큼 할당
	_iBufferSize = eBUFFER_DEFAULT;
	//_pBuffer = new char[_iBufferSize];
	Clear();
}

Packet::~Packet() {
	Release();
}

void Packet::Release(void) {
	/*if (_pBuffer != nullptr)
		delete[]_pBuffer;*/
}

void Packet::Clear(void) {
	_readPos = sizeof(NET_HEADER);
	_writePos = sizeof(NET_HEADER);
	_sendPos = 0;
}


int Packet::MoveWritePos(int iSize) {
	if (iSize > _iBufferSize - _writePos) {
		iSize = _iBufferSize - _writePos;
	}
	_writePos += iSize;
	return iSize;
}

int Packet::MoveReadPos(int iSize) {
	if (iSize > GetDataSize()) {
		iSize = GetDataSize();
	}
	_readPos += iSize;
	return iSize;
}

void Packet::SetLanHeader() {
	unsigned short len = GetDataSize();
	_sendPos = _readPos - sizeof(LAN_HEADER);
	// 헤더 내용물 넣어주기
	((LAN_HEADER *) (_pBuffer + _sendPos))->len = len;
}

void Packet::SetNetHeader() {
	if (_isEncode) return;
	_isEncode = true;
	_sendPos = _readPos - sizeof(NET_HEADER);

	NET_HEADER header{ 0 };
	header.code = PACKET_CODE;
	header.len = GetDataSize();
	header.randKey = (BYTE) rand(); // 0x31;

	int checksum = 0;
	for (int i = _readPos; i < _writePos; ++i) {
		unsigned char *temp = (unsigned char *) (_pBuffer + i);
		checksum += *temp;
	}
	header.checksum = checksum % 256;

	memcpy_s(_pBuffer + _sendPos, sizeof(NET_HEADER), &header, sizeof(NET_HEADER));
	Encode();
}

void Packet::Encode() {
	unsigned char *temp = _pBuffer + _readPos - 1;
	int len = GetDataSize() + 1;
	int randKey = ((NET_HEADER *) _pBuffer)->randKey;

	BYTE P = 0;
	BYTE E = 0;
	BYTE D;

	for (int i = 1; i <= len; ++i) {
		D = *temp;
		P = D ^ (P + randKey + i);
		E = P ^ (E + FIXED_KEY + i);
		*temp = E;
		temp++;
	}

}

bool Packet::Decode() {
	unsigned char *temp = _pBuffer + _readPos - 1;
	NET_HEADER header = *((NET_HEADER *) _pBuffer);
	int len = GetDataSize() + 1;
	BYTE randKey = _pBuffer[3];

	BYTE curP;
	BYTE prevP = 0;
	BYTE curE;
	BYTE prevE = 0;
	BYTE D;

	for (int i = 1; i <= len; ++i) {
		curE = (BYTE) *temp;
		curP = curE ^ (prevE + FIXED_KEY + i);
		D = curP ^ (prevP + randKey + i);

		*temp = D;
		prevE = curE;
		prevP = curP;

		temp++;
	}

	return VerifyCheckSum();
}

bool Packet::VerifyCheckSum() {
	int checksum = _pBuffer[4];
	int sum = 0;
	for (int i = _readPos; i < _writePos; ++i) {
		sum += *(_pBuffer + i);
	}
	sum %= 256;
	return sum == checksum;
}


void Packet::PrintPacket() {
	printf_s("\nPacket Start\n");
	for (int i = 0; i < _writePos; ++i) {
		printf_s("%02x ", _pBuffer[i]);
	}
	printf_s("\nPacket End\n");
}

inline Packet &Packet::operator=(Packet &clSrPacket) {
	// 버퍼 내용물 복사
	//this->Clear();
	this->_readPos = clSrPacket.GetReadPtr() - clSrPacket.GetBufferPtr();
	this->_writePos = clSrPacket.GetWritePtr() - clSrPacket.GetBufferPtr();
	memcpy_s(this->_pBuffer, this->_iBufferSize, clSrPacket.GetBufferPtr(), this->_writePos);
	return *this;
}
/*
inline Packet &Packet::operator<<(unsigned char byValue) {
	memcpy_s(_pBuffer + _writePos, _iBufferSize, &byValue, sizeof(byValue));
	_writePos += sizeof(byValue);
	return *this;
}

inline Packet &Packet::operator<<(char chValue) {
	memcpy_s(_pBuffer + _writePos, _iBufferSize, &chValue, sizeof(chValue));
	_writePos += sizeof(chValue);
	return *this;
}

inline Packet &Packet::operator<<(short shValue) {
	memcpy_s(_pBuffer + _writePos, _iBufferSize, &shValue, sizeof(shValue));
	_writePos += sizeof(shValue);
	return *this;
}

inline Packet &Packet::operator<<(unsigned short wValue) {
	memcpy_s(_pBuffer + _writePos, _iBufferSize, &wValue, sizeof(wValue));
	_writePos += sizeof(wValue);
	return *this;
}

inline Packet &Packet::operator<<(int iValue) {
	memcpy_s(_pBuffer + _writePos, _iBufferSize, &iValue, sizeof(iValue));
	_writePos += sizeof(iValue);
	return *this;
}

inline Packet &Packet::operator<<(unsigned long dwValue) {
	memcpy_s(_pBuffer + _writePos, _iBufferSize, &dwValue, sizeof(dwValue));
	_writePos += sizeof(dwValue);
	return *this;
}

inline Packet &Packet::operator<<(float fValue) {
	memcpy_s(_pBuffer + _writePos, _iBufferSize, &fValue, sizeof(fValue));
	_writePos += sizeof(fValue);
	return *this;
}

inline Packet &Packet::operator<<(__int64 iValue) {
	memcpy_s(_pBuffer + _writePos, _iBufferSize, &iValue, sizeof(iValue));
	_writePos += sizeof(iValue);
	return *this;
}

inline Packet &Packet::operator<<(double dValue) {
	memcpy_s(_pBuffer + _writePos, _iBufferSize, &dValue, sizeof(dValue));
	_writePos += sizeof(dValue);
	return *this;
}

inline Packet &Packet::operator>>(unsigned char &byValue) {
	memcpy_s(&byValue, sizeof(byValue), _pBuffer + _readPos, sizeof(byValue));
	_readPos += sizeof(byValue);
	return *this;
}

inline Packet &Packet::operator>>(char &chValue) {
	memcpy_s(&chValue, sizeof(chValue), _pBuffer + _readPos, sizeof(chValue));
	_readPos += sizeof(chValue);
	return *this;
}

inline Packet &Packet::operator>>(short &shValue) {
	memcpy_s(&shValue, sizeof(shValue), _pBuffer + _readPos, sizeof(shValue));
	_readPos += sizeof(shValue);
	return *this;
}

inline Packet &Packet::operator>>(unsigned short &wValue) {
	memcpy_s(&wValue, sizeof(wValue), _pBuffer + _readPos, sizeof(wValue));
	_readPos += sizeof(wValue);
	return *this;
}

inline Packet &Packet::operator>>(int &iValue) {
	memcpy_s(&iValue, sizeof(iValue), _pBuffer + _readPos, sizeof(iValue));
	_readPos += sizeof(iValue);
	return *this;
}

inline Packet &Packet::operator>>(unsigned long &dwValue) {
	memcpy_s(&dwValue, sizeof(dwValue), _pBuffer + _readPos, sizeof(dwValue));
	_readPos += sizeof(dwValue);
	return *this;
}

inline Packet &Packet::operator>>(float &fValue) {
	memcpy_s(&fValue, sizeof(fValue), _pBuffer + _readPos, sizeof(fValue));
	_readPos += sizeof(fValue);
	return *this;
}

inline Packet &Packet::operator>>(__int64 &iValue) {
	memcpy_s(&iValue, sizeof(iValue), _pBuffer + _readPos, sizeof(iValue));
	_readPos += sizeof(iValue);
	return *this;
}

inline Packet &Packet::operator>>(double &dValue) {
	memcpy_s(&dValue, sizeof(dValue), _pBuffer + _readPos, sizeof(dValue));
	_readPos += sizeof(dValue);
	return *this;
}//*/

int Packet::GetData(char *chpDest, int iSize) {
	if (iSize > GetDataSize()) {
		// iSize만큼 못빼면 그냥 안빼기
		return 0;
	}
	memcpy_s(chpDest, iSize, _pBuffer + _readPos, iSize);
	_readPos += iSize;
	return iSize;
}

int Packet::PutData(char *chpSrc, int iSrcSize) {
	if (iSrcSize > _iBufferSize - _writePos) {
		// iSize만큼 못넣으면 그냥 안넣기
		return 0;
	}
	memcpy_s(_pBuffer + _writePos, _iBufferSize, chpSrc, iSrcSize);
	_writePos += iSrcSize;
	return iSrcSize;
}

Packet *Packet::AllocAddRef() {
	Packet *pPacket;


#ifndef dfALLOCATOR
	pPacket = _packetPool.Alloc();
#else
	pPacket = new Packet;
#endif // dfPOOLALLOC
	pPacket->ResetRef();
	return pPacket;
}

void Packet::AddRef(int logic) {
#ifdef df_LOGGING_PACKET_COUNTER_LOGIC
	// LOGGING
	int idx = InterlockedIncrement(&_addlogidx);
	if (idx >= 99)
		_addlogidx = 0;
	_logAddLogic[idx] = logic;
#endif // df_LOGGING_PACKET_COUNTER_LOGIC
	InterlockedIncrement((long *) &_refCount.counter);
}


void Packet::SubRef(int logic) {
#ifdef df_LOGGING_PACKET_COUNTER_LOGIC
	// LOGGING
	int idx = InterlockedIncrement(&_sublogidx);
	if (idx >= 99)
		_sublogidx = 0;
	_logSubLogic[idx] = logic;
#endif // df_LOGGING_PACKET_COUNTER_LOGIC
	//Subref 
	InterlockedDecrement((long *) &_refCount.counter);
	RefCount tempRef;
	tempRef.counter = 0;
	tempRef.refStaus.isFreed = 1;

	if (InterlockedCompareExchange(&_refCount.counter, tempRef.counter, 0) == 0) {
		Clear();
#ifndef dfALLOCATOR
		_packetPool.Free(this);
#else
		delete this;
#endif // dfPOOLALLOC
	}
}

void Packet::ResetRef() {
	_refCount.refStaus.count = 1;
	_refCount.refStaus.isFreed = 0;
}
