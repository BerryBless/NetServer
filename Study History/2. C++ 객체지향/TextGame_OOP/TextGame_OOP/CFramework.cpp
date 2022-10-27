#include "pch.h"
#include "CFramework.h"
#include "Managers.h"

#pragma region Singleton 
CFramework::CFramework() {
	// 맴버변수 초기값으로
	_frame = 0;
	_objects.clear();
	memset(_objectTile, NULL, sizeof(_objectTile));
}
CFramework::~CFramework() {
	// 소멸시점이 프로그렘 종료시점이라 따로 안만듦
}
#pragma endregion

// 게임이 시작할때
void CFramework::Init() {
	// 타이틀 씬 로드
	I_SCENEMANAGER->Load(L"title");
}

// 게임 중 게임로직 업데이트
void CFramework::Update() {
	++_frame;//프레임 하나증가

	// 씬바꾸기 시도
	I_SCENEMANAGER->Change();

	// 오브젝트 순회하며 업데이트
	for (ITERATOR iter = _objects.begin(); iter != _objects.end(); ++iter) {
		// update
		(*iter)->Update();

	}
	
	I_SCENEMANAGER->Update();

	// _destroy큐에 있는걸 모두 삭제
	while (!_destroy.empty()) {
		ITERATOR iter = _destroy.front();
		_destroy.pop();

		_objectTile[(*iter)->_pos.Y][(*iter)->_pos.X] = NULL;	// 충돌타일에서 삭제
		(*iter)->Destroy();										// 소멸될시 그 객체가 해야할일 (이펙트 생성 등)
		delete (*iter);											// 그 객체 delete

		_objects.erase(iter);
	}


}


// 게임 중 화면 그리기
void CFramework::Render() {
	// 스크린 버퍼
	CScreenBuffer *sbInst = I_SCREENBUFFER;

	// 버퍼 초기화
	sbInst->Buffer_Clear();

	// 씬에서 그릴것 그리기
	I_SCENEMANAGER->Render();

	// 오브젝트 순회하며 그리기
	for (ITERATOR iter = _objects.begin(); iter != _objects.end(); ++iter) {
		(*iter)->Render();
	}

	// 화면에 출력
	sbInst->Buffer_Flip();
}

// 게임종료 확인
bool CFramework::InGame() {
	return I_SCENEMANAGER->InGame();
}

// 오브젝트 등록, 파괴
// 사용법 INSTANTIATE(new CPlayer);
IObject *CFramework::Instantiate(IObject *obj) {
	// _objects, _objectTile 에 등록하기
	// 충돌체크
	if (Collison(obj->_pos, obj) == true) {
		// 생성시 충돌이 일어나서 OnCollision, Destroy 호출 하고 바로 소멸 
		obj->Destroy();
		delete obj;	// TODO delete 모아서 하기?
		return NULL;
	}

	// 오브젝트 프레임워크에 등록
	this->_objects.push_back(obj);
	this->_objectTile[obj->_pos.Y][obj->_pos.X] = obj;
	return obj;
}
// 큐에 넣어두고 업데이트가 끝나면 삭제실행
// 사용법 DESTROY(pPlayer)
void CFramework::Destroy(IObject *obj) {
	// _destroy큐에 넣어두기, 큐를 비우는 시점 : 매프레임의 끝
	for (ITERATOR iter = _objects.begin(); iter != _objects.end(); ++iter) {
		if ((*iter) == obj) {
			// 2번 삭제 방지
			if ((*iter)->_activate == true) {
				_destroy.push(iter);
			}
			return;
		}
	}
}

// 움직일 오브젝트 obj 와 목적지에 있는 오브젝트 pTarget의 충돌체크
// 충돌이 가능한 상황이면 
// obj->OnCollison(pTarget) 호출
bool CFramework::Collison(int iX, int iY, IObject *obj) {
	// 범위체크
	if (iY < 0 || dfSCREEN_HEIGHT <= iY)
		return false;
	if (iX < 0 || dfSCREEN_WIDTH-1 <= iX)
		return false;

	IObject *pTarget = Find(iX, iY);
	// 오브젝트가 있나?
	if (pTarget != NULL) {
		// 그 오브젝트가 충돌을 체크하느냐?
		if (pTarget->_collison) {
			// 가는데 충돌할 오브젝트가 있음 충돌!
			obj->OnCollison(pTarget);
			return true;
		}
	}
	return false;
}

bool CFramework::Collison(Position pos, IObject *obj) {
	return Collison(pos.X,pos.Y, obj);
}

// 오브젝트 이동 시도
bool CFramework::TryMove(int iX, int iY, IObject *mobj) {
	// 범위체크
	if (iY < 0 || dfSCREEN_HEIGHT <= iY)
		return false;
	if (iX < 0 || dfSCREEN_WIDTH-1 <= iX)
		return false;

	// 충돌체크
	if (Collison(iX, iY, mobj) == true) {
		// 충돌이 일어남
		return false;
	}
	// 이동
	_objectTile[iY][iX] = _objectTile[mobj->_pos.Y][mobj->_pos.X];	// 포인터 움직일곳에 넣고
	_objectTile[mobj->_pos.Y][mobj->_pos.X] = NULL;					// 원래 있던곳 비우기
	// 인스턴스에도 알려주기
	mobj->_pos.X = iX;
	mobj->_pos.Y = iY;
	return true;
}

bool CFramework::TryMove(Position pos, IObject *mobj) {
	// 래핑
	return TryMove(pos.X, pos.Y, mobj);
}



IObject *CFramework::Find(int iX, int iY) {
	// 범위체크
	if (iY < 0 || dfSCREEN_HEIGHT <= iY)
		return NULL;
	if (iX < 0 || dfSCREEN_WIDTH-1 <= iX)
		return NULL;
	// 이좌표에 있는 오브젝트 반환
	return _objectTile[iY][iX];
}

IObject *CFramework::Find(Position pos) {
	// 래핑
	return Find(pos.X, pos.Y);
}

IObject *CFramework::Find(ObjectType type) {
	// 이타입의 오브젝트중 _objects 에서 첫번째로 마주치는 오브젝트 반환 못찾으면 nullptr 반환
	for (ITERATOR iter = _objects.begin(); iter != _objects.end(); ++iter) {
		if ((*iter)->_type == type)
			return *iter;
	}
	return NULL;
}

// 오브젝트 모두 지우기
void CFramework::claer() {
	// _objects 순회하며 delete 하면서 지우기
	ITERATOR iter = this->_objects.begin();
	while (iter != this->_objects.end()) {
		delete (*iter);
		iter = _objects.erase(iter);
	}
	// _objectTile 널값으로 밀어버리기
	memset(_objectTile, NULL, sizeof(_objectTile));
}
