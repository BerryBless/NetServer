#pragma once

#include "CorePch.h"

struct JobMessage {
	SESSION_ID _sessionID;
	WORD _Type;
	Packet *_pPacket;
};
