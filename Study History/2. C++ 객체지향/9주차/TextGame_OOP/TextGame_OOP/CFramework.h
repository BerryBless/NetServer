#pragma once
#include "CList.h"		// 리스트
#include "CQueue.h"		// 큐
#include "IObject.h"	// 오브젝트 인터페이스

// ============================================================================
//						매크로함수
// ---------------------------------------------------------------------------
// 싱글톤을 사용하기 편하게 매크로로 지정
// 사용법:
// 1. CFramework *inst = I_FRAMEWORK 후 일반 싱글톤 사용하듯 하용
// 2. CPlayer *myPlayer = (CPlayer*)INSTANTIATE(new CPlayer(x,y,hp,dmg));	프레임워크에 new한 오브젝트 등록후 반환
// 3. 등록된 오브젝트의 맴버함수에서 OBJECT_MOVE(x,y) 호출, this 오브젝트가 [x][y] 좌표로 이동시 true, 실패시 OnCollison 호출후 false 반환
// 4. DESTROY(this); 이 오브젝트는 이 프레임 맨 마지막에 소멸됨
// ============================================================================
#define I_FRAMEWORK			CFramework::GetInstance()					// CFramework *inst = I_FRAMEWORK 후 일반 싱글톤 사용하듯 하용
#define INSTANTIATE(pObj)	I_FRAMEWORK->Instantiate((IObject*)pObj)	// 프레임워크에 new한 오브젝트 등록후 반환
#define OBJECT_MOVE(x,y)	I_FRAMEWORK->TryMove(x,y,this)				// 이 오브젝트가 [x][y] 좌표로 이동시 true, 실패시 OnCollison 호출후 false 반환
#define DESTROY(pObj)		I_FRAMEWORK->Destroy((IObject*)pObj)		// 이 오브젝트는 이 프레임 맨 마지막에 소멸됨

// ============================================================================
//						프레임워크
// ---------------------------------------------------------------------------
// 게임내 존재하는 모든 오브젝트를 관리하는 싱글톤 클래스
// ---
// 맴버 변수
// _frame		: 시작부터 지금까지 몇 _frame만큼 지났음.
// _ingame		: TEMP : 게임이 진행중임
// _objects		: "지금" 게임에 존재하는 오브젝트
// _objectTile	: "지금" 게임에 존재하는 오브젝트가 있는 좌표계 (사용이유 : 좌표로 오브젝트 찾을때 시간복잡도 O(1))
// _destroy		: 이 프레임 마지막에 삭제될 오브젝트(ITERATOR)가 있는 큐
// ---
// main() 에서 실행할 함수
// main(){
//		Init();
//		while(true){
//			1회 반복 = 1 프레임
//			Update();
//			if(end() == false) break;
//			Render();
//		}
// }
// --
// Init();			: 초기화
// Update();		: 메인루프 : 게임로직 업데이트, 신 업데이트 ,마지막에 _destroy큐에 있는 오브젝트 모두 delete
// Render();		: 메인루프 : 오브젝트 랜더링
// InGame()			: 게임이 끝났는지
// ---
// 오브젝트 관리 프레임워크
// Instantiate(pObj)	: obj를 받아서 충돌체크후 _objects, _objectTile에 등록
// Destroy(pObj)		: _objects 순회하며 obj와 일치하는 이터레이터 _destroy에 푸쉬하기
// Collison(pos, pObj)	: pObj의 목적지 pos에 오브젝트(pTarget)가 존재하면 pObj->OnCollison(pTarget) 호출하고 true를 리턴 
// Find(type);			: 이타입의 오브젝트중 _objects 에서 첫번째로 마주치는 오브젝트 반환 못찾으면 nullptr 반환
// TryMove( pos, pObj)	: mobj를 pos로 이동 시도. 실패했고, 실패한이유가 target 오브젝트와 부딭친거면 mobj->OnCollison(target) 호출
// Find( )				: 매개변수 정보에 일치하는 오브젝트 반환
// claer()				: _objects, _objectTile 초기값으로, 존재하는 오브젝트 모두 delete하기
// Frame()				: 현재 몇 프레임째 진행중인지 리턴, Clock() 대용
// ============================================================================

class CFramework {
#pragma region Singleton 
	// 싱글톤
private:
	CFramework();	// 맴버 변수를 모두 초기값으로
	~CFramework();	// 소멸시점이 프로그렘 종료시점이라 따로 안만듦
public:
	// 전역 인스턴스를 얻어올 전역 함수
	static CFramework *GetInstance() {
		static CFramework _Instance;
		return &_Instance;
	};
#pragma endregion

private:
	typedef CList< IObject *>::iterator ITERATOR; // 이터레이터 쓰기 편하게 typedef
private:
	frame_t						_frame;						//	시작부터 지금까지 몇 _frame만큼 지났음.
	CList <IObject *>			_objects;					// "지금" 게임에 존재하는 오브젝트
	IObject *_objectTile[dfSCREEN_HEIGHT][dfSCREEN_WIDTH];	// "지금" 게임에 존재하는 오브젝트가 있는 좌표계 
	CQueue <ITERATOR>			_destroy;					// 이 프레임 마지막에 삭제될 오브젝트(ITERATOR)가 있는 큐
public:
	// main() 에서 실행할 함수
	void Init();		// 초기화
	void Update();		// 메인루프 : 게임로직 업데이트
	void Render();		// 메인루프 : 오브젝트 랜더링
	bool InGame();		// 게임이 진행중인지

public:
	IObject *Instantiate(IObject *obj);		// obj를 받아서 충돌체크후 _objects, _objectTile에 등록
	void Destroy(IObject *obj);				// _objects 순회하며 obj와 일치하는 이터레이터 _destroy에 푸쉬하기

	// obj의 목적지 pos에 오브젝트(pTarget)가 존재하면 obj->OnCollison(pTarget) 호출하고 true를 리턴 
	bool Collison(int iX, int iY, IObject *obj);
	bool Collison(Position pos, IObject *obj);	

	// 오브젝트 이동(목적지 좌표, 이동할 오브젝트)
	bool TryMove(int iX, int iY, IObject *mobj);
	bool TryMove(Position pos, IObject *mobj);		

	// 매개변수 정보에 일치하는 오브젝트 반환
	IObject *Find(int iX, int iY);	// 이 좌표에 있는 오브젝트
	IObject *Find(Position pos);	// 이 좌표에 있는 오브젝트
	IObject *Find(ObjectType type);	// 이타입의 오브젝트중 _objects 에서 첫번째로 마주치는 오브젝트 반환 못찾으면 NULL 반환

	// 관리하는 오브젝트 모두 비우기
	void claer();

	// 현재 몇 프레임째 진행중인지 리턴
	frame_t Frame() { return _frame; };
};

