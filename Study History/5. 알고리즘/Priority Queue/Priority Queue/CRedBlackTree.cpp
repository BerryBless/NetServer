#include "CRedBlackTree.h"
#include "CMemory.h"
#include <stdio.h>

extern CMemory g_memoryPool;

CRedBlackTree::CRedBlackTree() {


	// Nil 초기화
	_Nil = (stNODE *) g_memoryPool.Alloc(sizeof(stNODE));
	_Nil->Color = BLACK;
	_Nil->pParent = NULL;
	_Nil->pLeft = NULL;
	_Nil->pRight = NULL;
	// 루트 초기화
	_root = _Nil;
}

CRedBlackTree::~CRedBlackTree() {
	// TODO 후위순회하며 메모리 프리
}

bool CRedBlackTree::InsertNode(long long Key, long long Data) {
	// 지역변수
	stNODE *pNode;	// 새로운 노드
	stNODE *pCur;	// 현재 확인중인 노드

	// 새 노드 생성
	pNode = (stNODE *) g_memoryPool.Alloc(sizeof(stNODE));
	pNode->Color = RED;		// 빨간색만 추가 가능 (모든경로에 검정색의 수는 같기때문에)
	pNode->pParent = _Nil;
	pNode->pLeft = _Nil;
	pNode->pRight = _Nil;
	pNode->Key = Key;
	pNode->Data = Data;

	// 추가
	// 루트가 비었으면
	if (_root == _Nil) {
		// 첫번 노드로!
		_root = pNode;
		_root->Color = BLACK;
		return true;
	}

	pCur = _root;
	while (true) {
		// 같은! 거
		// 삽입 실패
		if (pCur->Key == Key) {
			return false;
		}

		// 작은! 거
		// 왼쪽에 삽입
		if (pCur->Key > Key) {
			if (pCur->pLeft != _Nil) {
				pCur = pCur->pLeft;
				continue;
			}

			// 왼쪽에 삽입
			pCur->pLeft = pNode;
			pNode->pParent = pCur;
			break;
		}

		// 큰! 거
		// 오른쪽에 삽입
		{
			if (pCur->pRight != _Nil) {
				pCur = pCur->pRight;
				continue;
			}
			pCur->pRight = pNode;
			pNode->pParent = pCur;
			break;
		}
	}


	return InsertSorting(pNode);
}

