#include "CBSearchTree.h"
#include <stdio.h>
#include <malloc.h>

CBSearchTree::CBSearchTree() {
	_root = nullptr;
}

CBSearchTree::~CBSearchTree() {
}

void CBSearchTree::InsertNode(int iData) {
	InsertNode(_root, iData);
}

bool CBSearchTree::InsertNode(st_NODE *pNow, int iData) {
	st_NODE *pNode;
	if (pNow == nullptr) {
		// 루트가 없음!
		pNode = (st_NODE*) malloc(sizeof(st_NODE));
		pNode->iDATA = iData;
		pNode->pLeft = nullptr;
		pNode->pRight = nullptr;
		_root = pNode;
		return true;
	}

	// 작은거
	if (pNow->iDATA > iData) {
		if (pNow->pLeft == nullptr) {
			// 왼쪽이 빔
			pNode = (st_NODE *) malloc(sizeof(st_NODE));
			pNode->iDATA = iData;
			pNode->pLeft = nullptr;
			pNode->pRight = nullptr;
			// 왼쪽 자식으로!
			pNow->pLeft = pNode;
			return true;
		} else {
			// 왼쪽으로!
			return InsertNode(pNow->pLeft, iData);
		}
	}
	// 큰거
	if (pNow->iDATA < iData) {
		if(pNow->pRight == nullptr){
			// 오른쪽이 빔
			pNode = (st_NODE *) malloc(sizeof(st_NODE));
			pNode->iDATA = iData;
			pNode->pLeft = nullptr;
			pNode->pRight = nullptr;
			// 오른쪽 자식으로!
			pNow->pRight = pNode;
			return true;
		} else {
			// 오른쪽으로!
			return InsertNode(pNow->pRight, iData);
		}
	}

	// 같은거
	return false;
}

bool CBSearchTree::DeleteNode(int iData) {

	return DeleteNode(_root,_root, iData);
}

bool CBSearchTree::DeleteNode(st_NODE *pParent,st_NODE *pNow, int iData) {
	if (pNow == nullptr) return false;
	st_NODE *pNode;
	if (pNow->iDATA == iData) {
		// 찾음!
		if (pNow->pLeft == nullptr && pNow->pRight == nullptr) {
			// 자식이 없음!
			if (pParent->iDATA < pNow->iDATA) {
				// 부보의 오른쪽에서 옴
				pParent->pRight = nullptr;
			} else {
				// 부보의 왼쪽에서 옴
				pParent->pLeft= nullptr;
			}
			free(pNow);
			return true;
		}
		if (pNow->pLeft != nullptr && pNow->pRight != nullptr) {
			// 양쪽다 자식이 있음!
			// 작은것중 가장 큰것
			pParent = pNow;
			pNode = pNow->pLeft;
			while (pNode->pRight != nullptr) {
				pParent = pNode;
				pNode = pNode->pRight;
			}
			// 데이터를 바꾸고
			pNow->iDATA = pNode->iDATA;
			// 왼쪽에서 가장큰곳의 말단노드를 지움
			free(pNode->pRight);
			pParent->pRight = nullptr;
			return true;
		}
		// 자식이 한쪽에만 있음
		if (pNow->pLeft != nullptr) {
			// 왼쪽에만 있음
			pNode = pNow->pLeft;
			if (pParent->iDATA < pNow->iDATA) {
				// 부보의 오른쪽에서 옴
				pParent->pRight = pNode;
			} else {
				// 부보의 왼쪽에서 옴
				pParent->pLeft = pNode;
			}
			free(pNow);
			return true;
		}
		if (pNow->pRight != nullptr) {
			// 오른쪽에만 있음
			pNode = pNow->pRight;
			if (pParent->iDATA < pNow->iDATA) {
				// 부보의 오른쪽에서 옴
				pParent->pRight = pNode;
			} else {
				// 부보의 왼쪽에서 옴
				pParent->pLeft = pNode;
			}
			free(pNow);
			return true;
		}
	}
	// 서칭
	if (pNow->iDATA > iData)
		return DeleteNode(pNow,pNow->pLeft, iData);
	if (pNow->iDATA < iData)
		return DeleteNode(pNow,pNow->pRight, iData);

	return false;
}

void CBSearchTree::PreorderPrint(st_NODE *pNow, int iDepth) {
	if (pNow == nullptr) return;
	for (int i = 0; i < iDepth; i++) printf_s("\t");
	printf_s("%d\n", pNow->iDATA);
	PreorderPrint(pNow->pLeft, iDepth + 1);
	PreorderPrint(pNow->pRight, iDepth + 1);
}

void CBSearchTree::InorderPrint(st_NODE *pNow, int iDepth) {
	if (pNow == nullptr) return;
	InorderPrint(pNow->pLeft, iDepth + 1);
	for (int i = 0; i < iDepth; i++) printf_s("\t");
	printf_s("%d\n", pNow->iDATA);
	InorderPrint(pNow->pRight, iDepth + 1);
}

void CBSearchTree::PostorderPrint(st_NODE *pNow, int iDepth) {
	if (pNow == nullptr) return;
	PostorderPrint(pNow->pLeft, iDepth + 1);
	PostorderPrint(pNow->pRight, iDepth + 1);
	for (int i = 0; i < iDepth; i++) printf_s(" ");
	printf_s("%d\n", pNow->iDATA);
}

void CBSearchTree::Print() {
	InorderPrint(_root, 0);
}
