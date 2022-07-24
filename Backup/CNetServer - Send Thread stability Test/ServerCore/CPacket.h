/////////////////////////////////////////////////////////////////////
// www.gamecodi.com						이주행 master@gamecodi.com
//
//
/////////////////////////////////////////////////////////////////////
/*---------------------------------------------------------------

	Packet.

	네트워크 패킷용 클래스.
	간편하게 패킷에 순서대로 데이타를 In, Out 한다.

	- 사용법.

	CPacket cPacket;

	넣기.
	clPacket << 40030;		or	clPacket << iValue;	(int 넣기)
	clPacket << 1.4;		or	clPacket << fValue;	(float 넣기)


	빼기.
	clPacket >> iValue;		(int 빼기)
	clPacket >> byValue;		(BYTE 빼기)
	clPacket >> fValue;		(float 빼기)

	!.	삽입되는 데이타 FIFO 순서로 관리된다.
		환형 큐는 아니므로, 넣기(<<).빼기(>>) 를 혼합해서 사용하지 않도록 한다



	* 실제 패킷 프로시저에서의 처리

	BOOL	netPacketProc_CreateMyCharacter(CPacket *pPacket)
	{
		DWORD dwSessionID;
		short shX, shY;
		char chHP;
		BYTE byDirection;

//		*pPacket >> dwSessionID >> byDirection >> shX >> shY >> chHP;


		*pPacket >> dwSessionID;
		*pPacket >> byDirection;
		*pPacket >> shX;
		*pPacket >> shY;
		*pPacket >> chHP;

		...
		...
	}


	* 실제 메시지(패킷) 생성부에서의 처리

	void	mpMoveStart(CPacket *pPacket, BYTE byDirection, short shX, short shY)
	{
		st_NETWORK_PACKET_HEADER	stPacketHeader;
		stPacketHeader.byCode = dfNETWORK_PACKET_CODE;
		stPacketHeader.bySize = 5;
		stPacketHeader.byType = dfPACKET_CS_MOVE_START;

		pPacket->PutData((char *)&stPacketHeader, dfNETWORK_PACKET_HEADER_SIZE);

		*pPacket << byDirection;
		*pPacket << shX;
		*pPacket << shY;

	}

----------------------------------------------------------------*/
#ifndef  __PACKET__
#define  __PACKET__
#include "CObjectPool_TLS.hpp"
class CPacket {
public:

	/*---------------------------------------------------------------
	Packet Enum.

	----------------------------------------------------------------*/
	enum en_PACKET {
		eBUFFER_DEFAULT = 1400		// 패킷의 기본 버퍼 사이즈.
	};

	//////////////////////////////////////////////////////////////////////////
	// 생성자, 파괴자.
	//
	// Return:
	//////////////////////////////////////////////////////////////////////////
	CPacket();

	virtual	~CPacket();


	//////////////////////////////////////////////////////////////////////////
	// 패킷  파괴.
	//
	// Parameters: 없음.
	// Return: 없음.
	//////////////////////////////////////////////////////////////////////////
	void	Release(void);


	//////////////////////////////////////////////////////////////////////////
	// 패킷 청소.
	//
	// Parameters: 없음.
	// Return: 없음.
	//////////////////////////////////////////////////////////////////////////
	void	Clear(void);


	//////////////////////////////////////////////////////////////////////////
	// 버퍼 사이즈 얻기.
	//
	// Parameters: 없음.
	// Return: (int)패킷 버퍼 사이즈 얻기.
	//////////////////////////////////////////////////////////////////////////
	int	GetBufferSize(void) { return _iBufferSize - sizeof(HEADER); }
	//////////////////////////////////////////////////////////////////////////
	// 현재 사용중인 사이즈 얻기.
	//
	// Parameters: 없음.
	// Return: (int)사용중인 데이타 사이즈.
	//////////////////////////////////////////////////////////////////////////
	int		GetSendSize(void);
	int		GetDataSize(void);



	//////////////////////////////////////////////////////////////////////////
	// 버퍼 포인터 얻기.
	//
	// Parameters: 없음.
	// Return: (char *)버퍼 포인터.
	//////////////////////////////////////////////////////////////////////////
	char *GetBufferPtr(void) { return _pBuffer; }
	char *GetSendPtr(void) { return _pBuffer + _sendPos; }
	char *GetReadPtr(void) { return _pBuffer + _readPos; }
	char *GetWritePtr(void) { return _pBuffer + _writePos; }

