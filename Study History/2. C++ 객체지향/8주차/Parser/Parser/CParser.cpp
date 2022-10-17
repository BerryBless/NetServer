#include "CParser.h"
#include <Windows.h>		// MultiByteToWideChar()
#include <stringapiset.h>	// MultiByteToWideChar()
#include <locale.h>
#define CRASH() int *p = nullptr; *p++

#pragma region StringFunc

int Strlen(const WCHAR* str) {
	int len = 0;
	while (*(str + len) != '\0') {
		len++;
	}
	return len;
}
void Strcpy(WCHAR* strDest, const WCHAR* strSrc) {
	int len = 0;
	while (*(strSrc + len) != '\0') {
		*(strDest + len) = *(strSrc + len);
		len++;
	}
	*(strDest + len) = '\0';
}
void Strcpy(WCHAR* strDest, const WCHAR* strSrc, const int len) {
	for (int i = 0; i < len; i++) {
		*(strDest + i) = *(strSrc + i);
	}
	*(strDest + len) = '\0';
}
int Strcmp(const WCHAR* str1, const WCHAR* str2) {
	int len = Strlen(str1);
	for (int i = 0; i < len; i++) {
		if (*(str1 + i) != *(str2 + i))
			return *(str1 + i) - *(str2 + i);
	}
	return *(str1 + len) - *(str2 + len);
}
int Strcmp(const WCHAR* str1, const WCHAR* str2, const int len) {
	for (int i = 0; i < len; i++) {
		if (*(str1 + i) != *(str2 + i))
			return *(str1 + i) - *(str2 + i);
	}
	return *(str1 + len) - *(str2 + len);
}

#pragma endregion


#pragma region CParser

#pragma region CNamespaceList

CParser::CNamespaceList::CNamespaceList() {
	_head = nullptr;
}

CParser::CNamespaceList::~CNamespaceList() {
	Clear(this->_head);
}

// 리스트 뒤에 노드 붙이기
void CParser::CNamespaceList::Push_back(const WCHAR* name, int start) {
	/* 꼬리에 노드 추가하기 */

	// 노드를 하나 생성
	stNamespaceInfoNode* node = new stNamespaceInfoNode;
	Strcpy(node->Info.Name, name);
	node->Info.Start = start;
	node->Info.End = MAXINT32;
	node->tail = nullptr;

	// 머리가 비었으면 머리로
	if (_head == nullptr) {
		_head = node;
	}
	// 아니면 꼬리에 가서 붙기
	else {
		stNamespaceInfoNode* itr = _head;
		while (itr->tail != nullptr) {
			itr = itr->tail;
		}

		itr->Info.End = node->Info.Start;	// 전노드의 끝은 새노드의 시작지점
		itr->tail = node;
	}

}

// 리스트순회 해서 값찾기
CParser::stNamespaceInfo* CParser::CNamespaceList::Find(const WCHAR* name) {
	// 찾을 네임스페이스를 받아서
	// 리스트를 순회
	if (name == NULL) {
		return nullptr;
	}

	stNamespaceInfoNode* itr = _head;
	while (itr != nullptr) {
		if (Strcmp(itr->Info.Name, name) == 0) {
			// 순회도중 찾으면  반환
			return &itr->Info;
		}
		itr = itr->tail;
	}
	// 찾지 못함
	return nullptr;
}

// 리스트 할당 해지
void CParser::CNamespaceList::Clear(stNamespaceInfoNode* node) {
	// 재귀로 끝까지가서
	// 되돌아가면서 할당 해제
	if (node == nullptr) {
		return;
	}
	Clear(node->tail);
	delete node;
}

#pragma endregion


