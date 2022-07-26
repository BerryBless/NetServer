/*
* 1. 특정 프로세스 핸들 얻음
2. lpMinimumApplicationAddress 메모리 시작주소 부터 VirtualQueryEx 로 정보 얻음
3. 얻은 메모리(페이지) 의 속성 개인메모리,커밋 확인
3-1. RegionSize 메모리 확보
3-2. 확보한 메모리로 ReadProcessMemory 로 RegionSize 만큼 읽어들임
3-3. 읽어들인 메모리를 뒤져서 원하는 값 탐색
3-4. 원하는 값을 찾았다면, 해당 메모리의 위치를 BaseAddress 기준으로 메모리 계산.
3-5. WriteProcessMemory 로 해당위치 값 변경
4. 확인 메모리 포인터를 ResionSize 만큼 다음으로 이동
5. lpMaximumApplicationAddress 를 벗어났다면 중단.
6. 아니면 2번으로 반복 ~
*/

/* 
* TODO :
* 메모리 찾는 방식 바꾸기 - 4바이트씩 텅텅 움직이는게 아니라 1바이트씩 4바이트 비교
* 최적화 - 너무 구려
*/

#include <stdio.h>
#include <Windows.h>
#include <list>

int main() {
	int pid = 16688;
	int value = 123;
	printf_s("pid >> ");
	scanf_s("%d", &pid);
	printf_s("찾을 변수값(int) >>");
	scanf_s("%d", &value);

	// 프로세스ID로 프로세스 핸들 모든권한으로 가져오기
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);

	//* 후보군 찾기
	std::list<DWORD*> lCandidate;	// 후보군 리스트

	SYSTEM_INFO SystemInfo;			// 시스템 정보
	GetSystemInfo(&SystemInfo);		// 시스템 정보 얻기
	// 메모리 페이지를 돌아가며 변수 찾기
	for (DWORD pMemory = (DWORD)SystemInfo.lpMinimumApplicationAddress; pMemory < (DWORD)SystemInfo.lpMaximumApplicationAddress;) {
		// 해당 페이지의 메모리의 정보 얻기
		MEMORY_BASIC_INFORMATION MemoryBasicInfo;
		SIZE_T size = VirtualQueryEx(hProcess, (LPCVOID)pMemory, &MemoryBasicInfo, sizeof(MemoryBasicInfo));

		// 그 페이지가 커밋된 상태, 해당프로세스에 종속(? 애매한 한국말) 된 상태
		// MEM_PRIVATE = Indicates that the memory pages within the region are private (that is, not shared by other processes).
		if (MemoryBasicInfo.Type == MEM_PRIVATE && MemoryBasicInfo.State == MEM_COMMIT) {
			//printf_s("%p : 커밋된 페이지 size(%p)\n", MemoryBasicInfo.BaseAddress, MemoryBasicInfo.RegionSize);

			// 페이지 크기만큼 버퍼 할당 
			BYTE* buffer = (BYTE*)malloc(sizeof(BYTE) * MemoryBasicInfo.RegionSize);
			// 버퍼에 한번에 페이지 정보 가져옴
			if (ReadProcessMemory(hProcess, MemoryBasicInfo.BaseAddress, buffer, sizeof(BYTE) * MemoryBasicInfo.RegionSize, NULL)) {
				//printf_s("메모리 읽기 성공!\n\n");

				// int 변수를 찾을꺼야!
				int* pIntVar = (int*)buffer;

				// 메모리 정보를 순회하며 원하는 변수 값이 있는 메모리주소 찾기
				for (int i = 0; i < MemoryBasicInfo.RegionSize / 4; i++) {
					// (p + i)는 이 프로세스의 주소, 해당 프로세스의 주소를 얻을려면 MemoryBasicInfo.BaseAddress + i
					if (*(pIntVar + i) == value) {
						//printf_s("%p :: %d\n", (DWORD*)MemoryBasicInfo.BaseAddress + i,value);
						lCandidate.push_back((DWORD*)MemoryBasicInfo.BaseAddress + i);
					}
				}
			}
			// 버퍼 할당 해제
			free(buffer);
		}
		// 다음 페이지 탐색!
		pMemory = pMemory + MemoryBasicInfo.RegionSize;
	}//*/

	//* 후보군에서 변수 바꾸며 솎아내기
	// 1개의 후보가 나올때까지 계속
	while (lCandidate.size() > 1)	{
		// 후보군 출력
		for (auto itr = lCandidate.begin(); itr != lCandidate.end(); ++itr) {
			printf_s("0x%p\n", *itr);
		}

		printf_s("찾을 변수값(int) >>");
		scanf_s("%d", &value);
		DWORD buffer;
		
		//* 후보군에서 찾기
		// 후보군 순회!
		auto itr = lCandidate.begin();
		while (itr != lCandidate.end()) {
			if (ReadProcessMemory(hProcess, (LPCVOID)*itr, &buffer, sizeof(buffer), NULL)) {
				if (buffer == value) {
				// 찾는 변수의 후보군!
					++itr;
					continue;
				}
			}
			// 버리는 후보
			itr = lCandidate.erase(itr);
		}
	}
	// 리스트 정리
	DWORD* pMemoryPos = *lCandidate.begin();
	lCandidate.clear();

	printf_s("메모리 찾기 완료 : 0x%p\n", pMemoryPos);
	//*/

	//* 메모리 값 바꾸기
	printf_s("바꿀 변수값(int) >>");
	scanf_s("%d", &value);

	// 찾은 후보를 입력받은 변수로 바꾸기
	if (WriteProcessMemory(hProcess, (LPVOID)pMemoryPos, &value, sizeof(value), NULL)) {
		printf_s("메모리 갱신 성공!");
	}
	//*/


	return 0;
}