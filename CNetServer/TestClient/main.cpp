#include "pch.h"
#include "LanClient.h"
int main() {
	int data = 10;
	LanClient client[100];
	for(int i=0;i<100;++i)
	for (int j = 0; j < 20; j++) {
		client[i].Connect();
	}
	while (true);
	return 0;
}