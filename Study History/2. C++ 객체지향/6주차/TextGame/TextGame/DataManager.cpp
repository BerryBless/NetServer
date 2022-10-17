#include "pch.h"
#include "DataManager.h"

#pragma region DataManager

char	_initFilePath[] = INIT_FILE_PATH;	// 초기 설정이 들어있는 파일 경로 
int		_stageDataCount;					// 몇 스테이지 까지 있나
char** _stageDataPath;						// 스테이지 파일 경로의 리스트
BYTE* _stageData = nullptr;			// 스테이지 파일의 내용물
int		_stageNow = -1;						// 스테이지 파일의 내용물 이 몇스테이지인가

//===================================
//				파일 읽기
// ----------------------------------
// buffer를 파일크기 만큼 malloc 후
// buffer에 파일의 모든 내용을 읽습니다.
// ----------------------------------
// 사용법)
// BYTE* buffer;
// Data_ReadFile(filePath, &buffer);
// 버퍼를 이용하여 뭔가 함
// free(buffer);	 // 안하면 메모리가 질질 새요!
// ----------------------------------
// TODO : free(buffer) 안해도 메모리 안새게 만들기
//===================================
bool Data_ReadFile(char* filePath, BYTE** buffer) {
	IF_PTR_NULL(filePath) return false; // 포인터 널체크

	FILE* fp;	// 파일 포인터
	int fileSize;// 버퍼의 크기를 할당하기 위한 파일의 사이즈
	fopen_s(&fp, filePath, "rb");
	if (fp == NULL) {
		// 파일을 읽지 못했으면 크래쉬
		CRASH();
	}

	// 사이즈 측정
	fseek(fp, 0, SEEK_END);
	fileSize = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	// 버퍼 동적 할당 (파일 사이즈 + 마지막 널문자)
	*buffer = (BYTE*)malloc(sizeof(BYTE) * (fileSize + 1));

	// 데이터 읽기
	if (fread_s(*buffer, sizeof(BYTE) * fileSize, sizeof(BYTE), fileSize, fp) < fileSize) {
		// 데이터를 읽지 못했으면 크래쉬
		CRASH();
		return false;
	}

	fclose(fp);// 파일을 닫습니다
	*(*buffer + fileSize) = '\0'; // 버퍼는 여기서 끝납니다!
	return true;
}

//===================================
//				초기화
// ----------------------------------
// 설정 파일을 읽어 데이터를 받을 준비를 합니다
// 파일 (Init.dat) 를 읽어 전역변수 데이터를 채웁니다.
// 초기파일 정보를 모두 읽으면 더이상 읽을필요가 없으므로 버퍼 할당 해지합니다
//===================================
void Data_Init() {
	// 초기파일(Init.dat) 열기
	BYTE* buffer;// 초기 파일이 들어갈 버퍼
	Data_ReadFile(_initFilePath, &buffer);
	// 몇스테이지 까지 있는가? 얻기
	int index = 0;
	if (Parsing((char*)buffer, &_stageDataCount, index) == false) {
		// 파싱에 실패!
		// TODO 파싱 다시시도
		CRASH();
	}
	// 문자열 배열 동적할당
	_stageDataPath = (char**)malloc(sizeof(char*) * _stageDataCount);

	// 각원소를 동적할당하면서 값 넣어주기
	for (int i = 0; i < _stageDataCount; i++) {
		_stageDataPath[i] = (char*)malloc(sizeof(char) * STRING_LEN_MAX);
		if (Parsing((char*)buffer, _stageDataPath[i], index) == false) {
			// 파싱에 실패!
			// TODO 파싱 다시시도
			CRASH();
		}
	}

	free(buffer); // 초기파일 정보를 모두 얻었으니 할당 헤지
}

//===================================
//			스테이지 불러오기
// ----------------------------------
// num 번째 스테이지 파일을 읽습니다.
// 파일의 버퍼를 _stageData에 저장하여 
// 스테이지를 초기화 할때마다 참조합니다.
// 스테이지가 바뀔때마다 _stageData를 free해줘야 합니다!
// 따라서 _stageData가 널포인터가 아니라면 free합니다
//===================================
void Data_LoadStage(int num) {
	// _stageData에 할당된 데이터가 있으면 free!
	if (_stageData != nullptr) {
		free(_stageData);
		_stageData = nullptr;
	}
	Data_ReadFile(_stageDataPath[num], &_stageData);
}
#pragma endregion