CParser::CParser(const WCHAR* FILEPATH) {
	// ================================================================
	// 	   파일입력
	// ----------------------------------------------------------------
	// 1. '[' 문자를 만나면 ']' 문자를 만날때까지 저장 (네임스페이스 이름)
	// 2. start는 ']' 문자 위치
	// 3. _namespaceList에 push_back(nameBuffer, index) 호출
	// ================================================================
	FILE* fp = NULL;	// 파일포인터
	char* buffer;		// UTF-8이 저장될 버퍼

	_wfopen_s(&fp, FILEPATH, L"r");	// 멀티바이트로 읽기
	if (fp == NULL) {  
		// 읽지 못함
		CRASH();
	}
	// 파일크기 구하기
	fseek(fp, 0, SEEK_END);
	_filesize = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	// buffer를 동적할당
	buffer = new char[_filesize];

	// buffer에 파일내용 한번에 넣기
	if (fread_s(buffer, _filesize, _filesize, 1, fp) < 1) {
		CRASH();
	}
	fclose(fp);


	// UTF-8 -> UNICODE 변환
	// https://docs.microsoft.com/en-us/windows/win32/api/stringapiset/nf-stringapiset-multibytetowidechar
	_buffer = new WCHAR[_filesize];
	if (MultiByteToWideChar(CP_UTF8, 0, buffer, sizeof(BYTE) * _filesize, _buffer, sizeof(WCHAR) * _filesize)== NULL) {
		// utf-8을 멀티바이트로 변경못함
		CRASH();
	}
	_wsetlocale(LC_ALL, L"korean"); // 한국어로 지역설정
	delete[] buffer;

	
	// ================================================================
	// 	   네임스페이스 지정
	// ----------------------------------------------------------------
	// 1. '[' 문자를 만나면 ']' 문자를 만날때까지 저장 (네임스페이스 이름)
	// 2. start는 ']' 문자 위치
	// 3. _namespaceList에 push_back(nameBuffer, index) 호출
	// 딱 여기서만 호출되야 하기때문에 따로 메소드를 파지않음.
	// ================================================================
	WCHAR nameBuffer[32];
	int start = -1;
	int nameLen;
	for (int index = 0; index < _filesize; index++) {
		if (*(_buffer + index) == L'[') {
			// '[' 다음문자부터
			start = index + 1;
		}
		if (*(_buffer + index) == L']' && start >= 0) {
			// ']' 문자까지 && '[' 문자를 한번이라도 만났으면
			// 1. 길이구하고
			// 2. 이름 저장하고
			// 3. _namespaceList.Push_back(nameBuffer, index) 호출
			nameLen = index - start;
			Strcpy(nameBuffer, (_buffer + start), nameLen);
			_namespaceList.Push_back(nameBuffer, index);

			// 짝이되는 대괄호를 만났으니.. 초기화
			start = -1;
		}
	}

	// index 초기값
	_iStart = 0;
	_iEnd = _filesize;
}
CParser::~CParser() {
	delete _buffer;
}

// private
// ===================================
//		index Skip
// -----------------------------------
// *(_buffer + index)가 파싱 가능한 문자가 나올때까지 스킵
// ===================================
bool CParser::Skip(int& index) {
	WCHAR buffer;
	while (true) {
		if (index >= _iEnd) {
			// 탐색범위를 벗어남
			return false;
		}
		buffer = *(_buffer + index);
		// key 또는 value 시작 찾기
		if ((L'a' <= buffer && buffer <= L'z') || // 알파벳 소문자
			(L'A' <= buffer && buffer <= L'Z') || // 알파벳 대문자
			(L'0' <= buffer && buffer <= L'9') || // 아라비아 숫자
			// 파서에 사용할 특수문자
			// '_' = 변수 첫글자 || '\"' = 문자열 시작과 끝
			buffer == L'_' || buffer == L'\"') {
			return true;
		}
		index++;
	}
	return false;
}
// ===================================
//		index Skip
// -----------------------------------
// *(_buffer + index)가 ch 문자가 나올때까지 index++
// ===================================
bool CParser::Skip(int& index, WCHAR ch) {
	WCHAR buffer;
	while (true) {
		if (index >= _iEnd) {
			// 탐색범위를 벗어남
			return false;
		}

		buffer = *(_buffer + index);

		if (buffer == ch) {// 원하는 문자면
			return true;
		}
		index++;
	}
	return false;
}


