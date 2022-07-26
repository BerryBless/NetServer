#pragma once
#include <stdio.h>
#include <string>
#include <list>
typedef wchar_t WCHAR;
#pragma region StringFunc

// WCHAR스트링의 길이를 구합니다
int Strlen(const WCHAR* str);
// WCHAR스트링을 복사합니다.
void Strcpy(WCHAR* strDest, const WCHAR* strSrc);
// WCHAR스트링을 len만큼 복사합니다.
void Strcpy(WCHAR* strDest, const WCHAR* strSrc, const int len);
// WCHAR스트링의 비교를 합니다
int Strcmp(const WCHAR* str1, const WCHAR* str2);
// WCHAR스트링의 비교를 len만큼 합니다
int Strcmp(const WCHAR* str1, const WCHAR* str2, const int len);

#pragma endregion



// =========================================================================
//									 파서
// -------------------------------------------------------------------------
// [Controls]					<- naespace
// bAlwaysRunByDefault=1		<- key=value
// bGamePadRumble=1
// bInvertYValues=0
// key=value 값을 파싱하는것
// 네임스페이스의 키를 찾음 변수를 뱉어줌
// .ini 파일 규칙
// [네임스페이스 이름] 이 네임스페이스가 끝나는 범위는 다음 '['를 만나거나 파일의 끝
// 네임스페이스와 변수의 이름은 알파벳, '_', '.', ','  만 포함가능
// value값이 문자열일때 ""안의 값은 멀티바이트 캐릭터셋
// =========================================================================
class CParser
{
private:
	// 네임스페이스 리스트 구조체
	struct stNamespaceInfo
	{
		WCHAR Name[32];		// 네임스페이스 이름
		int Start;			// 네임스페이스 시작 인덱스
		int End;			// 네임스페이스 끝 인덱스
	};

public:
	CParser(const WCHAR* FILEPATH);	// 파일 경로를 받아서 _buffer에 저장, _namespaceList생성
	~CParser();						// _buffer, _namespaceList 소멸

private:
	WCHAR* _buffer;	// 파일 내용물 (UTF-8을 유니코드로 변환해서 저장)
	int _filesize;	// 파일크기
	std::list<stNamespaceInfo> _namespaceList; // 네임스페이스 인덱스 리스트
	int _start;		// 인덱스 시작
	int _end;			// 인덱스 끝

private:
	bool Skip(int& index);				// key 또는 value 시작지점 찾기
	bool Skip(int& index, WCHAR ch);	// ch가 나올때까지 스킵
	bool TryGetKey(const WCHAR* key, int& index);			// 키값찾기, 키를 찾으면 value 시작 까지 index를 옮김


public: // getvalue

	bool SetNamespace(const WCHAR* nspace);

	// ===========================================================================
	// 파싱 함수들
	// --------------------------------------------------------------------------
	// [] 안 문자, key 값을 받아 value에 대입이 성공했으면 true
	// ===========================================================================
	bool TryGetValue(const WCHAR* key, bool& value);			// bool 파싱
	bool TryGetValue(const WCHAR* key, int& value);				// int 파싱
	bool TryGetValue(const WCHAR* key, float& value);			// float 파싱
	bool TryGetValue(const WCHAR* key, WCHAR* value );			// string (wchar)파싱


};

