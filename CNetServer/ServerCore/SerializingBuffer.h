#ifndef  __PACKET__
#define  __PACKET__
#include "ObjectPool_TLS.hpp"

#define PACKET_NET_HEADER		NET_HEADER
#define PACKET_NET_HEADER_SIZE	sizeof(PACKET_NET_HEADER)
#define PACKET_LAN_HEADER		LAN_HEADER
#define PACKET_LAN_HEADER_SIZE	sizeof(PACKET_LAN_HEADER)

#pragma pack(push, 1)
struct LAN_HEADER {
	unsigned short	len;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct NET_HEADER {
	unsigned char	code;
	unsigned short	len;
	unsigned char	randKey;
	unsigned char	checksum;

};
#pragma pack(pop)


class Packet {
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
	Packet();

	virtual	~Packet();


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
	int	GetBufferSize(void) { return _iBufferSize; }
	//////////////////////////////////////////////////////////////////////////
	// 현재 사용중인 사이즈 얻기.
	//
	// Parameters: 없음.
	// Return: (int)사용중인 데이타 사이즈.
	//////////////////////////////////////////////////////////////////////////
	int		GetSendSize(void) { return _writePos - _sendPos; }
	int		GetDataSize(void) {	return _writePos - _readPos;}



	//////////////////////////////////////////////////////////////////////////
	// 버퍼 포인터 얻기.
	//
	// Parameters: 없음.
	// Return: (char *)버퍼 포인터.
	//////////////////////////////////////////////////////////////////////////
	unsigned char *GetBufferPtr(void) { return _pBuffer; }
	unsigned char *GetSendPtr(void) { return _pBuffer + _sendPos; }
	unsigned char *GetReadPtr(void) { return _pBuffer + _readPos; }
	unsigned char *GetWritePtr(void) { return _pBuffer + _writePos; }

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
	void SetLanHeader();
	void SetNetHeader();

	void Encode();
	bool Decode();
	bool VerifyCheckSum();

	//////
	// debug
	/////
	void PrintPacket();
	/* ============================================================================= */
	// 연산자 오버로딩
	/* ============================================================================= */
	Packet &operator = (Packet &clSrPacket);

	//////////////////////////////////////////////////////////////////////////
	// 넣기.	각 변수 타입마다 모두 만듬.
	//////////////////////////////////////////////////////////////////////////
	Packet &operator << (unsigned char byValue);
	Packet &operator << (char chValue);

	Packet &operator << (short shValue);
	Packet &operator << (unsigned short wValue);

	Packet &operator << (int iValue);
	Packet &operator << (unsigned long dwValue);
	Packet &operator << (float fValue);

	Packet &operator << (__int64 iValue);
	Packet &operator << (double dValue);


	//////////////////////////////////////////////////////////////////////////
	// 빼기.	각 변수 타입마다 모두 만듬.
	//////////////////////////////////////////////////////////////////////////
	Packet &operator >> (unsigned char &byValue);
	Packet &operator >> (char &chValue);

	Packet &operator >> (short &shValue);
	Packet &operator >> (unsigned short &wValue);

	Packet &operator >> (int &iValue);
	Packet &operator >> (unsigned long &dwValue);
	Packet &operator >> (float &fValue);

	Packet &operator >> (__int64 &iValue);
	Packet &operator >> (double &dValue);




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
	static Packet *AllocAddRef();

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

	//int _logSubLogic[100]; // 디버깅용 SubRef의 0dl 언재되었나
	//unsigned long _logidx = 0;


	static ObjectPool_TLS<Packet> _packetPool;
	//static ObjectPool<Packet> _packetPool;

protected:
	//------------------------------------------------------------
	// 현재 버퍼의 사이즈.
	//------------------------------------------------------------
	int	_iBufferSize;

	//------------------------------------------------------------
	// 이 패킷이 인코딩 되었는지
	//------------------------------------------------------------
	bool _isEncode = false;

	//------------------------------------------------------------
	// 현재 버퍼에 사용중인 메모리 포인터.
	//------------------------------------------------------------
	unsigned char _pBuffer[eBUFFER_DEFAULT];

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



public:
	static constexpr int MSS = 1460;
	static constexpr int PACKET_CODE = 0x77;
	static constexpr int FIXED_KEY = 0x32;
	// 헤더 : 길이
	// buffer :: XX XX AA AA AA~~~ XX XX: CLan헤더, AA ~ 내용물
	// 헤더 : AA의 길이(바이트)

};



#endif
