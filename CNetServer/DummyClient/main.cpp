#include "pch.h"
#include "CEchoClient.h"

#define dfDUMMY_CNT 1000

int main() {
	CEchoClient *pDummy = new CEchoClient[dfDUMMY_CNT];

	for (int i = 0; i < dfDUMMY_CNT; ++i) {
		pDummy[i].Connect(L"127.0.0.1", 6000);
	}


	int cnt = 0;
	while (1)
	{
		CPacket *pPacket = CPacket::AllocAddRef();

		*pPacket << cnt++;

		pPacket->SetHeader();
		for (int i = 0; i < dfDUMMY_CNT; i++)
			pDummy[i].SendPacket(pPacket);
		pPacket->SubRef();
	}

	return 0;
}