#include "pch.h"
#include "Session.h"

CSession::~CSession() {
	_LOG(dfLOG_LEVEL_WARNING,
		L"CALL destructor : Session  [ SID : %d, LastRecvTime : %d, socket : %d ]",
_SID, _LastRecvTime, _sock);

}
