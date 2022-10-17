#include <iostream>
#include "CTest.h"
int main() {
	CTest* ct = new CTest(3);
	ct->Show();
	ct->Set(255);
	ct->Get();
	return 0;
}
