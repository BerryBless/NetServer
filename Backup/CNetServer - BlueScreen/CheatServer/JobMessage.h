#pragma once

#include "CorePch.h"

struct JobMessage {
	ULONGLONG _SessionID;
	WORD _Type;
	CPacket *_pPacket;
};
