#include "CBaseObject.h"
#include "Managers.h"

CBaseObject::CBaseObject() {
	_bActivate = TRUE;
	_bEndFrame = FALSE;
	_dwActionInput = -1;
	_iCurX = 0;
	_iCurY = 0;
	_iDelayCount = 0;
	_iFrameDelay = 0;
	_iObjectID = 0;
	_eObjectType = e_OBJECTTYPE::MAX;
	_iSpriteMax = 0;
	_iSpriteNow = 0;
	_iSpriteStart = 0;
}

CBaseObject::~CBaseObject() {
}

void CBaseObject::Update() {
	NextFrame();
}

void CBaseObject::Render(BYTE *bypDest, int iDestWidth, int iDestHeight, int iDestPitch) {
	// 출력
	CSpriteDib *spriteInst = I_SPRITEDIB;

	spriteInst->DrawSprite(GetSprite(), _iCurX, _iCurY, bypDest, iDestWidth, iDestHeight, iDestPitch);
}

void CBaseObject::Collision(CBaseObject *stAttacker) {
	// HIT
}



void CBaseObject::Destroy() {
	if (_bActivate == FALSE)return;
	_bActivate = FALSE;
}

void CBaseObject::NextFrame() {
	if (0 > _iSpriteStart) return;

	_iDelayCount++;
	if (_iDelayCount >= _iFrameDelay) {
		_iDelayCount = 0;

		// 에니메이션 끝면 처음으로
		_iSpriteNow++;
		if (_iSpriteNow >= _iSpriteMax) {
			_iSpriteNow = _iSpriteStart;
			_bEndFrame = TRUE;
		}

	}
}

void CBaseObject::ActionInput(DWORD dwAction) {
	_dwActionInput = dwAction;
}


void CBaseObject::SetPosition(int iX, int iY) {
	_iCurX = iX;
	_iCurY = iY;
}

void CBaseObject::SetSprite(int iSpriteStart, int iSpriteMax, int iFrameDelay) {
	_iSpriteStart = iSpriteStart;
	_iSpriteMax = iSpriteMax;
	_iFrameDelay = iFrameDelay;

	_iSpriteNow = _iSpriteStart;
	_iDelayCount = 0;
	_bEndFrame = FALSE;
}

int CBaseObject::GetCurX() {
	return _iCurX;
}

int CBaseObject::GetCurY() {
	return _iCurY;
}

int CBaseObject::GetObjectID() {
	return _iObjectID;
}

int CBaseObject::GetObjectType() {
	return (int)_eObjectType;
}

int CBaseObject::GetSprite() {
	return _iSpriteNow;
}

BOOL CBaseObject::IsActivate() {
	return _bActivate;
}

BOOL CBaseObject::IsEndFrame() {
	return _bEndFrame;
}

