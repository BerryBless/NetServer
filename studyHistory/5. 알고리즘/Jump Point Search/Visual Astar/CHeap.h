#pragma once
#include <vector>
/*--------------------------------------------
* 
* Compare(a,b)
*  return a < b ? 1 : -1;
--------------------------------------------*/
template <typename T>
class CHeap {
private:
	std::vector<T> _heap;	// 완전이진트리 배열로 표현
	// TODO 백터 대체할껄 찾아보기
	// [i]연산을 O(1)에 수행하며 메모리를 동적으로 할당하는 것은 ??
	int (*Compare)(T a, T b);
public:
	CHeap(int (*comp)(T, T)){
		Compare = comp;
	}

	void Push(T data) {
		int now;
		int next;

		_heap.push_back(data); // 맨뒤에 넣기
		now = _heap.size() - 1; // 0부터 시작

		while (now > 0) { // now가 루트가 될때까지
			next = (now - 1) / 2; // 부모 위치

			if (Compare(_heap[now], _heap[next]) < 0)
				break;

			// swap
			T temp = _heap[now];
			_heap[now] = _heap[next];
			_heap[next] = temp;

			// 다음위치로
			now = next;
		}

	}
	T Pop() {
		T ret = _heap[0];				// 루트를 리턴
		int lastIndex = _heap.size() - 1; // 마지막 노드 위치

		int now = 0;	// 현재 비교하는 위치
		int left;		// 왼쪽 자식
		int right;		// 오른쪽 자식
		int next;		// now의 다음위치

		// 제일 마지막 노드를 루트에 가져오기
		_heap[0] = _heap[lastIndex];
		_heap.pop_back();
		lastIndex--;

		// 루트에서 내려가기
		while (true) {
			left = now * 2 + 1;
			right = now * 2 + 2;

			next = now;

			// 왼쪽값이 현재보다 크면(작으면) 왼쪽으로 이동
			if (left <= lastIndex &&
				Compare(_heap[next], _heap[left]) < 0)
				next = left;

			// 오른쪽값이 현재보다 크면(작으면) 오른쪽으로 이동
			if (right <= lastIndex &&
				Compare(_heap[next], _heap[right]) < 0)
				next = right;

			// 이동을 하지 않았다
			if (next == now)
				break;

			// swap
			T temp = _heap[now];
			_heap[now] = _heap[next];
			_heap[next] = temp;


			// 다음위치로
			now = next;
		}

		return ret;
	}
	T Peek() {
		if (_heap.size() == 0)
			return T();

		return _heap[0];
	}
	
	int size() { return _heap.size(); }
};
