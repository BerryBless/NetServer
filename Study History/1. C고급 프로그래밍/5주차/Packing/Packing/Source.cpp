/*
파일패킹 과정

1. 합치고자 하는 파일의 존재유무 확인.

2. 합치고자 하는 파일들의 이름,사이즈 얻기

3. 패킹파일을 열어서 stPACK_HEADER 헤더 저장

4. 패킹파일에 파일헤더 stPACK_FILEINFO 개수만큼 저장

5. 합칠 파일들을 하나씩 열면서 내용 패킹 파일로 저장



파일언패킹 과정

1. 패킹파일 오픈

2. 패킹파일 stPACK_HEADER 해더 읽기.  Type 확인 패킹파일 확인.

4. 파일개수 확인

5. 파일 개수만큼 stPACK_FILEINFO 헤더 읽기

6. 파일 개수만큼 돌면서 stPACK_FILEINFO[N] 의 정보로 파일생성

7. 사이즈만큼 읽어서 저장

8. 끝..

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define FILENAMELEN 64
#define INPUT
#define CHRASH() do{int* p = nullptr; *p=0;}while(0)

struct  Pack_Header {
	unsigned int type;
	int filecnt;
};
struct  Pack_Fileinfo {
	char fileName[FILENAMELEN];	// 파일 이름
	__int64 size;				// 현 파일의 사이즈
	__int64 volume;				// 현 파일공간의 사이즈
	__int64 offset;				// 파일포인터 시작지점
};
char* Strcpy(char* strDest, const char* strSrc) {
	// 문자열 시작부터 끝('\0')나올때까지 src에서 dest로 복사
	int len = 0;
	while (*(strSrc + len) != '\0') {
		*(strDest + len) = *(strSrc + len);
		len++;
	}
	*(strDest + len) = '\0';
	return strDest;
} // 문자열 복사

bool ReadHeader(Pack_Header& fixedHeader, Pack_Fileinfo** buffer, FILE* packedFile) {
	/* 파일의 헤더를 읽기!
	*/
	// 고정헤더 읽기
	if (fread_s(&fixedHeader, sizeof(fixedHeader), sizeof(fixedHeader), 1, packedFile) < 1) {
		printf_s("파일 읽기를 실패했습니다.");
		return false;
	}
	// 고정헤더 정보 패킹타입, 패킹된 파일 개수
	printf_s("Type : %d :: FILE COUNT(%d)\n", fixedHeader.type, fixedHeader.filecnt);

	// 파일 개수에 따른 파일정보헤더 버퍼 동적할당
	*buffer = (Pack_Fileinfo*)malloc(sizeof(Pack_Fileinfo) * fixedHeader.filecnt);

	// 파일 정보를 sizeof(Pack_Fileinfo) 만큼 fixedHeader.filecnt 번 나눠서 읽기
	if (fread_s(*buffer, sizeof(Pack_Fileinfo) * fixedHeader.filecnt, sizeof(Pack_Fileinfo), fixedHeader.filecnt, packedFile) < fixedHeader.filecnt) {
		printf_s("파일 읽기를 실패했습니다.");
		return false;
	}

	// 파일정보헤더 정보 출력
	for (int i = 0; i < fixedHeader.filecnt; i++) {
		printf_s("%d : FILENAME(%s), FILESIZE(%lld), FILEVOLUME(%lld), FILEOFFSET(%lld)\n",
			i, (*buffer + i)->fileName, (*buffer + i)->size, (*buffer + i)->volume, (*buffer + i)->offset);
	}
	return true;
} // 헤더 읽기