#pragma region Parsing
// ==================================
// 파싱함수들
// 파일로 읽은 버퍼를 문자열로 가정하여 파싱합니다.
// index는 버퍼의 임의의 한 지점으로 부터 시작합니다
// 파싱 성공시 TRUE, 실패시 FALSE를 반환합니다.
// index를 공유하면서 연속적으로 파싱하는걸 전재로 합니다.
// ==================================

// 알파벳 또는 숫자를 만날때까지 index를 증가시킵니다.

void FindDataStartIndex(const char* str, int& index) {
	char buffer;// 현재 index가 가르키는 값을 저장할 임시변수
	while (true) {
		buffer = *(str + index);
		if (('a' <= buffer && buffer <= 'z') ||
			('A' <= buffer && buffer <= 'Z') ||
			('0' <= buffer && buffer <= '9') ||
			buffer == '\0') {
			break;
		}
		index++;
	}
}

// ==================================
//			 데이터 파싱 함수
// ----------------------------------
// 버퍼에 있는 문자열을 index지점으로 부터 NULL, 공백, 엔터등 각 상황이 끝나는 곳 까지 읽어 value에 담습니다.
// ----------------------------------
// ** 반드시 val는 미리 할당되어 있어야합니다. **
// ==================================

// 정수형 데이터를 파싱합니다.
bool Parsing(const char* str, int* val, int& index) {
	if (val == nullptr) {
		CRASH();
	}
	// 데이터를 시작지점
	// 숫자나 알파벳 부터 시작하기
	FindDataStartIndex(str, index);

	bool hit = false;	// 파싱 성공여부
	int temp = 0;		// 파싱된 정수
	char buffer;// 현재 index가 가르키는 값을 저장할 임시변수
	while (true) {

		buffer = *(str + index);
		// 숫자 발견!
		if ('0' <= buffer && buffer <= '9') {
			temp *= 10;
			temp += (buffer - '0');
			hit = true;
		}
		// 데이터의 시작이 숫자가 아니거나 숫자가 끝나면 리턴
		else {
			break;
		}
		index++;
	}
	*val = temp;
	return hit;
}
// 실수형데이터을 파싱합니다
bool Parsing(const char* str, float* val, int& index) {
	if (val == nullptr) {
		CRASH();
	}
	bool hit = false;// 파싱 성공여부
	int iPart = 0;	// 정수부
	float dPart = 0;// 소수부
	int tI;			// 인덱스를 임시로 저장할곳 (지수 계산할 현재인덱스 - 시작인덱스)
	int num = 0;	// 숫자부분
	float expo = 1;	// 지수수분 (1/10^N)

	// 정수부 구하기
	hit = Parsing(str, &iPart, index);
	// 소수부가 있으면
	if (*(str + index) == '.' && hit == true) {
		tI = index + 1;
		num = 0;	// 숫자부분
		expo = 1;	// 지수수분 (1/10^N)

		Parsing(str, &num, index);
		for (int i = tI; i < index; i++) {
			expo *= (float)0.1;
		}
		dPart = (float)num * expo;
	}

	*val = (float)iPart + dPart;

	return hit;
}
// 문자열 데이터을 파싱합니다
bool Parsing(const char* str, char* val, int& index) {
	if (val == nullptr) {
		CRASH();
	}

	char buffer;		// 현재 index가 가르키는 값을 저장할 임시변수
	bool hit = false;	// 파싱 성공 여부
	int len = 0;		// 문자열의 길이
	// 데이터를 시작지점
	// 숫자나 알파벳 부터 시작하기
	FindDataStartIndex(str, index);
	while (true) {
		// 읽을 수 없을때까지 len을 늘려가며 하나씩 복사
		buffer = *(str + index);
		if (buffer == 0x0a || buffer == 0x0d || buffer == 0x00) {
			// 줄바꿈(LF) || 복귀(CR) || 널문자 일때
			// 마지막에 널문자 넣어주기
			*(val + len) = '\0';
			break;
		}
		*(val + len) = buffer;
		index++;
		len++;
		hit = true;
	}
	return hit;
}
#pragma endregion