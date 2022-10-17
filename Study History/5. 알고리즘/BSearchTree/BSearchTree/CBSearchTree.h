#pragma once
class CBSearchTree {
private:
	struct st_NODE {
		int iDATA;
		st_NODE *pLeft;
		st_NODE *pRight;
	};

private:
	st_NODE *_root;

public:
	CBSearchTree();
	~CBSearchTree();
	void InsertNode(int iData);
	bool InsertNode(st_NODE *pNow, int iData);

	bool DeleteNode(int iData);
	bool DeleteNode(st_NODE *pParent,st_NODE *pNow, int iData);

	void PreorderPrint(st_NODE *pNow, int iDepth);
	void InorderPrint(st_NODE *pNow, int iDepth);
	void PostorderPrint(st_NODE *pNow, int iDepth);

	void Print();
};