bool FileCopyAndPaste(FILE* fileDest, FILE* fileSrc, long long bufferSize) {
	// TODO : 렘용량(16gb) 이상의 대용양 파일 대응
	// FILE* fileDest, FILE* fileSrc 모두 바이너리로 열어야함
	// 버퍼 사이즈만큼 src에서 dest로 파일내용 복사
	char* buffer;
	buffer = (char*)malloc(sizeof(char) * bufferSize);
	// 버퍼에 파일정보 읽기
	if (fread_s(buffer, sizeof(char) * bufferSize, sizeof(char) * bufferSize, 1, fileSrc) < 1) {
		printf_s("파일읽기를 실패했습니다!");
		return false;
	}
	fwrite(buffer, sizeof(char) * bufferSize, 1, fileDest);
	free(buffer);
}// 파일 복붙

bool Packing() {
	/*
		* 패킹
		1. 합치고자 하는 파일의 존재유무 확인.

		2. 합치고자 하는 파일들의 이름,사이즈 얻기

		3. 패킹파일을 열어서 Pack_Header 헤더 저장

		4. 패킹파일에 파일헤더 Pack_Fileinfo 개수만큼 저장

		5. 합칠 파일들을 하나씩 열면서 내용 패킹 파일로 저장
		*/
	FILE* objectFile;			// 패킹할 파일
	FILE* packFile;				// 패킹목적지
	Pack_Header fixedHeader;	// 고정정보 헤더
	Pack_Fileinfo* fileInfoHeader; // 파일정보헤더 패킹할 파일만큼 할당
#ifdef INPUT
	char** fileName;
	char packName[FILENAMELEN] = "Pack.pack";

	// 입력부
	printf_s("패킹:: 타입, 패킹할 파일 개수를 입력해주세요\n");
	printf_s("패킹:: 타입 >>");
	scanf_s("%d", &fixedHeader.type);
	printf_s("패킹:: 갯수 >>");
	scanf_s("%d", &fixedHeader.filecnt);

	fileName = (char**)malloc(sizeof(char) * fixedHeader.filecnt);

	for (int i = 0; i < fixedHeader.filecnt; i++) {
		fileName[i] = (char*)malloc(sizeof(char) * FILENAMELEN);
		printf_s("%d : ", i + 1);
		scanf_s("%s", fileName[i]);
	}
#else
	char fileName[4][64] = { "1.png", "2.txt" , "3.xml", "4.mp3" };
	char packName[] = "Pack.pack";
	fixedHeader.type = 0xffff;
	fixedHeader.filecnt = 4;
#endif // DEBUG

	// 헤더 선언
	fileInfoHeader = (Pack_Fileinfo*)malloc(sizeof(Pack_Fileinfo) * fixedHeader.filecnt);
	const int offset = sizeof(Pack_Header) + (sizeof(Pack_Fileinfo) * fixedHeader.filecnt);

	// 패킹될곳 파일 열기
	fopen_s(&packFile, packName, "wb");
	if (packFile == NULL) {
		printf_s("packFile == NULL");
		return false;
	}
	// 파일부터 복사하기 위해 헤더공간은 넘김
	fseek(packFile, offset, SEEK_SET);

	// 파일을 하나씩 열기
	for (int i = 0; i < fixedHeader.filecnt; i++) {
		// 파일 읽기
		fopen_s(&objectFile, fileName[i], "rb");
		if (objectFile == NULL) {
			printf_s("%s 파일이 없음", fileName[i]);
			return false;
		}

		// 헤더 정보 얻기
		// 사이즈 구하기
		fseek(objectFile, 0, SEEK_END);
		int objectFileSize = ftell(objectFile);
		fseek(objectFile, 0, SEEK_SET);

		// 파일정보 복사
		Strcpy((fileInfoHeader + i)->fileName, fileName[i]);
		(fileInfoHeader + i)->size = objectFileSize;
		(fileInfoHeader + i)->volume = objectFileSize;
		(fileInfoHeader + i)->offset = ftell(packFile);

		// 파일내용 복사
		FileCopyAndPaste(packFile, objectFile, objectFileSize);
		fclose(objectFile);
	}
	// 헤더 내용 쓰기
	fseek(packFile, 0, SEEK_SET);
	fwrite(&fixedHeader, sizeof(Pack_Header), 1, packFile);
	fwrite(fileInfoHeader, sizeof(Pack_Fileinfo), fixedHeader.filecnt, packFile);

	// 다 끝났으니 정리
	fclose(packFile);
	free(fileInfoHeader);

	return true;
}