bool CRedBlackTree::InsertSorting(stNODE *pNode) {
	// 삼촌노드 (부모 > 할아버지 > 옆 자식) 
	stNODE *pUncle;

	// 삽입완료
	while (pNode != _root) {


		//////////////////
		// 부모의 컬러가 블랙
		// 아무 문제 없음!
		////////////////////
		if (pNode->pParent->Color == BLACK) {
			break;
		}


		////////////////////////////////////////////////////////
		// 부모가 빨강
		// # 모든 문제는 부모가 레드인 경우이므로 할아버지는 블랙이다.
		////////////////////////////////////////////////////////

		// 삼촌이 오른쪽에 있을때!
		if (pNode->pParent->pParent->pLeft == pNode->pParent) {
			//		   G
			//		P	   U
			//	이쯤에N |
			pUncle = pNode->pParent->pParent->pRight; // 삼촌 노드
			if (pUncle->Color == RED) {
				// 부모의 컬러가 레드
				// 1. 부모 레드, 삼촌도 레드.
				//	할아버지가 레드로 바뀜으로 인해서,  할아버지 상단의 색상도 모두 비교를 해보아야 한다.

				//	그래서 결국 ROOT 노드까지 올라가면서 이를 반복 확인 해야한다.

				//	할아버지를 새 노드로 잡고 다시 확인.
				//  할아버지의 색이 양쪽으로 내려옴
				pNode->pParent->Color = BLACK;
				pUncle->Color = BLACK;
				// 할아버지가 빨개짐
				pNode->pParent->pParent->Color = RED;
				// 확인해야할 대상이 할아버지로 변함
				pNode = pNode->pParent->pParent;
				continue;
			}

			if (pUncle->Color == BLACK) {
				if (pNode == pNode->pParent->pRight) {
					// 2. 부모 레드, 삼촌은 블랙, 나는 부모의 오른쪽 레드
					// 3번상황으로 만들어버리기

					// 왼쪽으로 회선하면 (전)부모가 3번상황
					// 호적 바꾸기
					pNode = pNode->pParent;
					Rotate_Left(pNode);
				}
				// 3. 부모 레드, 삼촌은 블랙, 나는 부모의 왼쪽 레드
				//		부모를 검정
				//		할아버지는 레드
				//		할아버지 기준으로 우회전
				pNode->pParent->Color = BLACK;
				pNode->pParent->pParent->Color = RED;
				Rotate_Right(pNode->pParent->pParent);
				break;
			}
		} else {

			//		G
			//	 U	   P
			//		| 이쯤에 N


			// 위 상황의 정반대상황
			// 모든 방향이 반대로
			// 삼촌이 오른쪽에 있을때!
			pUncle = pNode->pParent->pParent->pLeft; // 삼촌 노드

			if (pUncle->Color == RED) {
				// 부모의 컬러가 레드
				// 1. 부모 레드, 삼촌도 레드.
				//	할아버지가 레드로 바뀜으로 인해서,  할아버지 상단의 색상도 모두 비교를 해보아야 한다.

				//	그래서 결국 ROOT 노드까지 올라가면서 이를 반복 확인 해야한다.

				//	할아버지를 새 노드로 잡고 다시 확인.
				//  할아버지의 색이 양쪽으로 내려옴
				pNode->pParent->Color = BLACK;
				pUncle->Color = BLACK;
				// 할아버지가 빨개짐
				pNode->pParent->pParent->Color = RED;
				// 확인해야할 대상이 할아버지로 변함
				pNode = pNode->pParent->pParent;
				continue;
			}


			if (pUncle->Color == BLACK) {
				if (pNode == pNode->pParent->pLeft) {
					// 2. 부모 레드, 삼촌은 블랙, 나는 부모의 왼 레드
					// 3번상황으로 만들어버리기


					// 왼쪽으로 회선하면 (전)부모가 3번상황
					// 호적 바꾸기
					pNode = pNode->pParent;
					Rotate_Right(pNode);
				}
				// 3. 부모 레드, 삼촌은 블랙, 나는 부모의 오른 레드
				//		부모를 검정
				//		할아버지는 레드
				//		할아버지 기준으로 우회전
				pNode->pParent->Color = BLACK;
				pNode->pParent->pParent->Color = RED;
				Rotate_Left(pNode->pParent->pParent);
				break;
			}
		}

	}

	// 가정) 루트는 무조건 검정색
	_root->Color = BLACK;
	return true;
}

bool CRedBlackTree::DeleteNode(long long Key) {
	stNODE *pNode = Find(Key); // 데이터를 가지고 있는 노드를 찾음
	stNODE *pDelNode = pNode;
	stNODE *pPearnt;			// pDelNode의 부모
	stNODE *pChildren;			// pDelNode의 자식 (어느방향이든 하나만 나옴)
	if (pNode == NULL) return false;

	if (pNode->pLeft != _Nil &&
		pNode->pRight != _Nil) {
		// 자식이 두개면 자신보다 작은 아이중 가장큰 아이를 선택
		// 이 아이는 오른쪽 자식은 절대없음
		pDelNode = pNode->pLeft;				// 자신보다 작은것중
		while (pDelNode->pRight != _Nil) {		// 가장 큰것
			pDelNode = pDelNode->pRight;
		}

		pNode->Key = pDelNode->Key;			// 나보다 작은것중 가장큰게 내가 됨
	}



	// pDelNode는 무조건 자식이 하나



	// 부모는 내부보
	pPearnt = pDelNode->pParent;
	// 자식은 방향에 따라
	if (pDelNode->pLeft == _Nil) {
		pChildren = pDelNode->pRight;
	} else {
		pChildren = pDelNode->pLeft;
	}

	if (pDelNode == _root) {
		//내가 루트일때
		_root = pChildren;
	}

	if (pPearnt->pLeft == pDelNode) {
		// 내가 부모의 왼자식
		pPearnt->pLeft = pChildren;
	} else {
		// 내가 부모의 오른자식
		pPearnt->pRight = pChildren;
	}
	// 내 자식의 부모는 내 부모
	pChildren->pParent = pPearnt;

	if (pDelNode->Color == BLACK) {
		// 지울거가 블랙인경우 정렬
		DeleteSorting(pChildren);
	}
	// 삭제된 노드 할당 해지
	g_memoryPool.Free(pDelNode);
	return true;
}

