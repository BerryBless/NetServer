#pragma once
#include "CMemory.h"
extern CMemory g_memoryPool;
template <typename T>
class CList {
public:
	struct Node {
		T _data;
		Node *_Prev ;
		Node *_Next ;
	};

	class iterator {
	private:
		Node *_node;
		// 이함수에선 _node를 접근 할 수 있음
		friend void CList::insert(iterator iter, T data);
		friend iterator CList::erase(iterator iter);
	public:
		iterator(Node *node = nullptr) {
			//인자로 들어온 Node 포인터를 저장
			this->_node = node;
		}

		// iter++
		iterator operator ++(int) {
			iterator temp = *this;
			//현재 노드를 다음 노드로 이동
			this->_node = this->_node->_Next;
			return temp;
		}

		// ++iter
		iterator &operator++() {
			this->_node = this->_node->_Next;
			return *this;
		}

		// iter--
		iterator operator --(int) {
			iterator temp = *this;
			this->_node = this->_node->_Prev;
			return temp;
		}

		// --iter
		iterator &operator--() {
			this->_node = this->_node->_Prev;
			return *this;
		}

		T &operator *() {
			//현재 노드의 데이터를 뽑음
			return this->_node->_data;
		}
		bool operator ==(const iterator &other) {
			return _node == other._node;
		}
		bool operator !=(const iterator &other) {
			return _node != other._node;
		}
	};

public:
	CList() {
		// 생성자

		// 꼬리랑 연결
		_head._Prev = nullptr;
		_head._Next = &_tail;

		// 머리랑 연결
		_tail._Prev = &_head;
		_tail._Next = nullptr;
	}
	~CList() {
		this->clear();
	}

	iterator begin() {
		//첫번째 데이터 노드를 가리키는 이터레이터 리턴
		return iterator(_head._Next);
	}
	iterator end() {
		//Tail 노드를 가리키는(데이터가 없는 진짜 더미 끝 노드) 이터레이터를 리턴
		//	또는 끝으로 인지할 수 있는 이터레이터를 리턴
		return iterator(&_tail);
	}
	// 머리에 추가
	void push_front(T data) {
		Node *pNode =  (Node *)g_memoryPool.Alloc(sizeof(Node));
		// 데이터 넣어주고
		pNode->_data = data;

		// 1. pNode의 앞은 _head
		// 2. pNode의 다음은 _head._Next
		// 3. _head의 다음은 pNode
		// 4. _head.다음의 앞은 pNode
		pNode->_Prev = &_head;
		pNode->_Next = _head._Next;
		_head._Next = pNode;
		pNode->_Next->_Prev = pNode;

		++_size;
	}
	// 꼬리에 추가
	void push_back(T data) {
		Node *pNode = (Node *)g_memoryPool.Alloc(sizeof(Node));
		// 데이터 넣어주고
		pNode->_data = data;

		// 1. pNode의 다음은 _tail
		// 2. pNode의 앞은 _tail._Prev
		// 3. _tail.앞은 pNode
		// 4. pNode 앞의 다음은 pNode
		pNode->_Next = &_tail;
		pNode->_Prev = _tail._Prev;
		_tail._Prev = pNode;
		pNode->_Prev->_Next = pNode;

		++_size;
	}
	T pop_front() {
		Node *pNode = _head._Next;
		// 데이터 뽑아두기
		T data = pNode->_data;

		// 1. pNode의 앞의 다음은 pNode의 다음
		// 2. pNode의 다음의 앞은 pNode의 앞
		pNode->_Prev->_Next = pNode->_Next;
		pNode->_Next->_Prev = pNode->_Prev;

		--_size;
		// 할당 해지후 리턴
		g_memoryPool.Free(pNode);
		return data;
	}
	T pop_back() {
		Node *pNode = _tail._Prev;
		// 데이터 뽑아두기
		T data = pNode->_data;

		// 1. pNode의 앞의 다음은 pNode의 다음
		// 2. pNode의 다음의 앞은 pNode의 앞
		pNode->_Prev->_Next = pNode->_Next;
		pNode->_Next->_Prev = pNode->_Prev;

		--_size;
		// 할당 해지후 리턴
		g_memoryPool.Free(pNode);
		return data;
	}
	void clear() {
		// 이터레이터로 순회하며 지우기
		CList<T>::iterator iter= this->begin();
		while (iter != this->end()) {
			iter = erase(iter);
		}
	}
	int size() { return _size; };
	bool empty() { return _size <= 0; };

	// sort


	void sort() {
		quick_sort(0, _size - 1);

		/* 버블 소트
		for (auto iter1 = this->begin(); iter1 != this->end(); ++iter1) {
			for (auto iter2 = iter1; ;) {
				++iter2;
				if (iter2 == this->end()) break;
				if ((*iter1 > *iter2)) {
					T temp = *iter1;
					*iter1 = *iter2;
					*iter2 = temp;
				}
			}
		}//*/
	}

