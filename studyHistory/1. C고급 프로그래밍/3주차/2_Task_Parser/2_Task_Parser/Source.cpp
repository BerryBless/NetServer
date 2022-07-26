#include <stdio.h>
#include <stdlib.h>

char _dic[20][2][20] = { {"i", "나"},{"am", "는"},{"a", "하나의"},{"boy", "소년"} }; // [20개][0=영어 1=한국][문자열 길이]

char* Parser(const char* str); // 번역 파서

char* Strlwr(char* str); // 문자열 소문자 반환
char* Strcpy(char* strDest, const char* strSrc); // 문자열 복사
char* Strcat(char* strDest, const char* strSrc); // 문자열 붙여넣기
int Strcmp(const char* str1, const char* str2); // 문자열 비교
int Strlen(const char* str); // 문자열 길이 반환


int main() {
	char input[100] = "I amasda Boy";
	gets_s(input);
	char* parsedString = Parser(input);
	printf_s("%s\n ", parsedString);
	free(parsedString);
	return 0;
}

char* Parser(const char* str) {
	// 소문자로 변환 (사전은 모두 소문자로 되어있음)
	int len = Strlen(str);
	char* buffer = (char*)malloc(sizeof(char) * len + 1);
	Strcpy(buffer, str);
	Strlwr(buffer);
	// 버퍼의 스페이스를 NULL로 string 끊기
	char* sBuffer[10]; // 단어 시작지점 저장
	int cnt = 0;
	sBuffer[cnt] = buffer;
	for (int i = 0; i < len; i++) {
		if (*(buffer + i) == ' ') {
			*(buffer + i) = '\0';
			sBuffer[++cnt]= (buffer + i + 1);
		}
	}
	// 끊은 문자열 파싱
	char* parsedString = (char*)malloc(sizeof(char) * 100);
	*parsedString = '\0';// 빈 문자열 입니다.
	for (int i = 0; i <= cnt; i++) {
		bool find = false;
		// 사전에서 파싱가능한지 찾기
		for (int j = 0;  j < 20; j++) {
			if (Strcmp(sBuffer[i], _dic[j][0]) == 0) {
				Strcat(parsedString, _dic[j][1]);
				find = true;
				break;
			}
		}
		// 파싱 불가능하면 원문 출력
		if (find == false) {
			Strcat(parsedString, sBuffer[i]);
		}
		// 사이에 공백
		Strcat(parsedString, " ");
	}
	// 리턴
	free(buffer);
	return parsedString;
} // 번역 파서


// 문자열 소문자 반환
char* Strlwr(char* str) {
	const int Atoa = 'a' - 'A';
	int len = Strlen(str);
	for (int i = 0; i < len; i++) {
		if ('A' <= *(str + i) && *(str + i) <= 'Z') {
			*(str + i) += Atoa;
		}
	}
	return str;
}
// 문자열 복사 (처음부터 NULL나올때 까지)
char* Strcpy(char* strDest, const char* strSrc) {
	int len = 0;
	while (*(strSrc + len) != '\0') {
		*(strDest + len) = *(strSrc + len);
		len++;
	}
	*(strDest + len) = '\0';
	return strDest;
}
// 문자열 붙이기
char* Strcat(char* strDest, const char* strSrc) {
	int lenD = Strlen(strDest);
	int lenS = Strlen(strSrc);

	for (int i = lenD; i <= lenD + lenS; i++) {
		*(strDest + i) = *(strSrc + i - lenD);
	}

	return strDest;
}
// 문자열 비교
int Strcmp(const char* str1, const char* str2) {
	int len = Strlen(str1);
	for (int i = 0; i < len; i++) {
		if (*(str1 + i) != *(str2 + i)) {
			return *(str1 + i) - *(str2 + i);
		}
	}
	return *(str1 + len ) - *(str2 + len );
} 
// 문자열 길이 0부터 셈
int Strlen(const char* str) {
	int len = 0;
	while (*(str + len) != '\0')
	{
		len++;
	}
	return len;
}