bool CRedBlackTree::DeleteSorting(stNODE *pNode) {
	// pNode 는 삭제후 대체된노드
	stNODE *pSibling;

	while (pNode != _root) {
		//------ 반복 검사 시작부 ------------
			// 삭제노드가 블랙인경우
			// pNode가 있는 쪽이 블랙 하나가 부족하다!!!
			// 삭제노드 = pNode
			// 1 삭제 노드의 부모와 자식이 모두 레드인 경우
			// 2 삭제 노드의 형제가 레드
			// 3 삭제 노드의 형제가 블랙이고 형제의 양쪽 자식이 블랙(부모가 레드인지 아닌지에 따라)
			// 4 삭제 노드의 형제가 블랙이고 형제의 왼자식이 레드, 오른자식은 블랙
			// 5 삭제 노드의 형제가 블랙이고 형제의 오른자식이 레드


			// 형제 구하기
		if (pNode->pParent->pLeft == pNode) {
			//내가 왼쪽 형재는 오른쪽 
			pSibling = pNode->pParent->pRight;
			// 자식노드는 무조건 하나


			if (pNode->Color == RED) {
				// 1 삭제 노드의 부모와 자식(pNode)이 모두 레드인 경우
				// 나를 대체할 노드를 블랙으로바꾸면 끝
				pNode->Color = BLACK;
				break;
			}




			// pNode가 블랙인경우
			if (pSibling->Color == RED) {
				// 2 삭제 노드의 형제가 레드
				// 형재를 블랙으로 바꾸고
				// 부모를 레드로
				// 부모 기준으로 좌회전
				// 그리고 pNode 기준으로 재검사 <<< ????

				pSibling->Color = BLACK;
				pNode->pParent->Color = RED;
				Rotate_Left(pNode->pParent);
				//continue;
			} else {
				// 형재가 검은색
				if (pSibling->pLeft->Color == BLACK &&
					pSibling->pRight->Color == BLACK) {
					// 3 삭제 노드의 형제가 블랙이고 형제의 양쪽 자식이 블랙
					// 이가족은 벨런스를 맞추고
					//		형제를 레드로
					//		이로인해 내 세대는 벨런스가 맞았지만 부모세대는 벨런스가 안맞게됨
					// 가족 벨런스체크를 위로 올리기
					pSibling->Color = RED;
					pNode = pNode->pParent;
					//continue;
				} else {
					if (pSibling->pRight->Color == BLACK) {
						// 4 삭제 노드의 형제가 블랙이고 형제의 왼자식이 레드, 오른자식은 블랙
						// 어캐어캐 해서 5 번상황 만들기
						// 어캐어캐
						// 1.형제의 왼자식을 블랙으로
						// 2. 형제를 레드로
						// 3. 형제 기준으로 우회전

						pSibling->pLeft->Color = BLACK;
						pSibling->Color = RED;
						Rotate_Right(pSibling);

						pSibling = pNode->pParent->pRight;

					}
					// 5 삭제 노드의 형제가 블랙이고 형제의 오른자식이 레드
					// 형재 컬러를 부모 컬러로
					// 부모 컬러는 블랙
					// 형재의 오른자식은 블랙
					// 부모기준 회전

					pSibling->Color = pNode->pParent->Color;
					pNode->pParent->Color = BLACK;
					pSibling->pRight->Color = BLACK;
					

					Rotate_Left(pNode->pParent);
					break;
				}
			}
		}

		else {
			//내가 왼쪽 형재는 왼쪽
			pSibling = pNode->pParent->pLeft;
			// 자식노드는 무조건 하나


			if (pNode->Color == RED) {
				// 1 삭제 노드의 부모와 자식(pNode)이 모두 레드인 경우
				// 나를 대체할 노드를 블랙으로바꾸면 끝
				pNode->Color = BLACK;
				break;
			}




			// pNode가 블랙인경우
			if (pSibling->Color == RED) {
				// 2 삭제 노드의 형제가 레드
				// 형재를 블랙으로 바꾸고
				// 부모를 레드로
				// 부모 기준으로 우회전
				// 그리고 pNode 기준으로 재검사 <<< ????

				pSibling->Color = BLACK;
				pNode->pParent->Color = RED;
				Rotate_Right(pNode->pParent);
				//continue;
			} else {
				// 형재가 검은색
				if (pSibling->pLeft->Color == BLACK &&
					pSibling->pRight->Color == BLACK) {
					// 3 삭제 노드의 형제가 블랙이고 형제의 양쪽 자식이 블랙
					// 이가족은 벨런스를 맞추고
					//		형제를 레드로
					//		이로인해 내 세대는 벨런스가 맞았지만 부모세대는 벨런스가 안맞게됨
					// 가족 벨런스체크를 위로 올리기
					pSibling->Color = RED;
					pNode = pNode->pParent;
					//continue;
				} else {
					if (pSibling->pLeft->Color == BLACK) {
						// 4 삭제 노드의 형제가 블랙이고 형제의 right자식이 레드, left자식은 블랙
						// 어캐어캐 해서 5 번상황 만들기
						// 어캐어캐
						// 1.형제의 right자식을 블랙으로
						// 2. 형제를 레드로
						// 3. 형제 기준으로 rotate_left

						pSibling->pRight->Color = BLACK;
						pSibling->Color = RED;
						Rotate_Left(pSibling);

						pSibling = pNode->pParent->pLeft;

					}
					// 5 삭제 노드의 형제가 블랙이고 형제의 오른자식이 레드
					// 형재 컬러를 부모 컬러로
					// 부모 컬러는 블랙
					// 형재의 left자식은 블랙
					// 부모기준 회전
					pSibling->Color = pNode->pParent->Color;
					pNode->pParent->Color = BLACK;
					pSibling->pLeft->Color = BLACK;

					Rotate_Right(pNode->pParent);
					break;
				}
			}
		}

	}


	return false;
}