bool Unpacking() {
	/*
		1. 패킹파일 오픈

		2. 패킹파일 Pack_Header 해더 읽기.  Type 확인 패킹파일 확인.

		4. 파일개수 확인

		5. 파일 개수만큼 Pack_Fileinfo 헤더 읽기

		6. 파일 개수만큼 돌면서 Pack_Fileinfo[N] 의 정보로 파일생성

		7. 사이즈만큼 읽어서 저장

		8. 끝..
		*/

	Pack_Header fixedHeader;		// 고정헤더 저장
	Pack_Fileinfo* fileInfoHeader;	// 파일정보 저장
	FILE* objectFile;				// 복원할곳
	FILE* packFile;					// 패킹된곳
	char packName[FILENAMELEN] = "Pack.pack";

	printf_s("패킹을 풀 파일 >> ");
	//scanf_s("%s", packName);
	printf_s("%s\n", packName);

	// 헤더 정보읽기
	fopen_s(&packFile, packName, "rb");
	if (packFile == NULL) {
		printf_s("packFile == NULL");
		return false;
	}

	// 헤더 읽기
	if (ReadHeader(fixedHeader, &fileInfoHeader, packFile) == false){
		printf_s("헤더읽기 실패!\n");
		return false;
	}

	printf_s("=====================================\n");
	// 파일 언패킹 시작
	for (int i = 0; i < fixedHeader.filecnt; i++) {
		// 복원할곳 열기
		fopen_s(&objectFile, (fileInfoHeader + i)->fileName, "wb");
		if (objectFile == NULL) {
			printf_s("파일 불러오기를 실패했습니다.");
			return false;
		}
		// 파일정보 헤더의 오프셋으로 부터 사이즈만큼 읽어서 복원할곳에 복사
		fseek(packFile, (fileInfoHeader + i)->offset, SEEK_SET);
		FileCopyAndPaste(objectFile, packFile, (fileInfoHeader + i)->size);
		fclose(objectFile);
	}
	// 정리
	free(fileInfoHeader);
	fclose(packFile);
	return true;
}
bool UpdatePackage() {
	// 헤더 정보읽기
	FILE* packFile;					// 패킹된 파일
	FILE* objectFile;				// 교체하길 원하는 파일
	Pack_Header fixedHeader;		// 고정된 헤더
	Pack_Fileinfo* fileInfoHeader;	// 파일정보 헤더
	char packName[FILENAMELEN] = "Pack.pack";

	printf_s("업데이트할 패킹 파일 >> ");
	//scanf_s("%s", packName);
	printf_s("%s\n", packName);

	// 패킹된 파일을 읽고 쓰기 가능한 바이너리로 열기
	fopen_s(&packFile, packName, "r+b");
	if (packFile == NULL) {
		printf_s("packFile == NULL");
		return false;
	}

	// 헤더 읽기
	if (ReadHeader(fixedHeader, &fileInfoHeader, packFile) == false)
	{
		printf_s("헤더읽기 실패!\n");
		return false;
	}

	/*
	* "갱신"민 가능 삽입 삭제가 아님
	* 1. 갱신파일이 현 파일 보다 같거나 작을때
	* // 작거나 같을
	// 그냥 덮어씀
	// 공간이 남아도 그냥 둠
	* 2. 갱신파일이 현 파일 보다 클때
	* // 클때
	// 파일 맨끝에다가 추가
	// offset이랑 volume바꾸기
	*/

	int updateIndex; // 이 파일을 갱신 하겠다
	printf_s("\n몇번째 파일을 갱신 >>");
	scanf_s("%d", &updateIndex);

	printf_s("선택된 파일 :: FILENAME(%s), FILESIZE(%lld), FILEVOLUME(%lld), FILEOFFSET(%lld)\n",
		(fileInfoHeader + updateIndex)->fileName, (fileInfoHeader + updateIndex)->size, (fileInfoHeader + updateIndex)->volume, (fileInfoHeader + updateIndex)->offset);


	char objectFileName[FILENAMELEN] = "z.txt";
	printf_s("\n바꿀 파일 이름 >>");
#ifdef INPUT
	scanf_s("%s", objectFileName);
#else
	printf_s("%s\n", objectFileName);
#endif 

	// 교체할 파일 열기
	fopen_s(&objectFile, objectFileName, "rb");
	if (objectFileName == NULL) {
		printf_s("objectFileName == NULL");
		return false;
	}

	// 파일 사이즈 구하기
	int objectSize;// 교체할 파일 사이즈
	fseek(objectFile, 0, SEEK_END);
	objectSize = ftell(objectFile);
	fseek(objectFile, 0, SEEK_SET);

	// 사이즈가 할당된곳보다 크냐 / 작거나 같냐
	if (objectSize > (fileInfoHeader + updateIndex)->volume) {
		// 클때 
		// 파일 맨끝에다가 추가
		// offset이랑 volume바꾸기
		fseek(packFile, 0, SEEK_END);
		(fileInfoHeader + updateIndex)->volume = objectSize;
		(fileInfoHeader + updateIndex)->offset = ftell(packFile);
	}
	else {
		// 작거나 같을때
		// 헤더에 있는 오프셋으로 파일 바꾸기
		fseek(packFile, (fileInfoHeader + updateIndex)->offset, SEEK_SET);
	}
	// 오프셋으로 찾아가서 파일 복사
	FileCopyAndPaste(packFile, objectFile, objectSize);

	// 헤더정보 바꾸기
	Strcpy((fileInfoHeader + updateIndex)->fileName, objectFileName);
	(fileInfoHeader + updateIndex)->size = objectSize;
	int curHeader = sizeof(Pack_Header) + (sizeof(Pack_Fileinfo) * updateIndex);

	// 헤더 갱신
	fseek(packFile, curHeader, SEEK_SET);
	fwrite((fileInfoHeader + updateIndex), sizeof(Pack_Fileinfo), 1, packFile);

	// 헤더 갱신 확인
	free(fileInfoHeader);
	fseek(packFile, 0, SEEK_SET);
	if (ReadHeader(fixedHeader, &fileInfoHeader, packFile) == false)
	{
		printf_s("헤더읽기 실패!\n");
		return false;
	}

	// 정리
	free(fileInfoHeader);
	fclose(objectFile);
	fclose(packFile);

	return true;
}


int main() {
	/*
	* TODO
	* 1. 드래그앤드롭 패킹(WINAPI)
	*/
	bool res;
	int pup;
	while (true) {
		printf_s("패킹 : 1 | 언패킹 : 2 >> ");

		scanf_s("%d", &pup);
		switch (pup) {
		case 1:
			res = Packing();
			if (res) {
				printf_s("패킹을 성공했습니다!\n");
			}
			else {
				printf_s("패킹을 실패했습니다...\n");
			}
			break;
		case 2:
			res = Unpacking();
			if (res) {
				printf_s("패킹을 성공적으로 풀었습니다!\n");
			}
			else {
				printf_s("패킹해제를 실패했습니다...\n");
			}
			break;
		case 3:
			res = UpdatePackage();
			if (res) {
				printf_s("패킹을 성공적으로 업데이트를 했습니다!\n");
			}
			else {
				printf_s("업데이트를 실패했습니다...\n");
			}
			break;
			
		default:
			break;
		}
	}
	return 0;
}