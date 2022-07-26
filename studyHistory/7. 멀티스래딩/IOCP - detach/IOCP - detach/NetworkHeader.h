#pragma once

#define dfPACKET_HEADER_LENGTH (int)sizeof(NETWORK_HEADER)

struct NETWORK_HEADER{
	unsigned short _payloadSize;
};