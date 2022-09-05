#pragma once

#include "CorePch.h"

struct JobMessage {
	ULONGLONG _SessionID;
	WORD _Type;
	Packet *_pPacket;
};
