#include <stdio.h>
#include "CPacket.h"
#include <Windows.h>
struct stTest {
	BYTE header;
	int iX;
	int iY;
};


class CBuffer : public CPacket {
public:
	CBuffer &operator << (BYTE byValue) {
		memcpy_s(_chpBuffer + _writePos, _iBufferSize, &byValue, sizeof(byValue));
		_writePos += sizeof(byValue);
		return *this;
	}
	CBuffer &operator >> (BYTE &byValue) {
		memcpy_s(&byValue, sizeof(byValue), _chpBuffer + _readPos, sizeof(byValue));
		_readPos += sizeof(byValue);
		return *this;
	}

	CBuffer &operator << (stTest stValue) {
		*this << stValue.header;
		CPacket::operator<< (stValue.iX); // ¸¶À½¿¡ ¾Èµê..
		CPacket::operator<< (stValue.iY);
		return *this;
	}
	CBuffer &operator >> (stTest &stValue) {
		*this >> stValue.header;
		CPacket::operator>> (stValue.iX);
		CPacket::operator>> (stValue.iY);

		return *this;
	}
};

void aa(CBuffer *buff) {
	stTest test;

	*buff >> test;
	printf_s("%d %d", test.iX, test.iY);
}

int main() {
	stTest test;
	test.header = 0x89;
	test.iX = 45;
	test.iY = 999;
	CBuffer buff;
	buff << test;

	aa(&buff);
	return 0;
}