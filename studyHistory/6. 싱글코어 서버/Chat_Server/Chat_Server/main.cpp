#include "NetworkCore.h"

#include <locale.h>
int main() {

	_wsetlocale(LC_ALL, L"korean");// 한국어 설정
	// 네트워크 초기화
	NetworkInitServer();

	while (true) {
		// 1회 회전 = 1프레임
		if (NetworkPorc() == false) {
			break;
		}
	}

	// 네트워크 정리
	NetworkCloseServer();
	return 0;
}