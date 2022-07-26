#include "CEffectObject.h"
#include "Managers.h"

CEffectObject::CEffectObject( int iX, int iY) : CBaseObject() {
	_eObjectType = e_OBJECTTYPE::EFFECT;
	
	SetPosition(iX, iY);
	// 이펙트 스프라이트 지정
	SetSprite((int) e_SPRITE::eEFFECT_SPARK_01, (int) e_SPRITE::eEFFECT_SPARK_MAX, dfDELAY_EFFECT);

}

CEffectObject::~CEffectObject() {
}

void CEffectObject::Update() {
	NextFrame();
	if (IsEndFrame() == TRUE) {
		this->Destroy();
	}
}

void CEffectObject::Render(BYTE *bypDest, int iDestWidth, int iDestHeight, int iDestPitch) {
	CSpriteDib *spriteInst = I_SPRITEDIB;

	spriteInst->DrawSprite(GetSprite(), _iCurX, _iCurY, bypDest, iDestWidth, iDestHeight, iDestPitch);
}