CRedBlackTree::stNODE *CRedBlackTree::Find(long long Key) {
	stNODE *pNode = _root;
	while (pNode != _Nil) {
		if (pNode->Key == Key) {
			return pNode;
		}
		if (pNode->Key > Key) {
			pNode = pNode->pLeft;
		} else {
			pNode = pNode->pRight;
		}
	}
	return NULL;
}

void CRedBlackTree::DisplayTree() {
	DrawTree(_root, 1);
}

void CRedBlackTree::DrawTree(stNODE *pNode, int iDepth) {
	if (pNode == _Nil)return;

	DrawTree(pNode->pLeft, iDepth + 1);
	for (int i = 0; i < iDepth; i++) printf_s("\t");
	printf_s("%lld[", pNode->Key);
	if (pNode->Color == BLACK) {
		printf_s("B]\n");
	} else {
		printf_s("R]\n");
	}
	DrawTree(pNode->pRight, iDepth + 1);
}


void CRedBlackTree::Rotate_Left(long long Key) {
	stNODE *pNode = _root;

	while (pNode != _Nil) {
		if (pNode->Key == Key) {
			Rotate_Left(pNode);
			return;
		}

		// 작다
		// 왼쪽
		if (pNode->Key > Key) {
			pNode = pNode->pLeft;
			continue;
		}
		// 크다
		pNode = pNode->pRight;
	}
}

