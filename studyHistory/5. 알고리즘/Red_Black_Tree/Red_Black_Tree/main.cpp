#include "CRedBlackTree.h"
#include <stdio.h>
#include <stdlib.h>
#include "CList.h"

#define dfMAXLEN 500000
#define dfSRAND 35
unsigned int Erand(int byteSize, bool bPositive = false) {
	unsigned int r = 0;		// 0000 0000
	int mask = 0xFF;		// 1111 1111
	for (int i = 0; i < byteSize; i++) {
		int t = rand();
		t &= mask;
		r = (r << 8); // 1byte 만큼 이동
		r |= t;
		//printf_s("%02d : 0x%08x\n", i, r);
	}
	if (bPositive) r &= ~(1 << byteSize);
	return r;
}




class CPos {
public:
	int x;
	int y;
	long long Hash() {
		return ((long long)y << 32) | x;
	}
};

bool comp(CPos a, CPos b) {
	return rand() % 2;
}

int main() {
	CRedBlackTree Tree;
	CList<CPos> answer;
	srand(30);

	CPos posrand;		// 랜덤 정수값 (키값)
	int randtime;	// 삽입/삭제 실행수
					
	// 1. 삽입
	// 2. 정답리스트 랜덤으로 섞기
	// 3. 정렬된 순서대로 트리에서 찾기
	// 4. 랜덤으로 지우기

	/*int n;
	while (true) {
		printf_s("\nINPUT >> ");
		scanf_s("%d", &n);
		printf_s("\n-----------\n");
		Tree.InsertNode(n);
		Tree.Print();
	}*/
	for (int TESTCASE = 1; ; TESTCASE++) {
		printf_s("\n====================\n");
		printf_s("TEST[%d]\n", TESTCASE);
		randtime = Erand(4, true) % dfMAXLEN+1;


		//삽입
		printf_s("Insert..\n");
		for (int i = 0; i < randtime; i++) {
			posrand.x = Erand(4);
			posrand.y = Erand(4);
			answer.push_back(posrand);
			Tree.InsertNode(posrand.Hash());
		}

		// 랜덤으로 섞기
		printf_s("Set Random..\n");
		for (int i = 0; i < randtime / 10; i++) {
			CPos temp = answer.pop_front();
			answer.push_back(temp);
		}

		// 있는지 확인
		printf_s("Check Node\n");
		for (auto iter = answer.begin(); iter != answer.end(); ++iter) {
			CRedBlackTree::stNODE *check = Tree.Find((*iter).Hash());
			if (check->Data != (*iter).Hash()) {
				FILE *fp;
				fopen_s(&fp, "crash.log", "w");
				if (fp == NULL) {
					fp = stdout;
				}

				fprintf_s(fp, "crashed :: [%d %d]\n", dfSRAND, TESTCASE);
				fprintf_s(fp, "error case \nanser");
				while (!answer.empty()) {
					CPos temp = answer.pop_front();
					fprintf_s(fp, "(%d, %d)\t", temp.x, temp.y);
				}

				fclose(fp);
				return 1;
			}
		}

		printf_s("Delete Node...\n");
		while (!answer.empty()) {
			CPos buffer = answer.pop_back();
			Tree.DeleteNode(buffer.Hash());
		}

		if (Tree.empty()) {
			FILE *fp;
			fopen_s(&fp, "crash.log", "w");
			if (fp == NULL) {
				fp = stdout;
			}

			fprintf_s(fp, "crashed :: [%d %d]\n", dfSRAND, TESTCASE);
			fprintf_s(fp, "error case \nanser");
			while (!answer.empty()) {
				CPos temp = answer.pop_front();
				fprintf_s(fp, "(%d, %d)\t", temp.x, temp.y);
			}

			fclose(fp);
				return 1;
		}

		printf_s("Clear");
		printf_s("\n====================\n");
	}



	return 0;
}