// ===================================
//		key find
// -----------------------------------
// index부터 key값이 나올때까지 순회
// ===================================
bool CParser::TryGetKey(const WCHAR* key, int& index) {
	int keylen = Strlen(key) - 1;
	while (index < _iEnd) {
		this->Skip(index, *key);
		if (Strcmp(key, _buffer + index, keylen) == 0) {
			this->Skip(index, L'=');	// = 다음으로 넘기기
			this->Skip(index);			// = 다음에 공백있을시 건너뛰기
			return true;
		}
		index++;
	}
	return false;
}

// public

// ===================================
//		find namespace index
// -----------------------------------
// 네임스페이스 nspace 를 _namespaceList에서 찾아 노드값을 반환
// ===================================
bool CParser::SetNamespace(const WCHAR* nspace) {
	// 네임스페이스 찾기
	const stNamespaceInfo* node = this->_namespaceList.Find(nspace);
	if (node == nullptr) {
		// 문제있음 : 존재하지않는 네임스페이스
		return false;
	}
	_iStart = node->Start;
	_iEnd = min(node->End, _filesize);
	return true;
}
// ===================================
//		bool 파싱
// -----------------------------------
// 1. index부터 key 값을 찾기
// 2. 값을 못 찾았으면.. false 리턴
// 3. 찾았으면 index가 key = value 에서 value 첫번째를 가르킴
// 4. boolean변수 파싱
//	4-1. 0, 'f', 'F' 중 하나면 false
//  4-2. 1, 't', 'T' 중 하나면 true
// 5. 값을 value에 넣고 성공했다고 true를 리턴하기
// ===================================
bool CParser::TryGetValue(const WCHAR* key, bool& value) {
	int index = this->_iStart;		// 네임스페이스 시작

	// key 찾기
	if (TryGetKey(key, index) == false) {
		// 키값을 찾지 못함
		return false;
	}

	// value 파싱
	bool bValue = 0;		// 파싱할 불리언
	WCHAR buffer;			// 현재 index가 가르키는 값을 저장할 임시변수

	buffer = *(_buffer + index);
	// 변수 발견
	if (buffer == L'0' || buffer == L'f' || buffer == L'F') {
		// 0 || false || FALSE
		bValue = false;
	}
	else if (buffer == L'1' || buffer == L't' || buffer == L'T') {
		// 1 || true || TRUE
		bValue = true;
	}
	else {
		return false;
	}
	value = bValue;

	return true;
}

