#include "CPacket.h"
#include <malloc.h>
#include <string.h>
#include <Windows.h>

CPacket::CPacket() {
	// 기본 사이즈만큼 할당
	_iBufferSize = eBUFFER_DEFAULT;
	_pBuffer = new char[_iBufferSize];
	Clear();
}

CPacket::CPacket(int iBufferSize) {
	// 할당된 사이즈 만큼 할당
	_iBufferSize = iBufferSize;
	_pBuffer = new char[_iBufferSize];
	Clear();
}

CPacket::~CPacket() {
	Release();
}

void CPacket::Release(void) {
	delete[]_pBuffer;
}

void CPacket::Clear(void) {
	_readPos = sizeof(HEADER);
	_writePos = sizeof(HEADER);
	_sendPos = 0;
}

int CPacket::GetSendSize(void) {
	return _writePos - _readPos + sizeof(HEADER);
}

int CPacket::GetDataSize(void) {
	return _writePos - _readPos;
}

int CPacket::MoveWritePos(int iSize) {
	if (iSize > _iBufferSize - _writePos) {
		iSize = _iBufferSize - _writePos;
	}
	_writePos += iSize;
	return iSize;
}

int CPacket::MoveReadPos(int iSize) {
	if (iSize > GetDataSize()) {
		iSize = GetDataSize();
	}
	_readPos += iSize;
	return iSize;
}

void CPacket::SetHeader() {
	unsigned short len = GetDataSize();
	_sendPos = _readPos - sizeof(HEADER);
	((HEADER *) (_pBuffer + _sendPos))->len = len;
}

CPacket &CPacket::operator=(CPacket &clSrcPacket) {
	// 버퍼 내용물 복사
	this->Clear();
	this->_writePos = clSrcPacket.GetDataSize();
	memcpy_s(this->_pBuffer, this->_iBufferSize, clSrcPacket.GetBufferPtr(), this->_writePos);
	return *this;
}

CPacket &CPacket::operator<<(unsigned char byValue) {
	memcpy_s(_pBuffer + _writePos, _iBufferSize, &byValue, sizeof(byValue));
	_writePos += sizeof(byValue);
	return *this;
}

CPacket &CPacket::operator<<(char chValue) {
	memcpy_s(_pBuffer + _writePos, _iBufferSize, &chValue, sizeof(chValue));
	_writePos += sizeof(chValue);
	return *this;
}

CPacket &CPacket::operator<<(short shValue) {
	memcpy_s(_pBuffer + _writePos, _iBufferSize, &shValue, sizeof(shValue));
	_writePos += sizeof(shValue);
	return *this;
}

CPacket &CPacket::operator<<(unsigned short wValue) {
	memcpy_s(_pBuffer + _writePos, _iBufferSize, &wValue, sizeof(wValue));
	_writePos += sizeof(wValue);
	return *this;
}

CPacket &CPacket::operator<<(int iValue) {
	memcpy_s(_pBuffer + _writePos, _iBufferSize, &iValue, sizeof(iValue));
	_writePos += sizeof(iValue);
	return *this;
}

CPacket &CPacket::operator<<(unsigned long dwValue) {
	memcpy_s(_pBuffer + _writePos, _iBufferSize, &dwValue, sizeof(dwValue));
	_writePos += sizeof(dwValue);
	return *this;
}

CPacket &CPacket::operator<<(float fValue) {
	memcpy_s(_pBuffer + _writePos, _iBufferSize, &fValue, sizeof(fValue));
	_writePos += sizeof(fValue);
	return *this;
}

CPacket &CPacket::operator<<(__int64 iValue) {
	memcpy_s(_pBuffer + _writePos, _iBufferSize, &iValue, sizeof(iValue));
	_writePos += sizeof(iValue);
	return *this;
}

CPacket &CPacket::operator<<(double dValue) {
	memcpy_s(_pBuffer + _writePos, _iBufferSize, &dValue, sizeof(dValue));
	_writePos += sizeof(dValue);
	return *this;
}

CPacket &CPacket::operator>>(unsigned char &byValue) {
	memcpy_s(&byValue, sizeof(byValue), _pBuffer + _readPos, sizeof(byValue));
	_readPos += sizeof(byValue);
	return *this;
}

CPacket &CPacket::operator>>(char &chValue) {
	memcpy_s(&chValue, sizeof(chValue), _pBuffer + _readPos, sizeof(chValue));
	_readPos += sizeof(chValue);
	return *this;
}

CPacket &CPacket::operator>>(short &shValue) {
	memcpy_s(&shValue, sizeof(shValue), _pBuffer + _readPos, sizeof(shValue));
	_readPos += sizeof(shValue);
	return *this;
}

CPacket &CPacket::operator>>(unsigned short &wValue) {
	memcpy_s(&wValue, sizeof(wValue), _pBuffer + _readPos, sizeof(wValue));
	_readPos += sizeof(wValue);
	return *this;
}

CPacket &CPacket::operator>>(int &iValue) {
	memcpy_s(&iValue, sizeof(iValue), _pBuffer + _readPos, sizeof(iValue));
	_readPos += sizeof(iValue);
	return *this;
}

CPacket &CPacket::operator>>(unsigned long &dwValue) {
	memcpy_s(&dwValue, sizeof(dwValue), _pBuffer + _readPos, sizeof(dwValue));
	_readPos += sizeof(dwValue);
	return *this;
}

CPacket &CPacket::operator>>(float &fValue) {
	memcpy_s(&fValue, sizeof(fValue), _pBuffer + _readPos, sizeof(fValue));
	_readPos += sizeof(fValue);
	return *this;
}

CPacket &CPacket::operator>>(__int64 &iValue) {
	memcpy_s(&iValue, sizeof(iValue), _pBuffer + _readPos, sizeof(iValue));
	_readPos += sizeof(iValue);
	return *this;
}

CPacket &CPacket::operator>>(double &dValue) {
	memcpy_s(&dValue, sizeof(dValue), _pBuffer + _readPos, sizeof(dValue));
	_readPos += sizeof(dValue);
	return *this;
}

int CPacket::GetData(char *chpDest, int iSize) {
	if (iSize > GetDataSize()) {
		// iSize만큼 못빼면 그냥 아빼기
		return 0;
	}
	memcpy_s(chpDest, iSize, _pBuffer + _readPos, iSize);
	_readPos += iSize;
	return iSize;
}

int CPacket::PutData(char *chpSrc, int iSrcSize) {
	if (iSrcSize > _iBufferSize - _writePos) {
		// iSize만큼 못넣으면 그냥 안넣기
		return 0;
	}
	memcpy_s(_pBuffer + _writePos, _iBufferSize, chpSrc, iSrcSize);
	_writePos += iSrcSize;
	return iSrcSize;
}

void CPacket::AddRef() {
	InterlockedIncrement(&_refCount);
}


void CPacket::SubRef() {
	InterlockedDecrement(&_refCount);

}

void CPacket::ResetRef() {
	_refCount = 0;
}
