#include "CCameraObject.h"

void CCameraObject::Update() {
	if (_pTargetObject == NULL) return;
	_iCurX = _pTargetObject->GetCurX() - (dfSCREEN_WIDTH / 2);
	_iCurY = _pTargetObject->GetCurY() - (dfSCREEN_HEIGHT / 2);

	if (_iCurX > dfCAMERA_MAX_WIDTH)
		_iCurX = dfCAMERA_MAX_WIDTH;
	else if (_iCurX < dfCAMERA_MIN_WIDTH)
		_iCurX = dfCAMERA_MIN_WIDTH;

	if (_iCurY > dfCAMERA_MAX_HEIGHT)
		_iCurY = dfCAMERA_MAX_HEIGHT;
	else if (_iCurY < dfCAMERA_MIN_HEIGHT)
		_iCurY = dfCAMERA_MIN_HEIGHT;
}

void CCameraObject::SetTarget(CBaseObject *pTarget) {
	if (pTarget == nullptr) return;
	_pTargetObject = pTarget;
}
