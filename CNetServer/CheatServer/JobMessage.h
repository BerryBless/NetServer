#pragma once

#include "CorePch.h"

struct JobMessage {
	ULONGLONG _sessionID;
	WORD _Type;
	Packet *_pPacket;
};
