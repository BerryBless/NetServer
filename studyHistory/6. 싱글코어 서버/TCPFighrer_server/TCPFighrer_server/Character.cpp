#include "pch.h"
#include "Character.h"
#include "Sector.h"

CCharacter::~CCharacter() {
	_LOG(dfLOG_LEVEL_WARNING,
		L"CALL destructor : Character\n\
\t_pSession[ SID : %d, LastRecvTime : %d, socket : %d ]\n\
\tWorld Position (x [%d],y [%d])\n\
\tCurSectorPosition (x [%d],y [%d])\n\
\tOldSectorPosition (x [%d],y [%d])\n\
\tHP[%d]",
		_pSession->_SID, _pSession->_LastRecvTime, _pSession->_sock,
		_X,_Y,
		_curSecPos._X, _curSecPos._Y,
		_oldSecPos._X, _oldSecPos._Y,
		_chHP
	);
}
