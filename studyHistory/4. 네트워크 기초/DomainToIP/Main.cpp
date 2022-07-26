#pragma comment(lib, "ws2_32")
#include <WinSock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <locale.h>
////////////////////
// 도메인을 IP주소로 바꿈니다
// --------
// 사용법
//
//SOCKADDR_IN SockAddr;
//IN_ADDR Addr;
//memset(&SockAddr, 0, sizeof(SockAddr));
//DomainToIP(L"google.com", &Addr);
//
//SockAddr.sin_family = AF_INET;
//SockAddr.sin_addr = Addr;
//SockAddr.sin_port = htons(80);
////////////////////
BOOL DomainToIP(WCHAR* szDomain, IN_ADDR* pAddr) {
	ADDRINFOW* pAddrInfo;	// IP정보
	SOCKADDR_IN* pSockAddr;
	// pAddrInfo 에 도메인을 IP로 변환한것을 "리스트로 변환 해줌" (이중포인터)
	// 외부에서 반드시 해재 해줘야함!

	if (GetAddrInfo(szDomain, L"0", NULL, &pAddrInfo) != 0) {
		return FALSE;
	}
	pSockAddr = (SOCKADDR_IN*)pAddrInfo->ai_addr;
	*pAddr = pSockAddr->sin_addr;

	FreeAddrInfo(pAddrInfo); // pAddrInfo 리스트 할당 해재!!!!!!
	return TRUE;
}

int DomainToIP(WCHAR* szDomain, IN_ADDR* pAddr, int size)
{
	ADDRINFOW* pAddrInfo;
	SOCKADDR_IN* pSockAddr;
	if (GetAddrInfo(szDomain, L"0", NULL, &pAddrInfo) != 0)
	{
		return -1;
	}

	int count;

	for (count = 0; count < size; ++count) {
		if (pAddrInfo == nullptr) {
			break;
		}

		pSockAddr = (SOCKADDR_IN*)pAddrInfo->ai_addr;
		pAddr[count] = pSockAddr->sin_addr;
		pAddrInfo = pAddrInfo->ai_next;
	}

	FreeAddrInfo(pAddrInfo);

	return count;
}

int main() {
	setlocale(LC_ALL, "");
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
		return 1;
	}

	WCHAR domain[] = L"shsuzakfn.gihyeon.com";
	IN_ADDR addr;
	WCHAR IPstr[128];

	DomainToIP(domain, &addr);

	InetNtop(AF_INET, &addr, IPstr, INET_ADDRSTRLEN);

	wprintf_s(L"%s (%s)",domain, IPstr);

	WSACleanup();
	return 0;
}