// ===================================
//		int 파싱
// -----------------------------------
// 1. 네임스페이스 index 계산
// 2. index부터 key 값을 찾기
// 3. 값을 못 찾았으면.. false 리턴
// 4. 찾았으면 index가 key = value 에서 value 첫번째를 가르킴
// 5. int 변수 파싱
//  5-0. 파싱될 값을 저장할 변수 iValue
//	5-1. 숫자범위 '0' ~ '9' 사이의 값을 체크하고
//  5-1-1. 숫자값이면 iValue를 10 곱하고 그 숫자값 더하기
//  5-1-2. 아니면 끝내기
// 6. 값을 value에 넣고 성공했다고 true를 리턴하기
// ===================================
bool CParser::TryGetValue(const WCHAR* key, int& value) {
	int index;		// 네임스페이스 시작

	// key 찾기
	if (TryGetKey(key, index) == false) {
		// 키값을 찾지 못함
		return false;
	}

	// value 파싱
	int iValue = 0;		// 파싱될 정수
	WCHAR buffer;// 현재 index가 가르키는 값을 저장할 임시변수
	while (index < 	_iEnd)	// 네임스페이스 끝
	 {
		// 지금 이문자를 보고있다
		buffer = *(_buffer + index);
		// 숫자 발견!
		if (L'0' <= buffer && buffer <= L'9') {
			iValue *= 10;
			iValue += (buffer - L'0');
		}
		// 데이터의 시작이 숫자가 아니거나 숫자가 끝나면 리턴
		else {
			break;
		}
		index++;
	}
	value = iValue;

	return true;
}
// ===================================
//		float 파싱
// -----------------------------------
// 1. 네임스페이스 index 계산
// 2. index부터 key 값을 찾기
// 3. 값을 못 찾았으면.. false 리턴
// 4. 찾았으면 index가 key = value 에서 value 첫번째를 가르킴
// 5. float 변수파싱
//  5-0. 정수부, 소수부를 따로 구해서 더하기
//  5-1. 정수부 숫자범위 '0' ~ '9' 사이의 값을 체크하고
//		5-1-1. 숫자값이면 iPart를 10 곱하고 그 숫자값 더하기
//		5-1-2. 아니면 끝내기
//	5-2. 소수부 숫자범위 '0' ~ '9' 사이의 값을 체크하고
//		5-2-1. 숫자값이면 dPart를 10 곱하고 그 숫자값 더하기, 몇번 구했는지 체크하기
//		5-2-2. 끝나면 몇번 구했는지를 0.1를 계속 곱하면서 돌기
//  5-3. value 에 iPart, dPart 더하기
// 6. 값을 value에 넣고 성공했다고 true를 리턴하기
// ===================================
bool CParser::TryGetValue(const WCHAR* key, float& value) {
	int index = this->_iStart;		// 네임스페이스 시작

	// key 찾기
	if (TryGetKey(key, index) == false) {
		// 키값을 찾지 못함
		return false;
	}

	// value 파싱
	WCHAR buffer;	// 현재 index가 가르키는 값을 저장할 임시변수
	int iPart = 0;	// 정수부
	float dPart = 0;// 소수부
	int tI;			// 인덱스를 임시로 저장할곳 (지수 계산할떄 현재인덱스 - 시작인덱스)
	int num = 0;	// 숫자부분
	float expo = 1;	// 지수수분 (1/10^N)

	// 정수부 구하기
	while (index < _iEnd) {
		buffer = *(_buffer + index);
		// 숫자 발견!
		if (L'0' <= buffer && buffer <= L'9') {
			iPart *= 10;
			iPart += (buffer - L'0');
		}
		// 데이터의 시작이 숫자가 아니거나 숫자가 끝나면 리턴
		else {
			break;
		}
		index++;
	}
	// 소수부가 있으면
	if (*(_buffer + index) == '.') {
		tI = ++index ;
		num = 0;	// 숫자부분
		expo = 1;	// 지수수분 (1/10^N)

		while (index < _iEnd) {
			buffer = *(_buffer + index);
			// 숫자 발견!
			if (L'0' <= buffer && buffer <= L'9') {
				num *= 10;
				num += (buffer - L'0');
			}
			// 데이터의 시작이 숫자가 아니거나 숫자가 끝나면 리턴
			else {
				break;
			}
			index++;
		}
		for (int i = tI; i < index; i++) {
			expo *= (float)0.1;
		}
		dPart = (float)num * expo;
	}

	value = (float)iPart + dPart;

	return true;
}
// ===================================
//		문자열(WCHAR*) 파싱
// -----------------------------------
// 1. 네임스페이스 index 계산
// 2. index부터 key 값을 찾기
// 3. 값을 못 찾았으면.. false 리턴
// 4. 찾았으면 index가 key = value 에서 value 첫번째를 가르킴
// 5. 문자열 변수 파싱
//	5-1. 문자열은 "치킨먹고싶다" 이렇게 앞뒤로 큰따옴표가 붙음
//	5-2. 큰따옴표 사이의 길이를 구해서
//	5-3. *미리 할당된* value포인터에 strcmp하기 
// 6. 값을 value에 넣고 성공했다고 true를 리턴하기
// ===================================
bool CParser::TryGetValue(const WCHAR* key, WCHAR* value) {
	int index = this->_iStart;		// 네임스페이스 시작

	// key 찾기
	if (TryGetKey(key, index) == false) {
		// 키값을 찾지 못함
		return false;
	}

	// value 파싱
	WCHAR buffer = *(_buffer + index);
	int start;
	int len;

	// " 으로 시작하지 않으면 문자열이 아님
	if (buffer == L'\"') {
		start = ++index;
		while (index < _iEnd) {
			buffer = *(_buffer + index);
			// 다음 " 만날때까지 쭉쭉 진행
			if (buffer == L'\"') {
				break;
			}
			index++;
		}
	}
	else return false;

	len = index - start; // 문자열 길이 = 끝 - 시작
	
	
	Strcpy(value, _buffer + start, len);

	return true;
}

#pragma endregion