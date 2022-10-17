#include "CClient.h"

void CClient::Init(DWORD ID, SOCKET sock, SOCKADDR_IN addr) {
	_ID = ID;
	_sock = sock;
	_Addr = addr;
}