	template <typename Compare>
	void sort(Compare comp) {
		/* 버블 소트
		PRO_BEGIN(L"sort");
		for (auto iter1 = this->begin(); iter1 != this->end(); ++iter1) {
			for (auto iter2 = iter1; ;) {
				++iter2;
				if (iter2 == this->end()) break;
				if (comp(*iter1, *iter2)) {
					T temp = *iter1;
					*iter1 = *iter2;
					*iter2 = temp;
				}
			}
		}
		PRO_END(L"sort");
		//*/

		quick_sort(0, _size - 1, comp);

	}

	// 퀵소트 버전 1
	void quick_sort(int start, int end) {
		if (end <= start) return;

		T temp;

		// 퀵소트 인덱스
		int p = start;									// 피벗
		int l = start + 1;								// 왼쪽
		int r = end;									// 오른쪽

		// 실제 비교할 이터레이터
		iterator pIter = begin();
		advance(pIter, start);							// 피벗
		iterator lIter = begin();
		advance(lIter, start + 1);						// 왼쪽
		iterator rIter = begin();
		advance(rIter, end);							// 오른쪽


		while (l <= r) {
			//while (arr[p] >= arr[l]) {
			while (*pIter >= *lIter) {
				++l;
				++lIter;
				if (l >= r) break;
			}

			//while (arr[p] <= arr[r]) {
			while (*pIter <= *rIter) {
				--r;
				--rIter;
				if (r < l) break;
			}

			if (l < r) {
				//Swap(arr[l], arr[r]);
				temp = *lIter;
				*lIter = *rIter;
				*rIter = temp;
			}
		}
		//Swap(arr[p], arr[r]);
		temp = *rIter;
		*rIter = *pIter;
		*pIter = temp;

		quick_sort(start, r);
		quick_sort(l, end);
	}



	// 퀵소트 버전 2
	template <typename Compare>
	int quick_sort_patition(int start, int end, Compare comp) {
		int p = end;
		iterator pIter = begin();
		advance(pIter, p);

		// 고정되어 비교할놈
		int j = start;
		iterator jIter = begin();
		advance(jIter, j);

		// 움직이며 비교할놈
		int i = start;
		iterator iIter = begin();
		advance(iIter, i);
		T temp;
		while (i < end) {
			if (comp(*iIter, *pIter)) {
				// 피벗과 비교하면서 작은걸 왼쪽에 고정하기
				temp = *iIter;
				*iIter = *jIter;
				*jIter = temp;

				++j;
				++jIter;
			}
			++i;
			++iIter;
		}
		// 피벗 스왑
		temp = *pIter;
		*pIter = *jIter;
		*jIter = temp;

		return j; // j를 기준으로 comp에 따른 좌우 분류 완료
	}

	template <typename Compare>
	void quick_sort(int start, int end, Compare comp) {
		if (end <= start) return;


		// 퀵소트 인덱스
		int p = quick_sort_patition(start, end, comp);// 피벗
		quick_sort(start, p - 1, comp);
		quick_sort(p + 1, end, comp);
	}

	// uterator의 앞부분에 data추가
	void insert(iterator iter, T data) {
		Node *pNode = new Node;
		// 데이터 넣어주고
		pNode->_data = data;

		// 1. pNode의 다음은 iter
		// 2. pNode의 앞은 iter의 앞
		// 3. _tail.앞은 pNode
		// 4. _tail.앞의 다음은 pNode
		pNode->_Next = iter._node;
		pNode->_Prev = iter._node->_Prev;
		iter._node->_Prev = pNode;
		pNode->_Prev->_Next = pNode;

		++_size;
	}

	iterator erase(iterator iter) {
		//- 이터레이터의 그 노드를 지움.
		//- 그리고 지운 노드의 다음 노드를 카리키는 이터레이터 리턴
		Node *pNode = iter._node;
		iterator next(pNode->_Next);
		// 1. pNode의 앞의 다음은 pNode의 다음
		// 2. pNode의 다음의 앞은 pNode의 앞
		pNode->_Prev->_Next = pNode->_Next;
		pNode->_Next->_Prev = pNode->_Prev;


		--_size;
		// 할당 해지후 리턴
		g_memoryPool.Free(pNode);
		return next;
	}
	void remove(T Data) {

		for (auto iter = begin(); iter != end(); ++iter) {
			if (*iter == Data)
				erase(iter);
		}
	}

	// p규칙에 따라 지움
	template <typename UnaryPredicate>
	void remove_if(UnaryPredicate p) {
		auto iter = begin();
		while (iter != end()) {
			if (p(*iter)) {
				iter = erase(iter);
			} else
				++iter;
		}
	}

private:
	int _size = 0;
	Node _head;
	Node _tail;
};

// 현재 이터레이터를 n만큼 움직임
template <typename InputIter, typename Distance >
void advance(InputIter &it, Distance n) {
	if (n < 0) {
		for (int i = 0; i < n; i++) {
			--it;
		}
	} else {
		for (int i = 0; i < n; i++) {
			++it;
		}
	}
}