/*
- Node N 을 기준으로 좌회전


	   N

   A       D

 B   C   E   F


		< N 에서 좌회전 >

	   D

	 N    F

   A   E

 B   C

N 의 오른 자식 (D) 이 N 의 위치로 오며 N 은 왼쪽자식(D) 의 왼편으로 붙음.

이때 N 의 오른자식 (D) 의 왼편 자식 (E) 은  N 의 오른편으로 붙음.


1. N의 왼쪽자식 = E
2. E의 부모 = N
3. D의 부모 = N의 부모
4. N의 부모에 따라 다름
	i) Nil일때 (N == root) D가 루트가 됨
	ii) 왼쪽으로 왔을때 D는 N의 부모의 왼쪽자식
	iii) 오른쪽으로 왔을때 D는 N의 부모의 오른쪽자식
5. D의 왼쪽 자식 = N;
6. N의 부모 = D
*/
void CRedBlackTree::Rotate_Left(stNODE *pPivotNode) {
	if (pPivotNode == NULL) return;
	stNODE *pTemp = pPivotNode->pRight;		// 바꿀곳 (D)

	pPivotNode->pRight = pTemp->pLeft;		// 1. N의 오른쪽자식 = D의 왼쪽자식
	if (pTemp->pLeft != _Nil)//닐이 아니면 부모 바꿈.. TODO 이조건 없에기??????
		pTemp->pLeft->pParent = pPivotNode;		// 2. E의 부모 = N
	pTemp->pParent = pPivotNode->pParent;		// 3. D의 부모 = N의 부모
	if (pPivotNode->pParent == _Nil)			// 4. N의 부모의 상태에따라 다름
		_root = pTemp;						// 4.1. N이 루트면 D가 루트로!
	else {
		if (pPivotNode == pPivotNode->pParent->pLeft) //4.2. 부모의 왼쪽에서 옴
			pPivotNode->pParent->pLeft = pTemp;	// N의 부모의 왼쪽자식 = D	

		else														//4.3. 부모의 오른쪽에서 옴
			pPivotNode->pParent->pRight = pTemp;	// N의 부모의 오른쪽자식 =D
	}
	pTemp->pLeft = pPivotNode;				// 5. D의 왼쪽 = N
	pPivotNode->pParent = pTemp;				// 6. N의 부모 = D
}

void CRedBlackTree::Rotate_Right(long long Key) {
	stNODE *pNode = _root;

	while (pNode != _Nil) {
		if (pNode->Key == Key) {
			Rotate_Right(pNode);
			return;
		}

		// 작다
		// 왼쪽
		if (pNode->Key > Key) {
			pNode = pNode->pLeft;
			continue;
		}
		// 크다
		pNode = pNode->pRight;
	}
}


/*

- Node N 을 기준으로 우회전

	   N

   A       D

 B   C   E   F


		< N 에서 우회전 >

	   A

   B       N

		C     D

			E    F


N 의 왼쪽 자식 (A) 이 N 의 위치로 오며 N 은 왼쪽자식(A) 의 오른편으로 붙음.

이때 N 의 왼쪽자식 (A) 의 오른편 자식 (C) 은  N 의 왼편으로 붙음.


1. N의 왼쪽자식 = C
2. C의 부모 = N
3. A의 부모 = N의 부모
4. N이 어디서 왔는지에 따라 다름
	i) N의 부모가 Nil일때 root = a
	ii) 왼쪽에서 왔을때 N의 부모의 왼쪽자식은 A
	iii) 오른쪽에서 왔을때 N의 부모의 오른쪽자식은 A
5. A의 오른쪽 자식 = N
6. N의 부모 = A

*/
void CRedBlackTree::Rotate_Right(stNODE *pPivotNode) {
	if (pPivotNode == NULL) return;


	stNODE *pTemp = pPivotNode->pLeft;			// 바꿀곳 (A)

	pPivotNode->pLeft = pTemp->pRight;			// N의 왼쪽자식 = A의 오른쪽자식
	if (pTemp->pRight != _Nil) // 닐이 아니면 부모 바꿈.. TODO 이조건 없에기??????
		pTemp->pRight->pParent = pPivotNode;			// C의 부모 = N
	pTemp->pParent = pPivotNode->pParent;			// A의 부모 = N의 부모 
	if (pPivotNode->pParent == _Nil)				// N이 루트
		_root = pTemp;							// 루트 바꿔주기
	else {
		if (pPivotNode == pPivotNode->pParent->pLeft)	// 부모의 왼쪽에서 옴
			pPivotNode->pParent->pLeft = pTemp;					// 왼쪽자식은 A

		else														// 부모의 오른쪽에서 옴
			pPivotNode->pParent->pRight = pTemp;					// 오른쪽 자식은 A
	}
	pTemp->pRight = pPivotNode;					// A의 오른쪽자식 = N
	pPivotNode->pParent = pTemp;					// N의 부모 = A
}
