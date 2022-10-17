#ifndef __DEFINE__
#define __DEFINE__


struct st_SESSION
{
	int SessionID;
};


struct st_PLAYER
{
	int SessionID;
	int Content[3];
};

// 찾은오류
// 스레드는 3개
// 증상 : WaitForMultipleObjects가 무한대기
//#define dfTHREAD_NUM	4
#define dfTHREAD_NUM	3
#endif