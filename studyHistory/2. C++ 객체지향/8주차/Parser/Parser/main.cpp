#include <stdio.h>
#include <string>
#include <locale.h>
#include "CParser.h"
int main() {
	CParser parser(L"Skyrim.ini");

	int iVal;
	bool bVal;
	float fVal;
	WCHAR* sVal = new WCHAR[30];
	parser.SetNamespace(L"Controls");

	if (parser.TryGetValue(L"bInvertYValues", bVal)) {
		if (bVal)
		{
			wprintf_s(L"TRUE\n");
		}
		else {
			wprintf_s(L"false\n");
		}
	}

	const WCHAR* valName = L"fGamepadHeadingSensitivity";

	if (parser.TryGetValue(valName, fVal)) {
		wprintf_s(L"%f\n", fVal);
	}


	return 0;
}