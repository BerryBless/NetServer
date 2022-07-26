#include "OtherClass.h"

OtherClass::OtherClass() {
	_parr = (int *)g_memoryPool.Alloc(20);
}

OtherClass::~OtherClass() {
	g_memoryPool.Free(_parr);
}
