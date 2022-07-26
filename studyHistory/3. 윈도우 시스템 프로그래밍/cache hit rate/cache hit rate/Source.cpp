// 참조 : https://github.com/sonsidal/cache_simulator/blob/master/cache.c

#include <stdio.h>
#include <math.h>


typedef unsigned long long PTRBITS; // 포인터 크기만큼 연산가능한 비트


class CCacheArchitecture {
private:
	int _iL1size ;
	int _iCacheWay ;
	int _iCachelineSize ;

	int _iTagBits;
	int _iIndexBits;
	int _iOffsetBits;

	PTRBITS _pbTagMask = 0;
	PTRBITS _pbIndexMask = 0;
	PTRBITS _pbOffsetMask = 0;

public:
	CCacheArchitecture(int iL1size, int iCacheWay, int iCachelineSize) {
		_iL1size = iL1size;
		_iCacheWay = iCacheWay;
		_iCachelineSize = iCachelineSize;

		// index 구하기
		int t = iL1size * 1024; // kb to byte
		t = t / iCachelineSize; // 이 캐쉬엔 몇개의 캐시라인이 들어가나
		t = t / iCacheWay;		// 이 캐쉬엔 몇개의 웨이가 하나의 인덱스를 이루나
		// 그 인덱스가 몇개의 비트로 표현이 가능한가 64면 0~63 이니 
		_iIndexBits = 0;
		while (t >>= 1) { _iIndexBits++; }

		// offset 구하기
		// 캐시라인을 몇개의 비트로 표현이 가능한가 64면 0~63 이니 
		_iOffsetBits = 0;
		while (iCachelineSize >>= 1) { _iOffsetBits++; }

		// tag 구하기 주소길이 - (index + offset)
		t = sizeof(void *); // 주소길이 (byte)
		t = t * 8;			// byte to bit
		_iTagBits = t - (_iIndexBits + _iOffsetBits);


		// tagmask
		_pbTagMask = ~_pbTagMask;
		_pbTagMask = _pbTagMask << (_iIndexBits + _iOffsetBits);

		// offsetmask
		for (int i = 0; i < _iOffsetBits; i++) {
			_pbOffsetMask = _pbOffsetMask << 1;
			_pbOffsetMask = _pbOffsetMask | 1;
		}
		// indexmask
		_pbIndexMask = _pbTagMask | _pbOffsetMask;
		_pbIndexMask = ~_pbIndexMask;
	}

	PTRBITS PtrMaskTag(PTRBITS ptr) { return ptr & _pbTagMask; }
	PTRBITS PtrMaskIndex(PTRBITS ptr) { return ptr & _pbIndexMask; }
	PTRBITS PtrMaskOffset(PTRBITS ptr) { return ptr & _pbOffsetMask; }

	void PrintPtrInfo(PTRBITS ptr) {
		printf_s("%#I64x : tag(%#I64x), index(%#I64x), offset(%#I64x)\n", ptr,
			PtrMaskTag(ptr), PtrMaskIndex(ptr), PtrMaskOffset(ptr));
	}
	void PrintCacheInfo() {
		printf_s("\n------------------------------\n");
		printf_s("L1 Size(%d kb), Cache Line Size(%d byte), Cache Way(%d-way)\n",_iL1size, _iCachelineSize, _iCacheWay);
		printf_s("tag\t: %3d bits, mask ( %#018I64x )\n", _iTagBits, _pbTagMask);
		printf_s("index\t: %3d bits, mask ( %#018I64x )\n", _iIndexBits, _pbIndexMask);
		printf_s("offset\t: %3d bits, mask ( %#018I64x )\n", _iOffsetBits, _pbOffsetMask);
		printf_s("------------------------------\n");
	}
};

static CCacheArchitecture _cache(32, 8, 64);

void A() {
	printf_s("\n===================================================\n");
	printf_s("A()\n");
	int arr[2048];
	for (int i = 0; i < 2048; i++)
		_cache.PrintPtrInfo((PTRBITS) (arr + i));
	printf_s("\n===================================================\n");
}
void B() {
	printf_s("\n===================================================\n");
	printf_s("B()\n");
	int *arr;
	arr = new int[1024];
	for (int i = 0; i < 10; i++)
		_cache.PrintPtrInfo((PTRBITS) (arr + i));
	printf_s("\n===================================================\n");

}

int main() {

	//fprintf_s(fp,"iL1size(byte), iCacheWay(count), iCachelineSize(byte)");
	//scanf_s("%d %d %d",&iL1size, &iCacheWay, &iCachelineSize);

	printf_s("\n===================================================\n");
	printf_s("main()\n");
	int arr[2048];
	_cache.PrintCacheInfo();
	for (int i = 0; i < 10; i++)
		_cache.PrintPtrInfo((PTRBITS) (arr + i));
	printf_s("\n===================================================\n");
	A();
	B();
	return 0;
}