	//////////////////////////////////////////////////////////////////////////
	// 버퍼 Pos 이동. (음수이동은 안됨)
	// GetBufferPtr 함수를 이용하여 외부에서 강제로 버퍼 내용을 수정할 경우 사용. 
	//
	// Parameters: (int) 이동 사이즈.
	// Return: (int) 이동된 사이즈.
	//////////////////////////////////////////////////////////////////////////
	int		MoveWritePos(int iSize);
	int		MoveReadPos(int iSize);


	//////////////////////////////////////////////////////////////////////////
	// 	   헤더 설정
	//////////////////////////////////////////////////////////////////////////
	void SetHeader();



	/* ============================================================================= */
	// 연산자 오버로딩
	/* ============================================================================= */
	CPacket &operator = (CPacket &clSrcPacket);

	//////////////////////////////////////////////////////////////////////////
	// 넣기.	각 변수 타입마다 모두 만듬.
	//////////////////////////////////////////////////////////////////////////
	CPacket &operator << (unsigned char byValue);
	CPacket &operator << (char chValue);

	CPacket &operator << (short shValue);
	CPacket &operator << (unsigned short wValue);

	CPacket &operator << (int iValue);
	CPacket &operator << (unsigned long dwValue);
	CPacket &operator << (float fValue);

	CPacket &operator << (__int64 iValue);
	CPacket &operator << (double dValue);


	//////////////////////////////////////////////////////////////////////////
	// 빼기.	각 변수 타입마다 모두 만듬.
	//////////////////////////////////////////////////////////////////////////
	CPacket &operator >> (unsigned char &byValue);
	CPacket &operator >> (char &chValue);

	CPacket &operator >> (short &shValue);
	CPacket &operator >> (unsigned short &wValue);

	CPacket &operator >> (int &iValue);
	CPacket &operator >> (unsigned long &dwValue);
	CPacket &operator >> (float &fValue);

	CPacket &operator >> (__int64 &iValue);
	CPacket &operator >> (double &dValue);




	//////////////////////////////////////////////////////////////////////////
	// 데이타 얻기.
	//
	// Parameters: (char *)Dest 포인터. (int)Size.
	// Return: (int)복사한 사이즈.
	//////////////////////////////////////////////////////////////////////////
	int		GetData(char *chpDest, int iSize);

	//////////////////////////////////////////////////////////////////////////
	// 데이타 삽입.
	//
	// Parameters: (char *)Src 포인터. (int)SrcSize.
	// Return: (int)복사한 사이즈.
	//////////////////////////////////////////////////////////////////////////
	int		PutData(char *chpSrc, int iSrcSize);

	//////////////////////////////////////////////////////////////////////////
	// 할당받으며 참조포인트 리셋후 하나 증가
	//
	//////////////////////////////////////////////////////////////////////////
	static CPacket *AllocAddRef();

	//////////////////////////////////////////////////////////////////////////
	// 이 패킷의 참조 카운터 증가
	//
	//////////////////////////////////////////////////////////////////////////
	void	AddRef(int logic = 0);


	//////////////////////////////////////////////////////////////////////////
	// 이 패킷의 참조 카운터 감소
	// 참조카운터가 0이면 할당해지!
	//////////////////////////////////////////////////////////////////////////
	void	SubRef(int logic = 0);

	void ResetRef();

	int _logSubLogic[100]; // 디버깅용 SubRef의 0dl 언재되었나
	unsigned long _logidx = 0;


	static ObjectPool_TLS<CPacket> _packetPool;
	//static CObjectPool<CPacket> _packetPool;

protected:
	//------------------------------------------------------------
	// 현재 버퍼의 사이즈.
	//------------------------------------------------------------
	int	_iBufferSize;

	//------------------------------------------------------------
	// 현재 버퍼에 사용중인 사이즈.
	//------------------------------------------------------------
	//int	_iDataSize;

	//------------------------------------------------------------
	// 현재 버퍼에 사용중인 메모리 포인터.
	//------------------------------------------------------------
	char _pBuffer[eBUFFER_DEFAULT];

	//------------------------------------------------------------
	// 현재 버퍼에 읽거나 쓸 위치
	//------------------------------------------------------------
	int _writePos; // 쓸 위치
	int _readPos;  // 읽을 위치
	int _sendPos; // CLan전용 헤더 + 페이로드

	//------------------------------------------------------------
	// 참조카운터
	//------------------------------------------------------------
	struct RefCount {
		union {
			long counter = 0;
			struct {
				short count;
				short isFreed; // 메모리풀에 돌아갔는지
			} refStaus;
		};
	};
	RefCount _refCount;
	// 헤더 : 길이
	// buffer :: XX XX AA AA AA~~~ XX XX: CLan헤더, AA ~ 내용물
	// 헤더 : AA의 길이(바이트)
	struct HEADER {
		unsigned short len;
	};

};



#endif
