#pragma once
#include "CObjectPool.hpp"
#include <wtypes.h>

template <typename DATA>
class ObjectPool_TLS {
	friend class CChunk;
private:
	class CChunk;
	struct st_Element {
		DATA			data;
		CChunk *pOrigin;
		void *code;
	};

	class CChunk {
	public:
		DATA *Alloc(void);
		bool	Free(DATA *pData);

		enum {
			MAX_SIZE = 1000
		};

		st_Element					_ObjectArr[MAX_SIZE];
		int							_AllocCount = 0;
		DWORD						_threadID;
		ObjectPool_TLS *_pObjPool = nullptr;
		alignas(64) LONG			_FreeCount;
	};
private:
	alignas(64) DWORD			_Size;
	bool						_bSizeCheck;
	bool						_bPlacementNew;
	DWORD						_tlsIdx;
public:
	ObjectPool_TLS(bool bPlacementNew = false, bool sizeCheck = false);
	virtual	~ObjectPool_TLS();
	DATA *Alloc(void);
	void	Free(DATA *pData);
	int		GetCapacity(void);
	DWORD	GetSize(void);
	void	OnOffCounting() { _bSizeCheck = !_bSizeCheck; }

	CObjectPool<CChunk> *_pObjectPool;


};

template<typename DATA>
inline ObjectPool_TLS<DATA>::ObjectPool_TLS(bool bPlacementNew, bool sizeCheck) {
	_Size = 0;
	_bSizeCheck = sizeCheck;
	_bPlacementNew = bPlacementNew;
	//_pObjectPool = new CObjectPool<CChunk>(0, bPlacementNew);
	_pObjectPool = (CObjectPool<CChunk>*)_aligned_malloc(sizeof(CObjectPool<CChunk>), 64);
	new (_pObjectPool) CObjectPool<CChunk>;
	_tlsIdx = TlsAlloc();
}
template<typename DATA>
inline ObjectPool_TLS<DATA>::~ObjectPool_TLS() {
	if (_pObjectPool != nullptr) {
		_aligned_free(_pObjectPool);
	}
}
template<typename DATA>
inline DATA *ObjectPool_TLS<DATA>::Alloc(void) {
	CChunk *chunk = (CChunk *) TlsGetValue(_tlsIdx);
	DWORD myID = GetCurrentThreadId();

	if (chunk == nullptr || chunk->_AllocCount == CChunk::MAX_SIZE || chunk->_threadID != myID)
		//if (chunk == nullptr || chunk->_threadID != myID || chunk->_AllocCount == CChunk::MAX_SIZE)
	{
		chunk = _pObjectPool->Alloc();
		//packetLog(30040, GetCurrentthreadID(), chunk, nullptr, chunk->_AllocCount, chunk->_FreeCount);
		chunk->_threadID = myID;
		chunk->_pObjPool = this;
		chunk->_AllocCount = 0;
		chunk->_FreeCount = 0;
		TlsSetValue(_tlsIdx, chunk);


	}

	if (_bSizeCheck) {
		InterlockedIncrement((LONG *) &_Size);
	}
	DATA *ret = chunk->Alloc();
	if (_bPlacementNew) {
		new (ret) DATA();
	}
	return ret;
}
template<typename DATA>
inline void ObjectPool_TLS<DATA>::Free(DATA *pData) {
	if (_bPlacementNew) {
		pData->~DATA();
	}

	st_Element *block = (st_Element *) pData;

	block->pOrigin->Free(pData);

	if (_bSizeCheck) {
		InterlockedDecrement((LONG *) &_Size);
	}


}
template<typename DATA>
inline int ObjectPool_TLS<DATA>::GetCapacity(void) {
	return _pObjectPool->GetCapacity() * CChunk::MAX_SIZE;
}
template<typename DATA>
inline DWORD ObjectPool_TLS<DATA>::GetSize(void) {
	if (_bSizeCheck) {
		return _Size;
	}

	return _pObjectPool->GetSize();
}
template<typename DATA>
inline DATA *ObjectPool_TLS<DATA>::CChunk::Alloc(void) {
	st_Element *pBlock = (st_Element *) &_ObjectArr[_AllocCount++];

	//packetLog(20000, GetCurrent_threadID(), this, pBlock, _AllocCount, _FreeCount);
	pBlock->pOrigin = this;
	pBlock->code = _pObjPool;
	//pBlock->checkSum_over = CHUNK_CHECKSUM;

	return (DATA *) pBlock;
}
template<typename DATA>
inline bool ObjectPool_TLS<DATA>::CChunk::Free(DATA *pData) {
	st_Element *block = (st_Element *) pData;

	if (block->code != _pObjPool) {
		CRASH();
		return false;
	}

	if (InterlockedIncrement(&_FreeCount) == CChunk::MAX_SIZE) {
		//packetLog(40040, GetCurrent_threadID(), this, block, _AllocCount, ret);
		//block->pOrigin->_threadID = 0;
		_pObjPool->_pObjectPool->Free(block->pOrigin);
	}

	return true;
}



template <typename DATA>
class LF_ObjectPool_TLS {
	friend class CChunk;
private:
	class CChunk;
	struct st_Element {
		DATA			data;
		CChunk *pOrigin;
		void *code;
	};

	class CChunk {
	public:
		DATA *Alloc(void);
		bool	Free(DATA *pData);

		enum {
			MAX_SIZE = 1000
		};

		st_Element					_ObjectArr[MAX_SIZE];
		int							_AllocCount = 0;
		DWORD						_threadID;
		LF_ObjectPool_TLS *_pObjPool = nullptr;
		alignas(64) LONG			_FreeCount;
	};
private:
	alignas(64) DWORD			_Size;
	bool						_bSizeCheck;
	DWORD						_tlsIdx;
public:
	LF_ObjectPool_TLS(bool bPlacementNew = false, bool sizeCheck = false);
	virtual	~LF_ObjectPool_TLS();
	DATA *Alloc(void);
	void	Free(DATA *pData);
	int		GetCapacity(void);
	DWORD	GetSize(void);
	void	OnOffCounting() { _bSizeCheck = !_bSizeCheck; }

	CLF_ObjectPool<CChunk> *_pObjectPool;


};

template<typename DATA>
inline LF_ObjectPool_TLS<DATA>::LF_ObjectPool_TLS(bool bPlacementNew, bool sizeCheck) {
	_Size = 0;
	_bSizeCheck = sizeCheck;
	//_pObjectPool = new CObjectPool<CChunk>(0, bPlacementNew);
	_pObjectPool = (CLF_ObjectPool<CChunk>*)_aligned_malloc(sizeof(CLF_ObjectPool<CChunk>), 64);
	new (_pObjectPool) CLF_ObjectPool<CChunk>;
	_tlsIdx = TlsAlloc();
}
template<typename DATA>
inline LF_ObjectPool_TLS<DATA>::~LF_ObjectPool_TLS() {
	if (_pObjectPool != nullptr) {
		_aligned_free(_pObjectPool);
	}
}
template<typename DATA>
inline DATA *LF_ObjectPool_TLS<DATA>::Alloc(void) {
	CChunk *chunk = (CChunk *) TlsGetValue(_tlsIdx);
	DWORD myID = GetCurrentThreadId();

	if (chunk == nullptr || chunk->_AllocCount == CChunk::MAX_SIZE || chunk->_threadID != myID)
		//if (chunk == nullptr || chunk->_threadID != myID || chunk->_AllocCount == CChunk::MAX_SIZE)
	{
		chunk = _pObjectPool->Alloc();
		//packetLog(30040, GetCurrentthreadID(), chunk, nullptr, chunk->_AllocCount, chunk->_FreeCount);
		chunk->_threadID = myID;
		chunk->_pObjPool = this;
		chunk->_AllocCount = 0;
		chunk->_FreeCount = 0;
		TlsSetValue(_tlsIdx, chunk);


	}

	if (_bSizeCheck) {
		InterlockedIncrement((LONG *) &_Size);
	}

	return chunk->Alloc();
}
template<typename DATA>
inline void LF_ObjectPool_TLS<DATA>::Free(DATA *pData) {
	st_Element *block = (st_Element *) pData;

	block->pOrigin->Free(pData);

	if (_bSizeCheck) {
		InterlockedDecrement((LONG *) &_Size);
	}

}
template<typename DATA>
inline int LF_ObjectPool_TLS<DATA>::GetCapacity(void) {
	return _pObjectPool->GetCapacity() * CChunk::MAX_SIZE;
}
template<typename DATA>
inline DWORD LF_ObjectPool_TLS<DATA>::GetSize(void) {
	if (_bSizeCheck) {
		return _Size;
	}

	return _pObjectPool->GetSize();
}
template<typename DATA>
inline DATA *LF_ObjectPool_TLS<DATA>::CChunk::Alloc(void) {
	st_Element *pBlock = (st_Element *) &_ObjectArr[_AllocCount++];

	//packetLog(20000, GetCurrent_threadID(), this, pBlock, _AllocCount, _FreeCount);
	pBlock->pOrigin = this;
	pBlock->code = _pObjPool;
	//pBlock->checkSum_over = CHUNK_CHECKSUM;

	return (DATA *) pBlock;
}
template<typename DATA>
inline bool LF_ObjectPool_TLS<DATA>::CChunk::Free(DATA *pData) {
	st_Element *block = (st_Element *) pData;

	if (block->code != _pObjPool) {
		CRASH();
		return false;
	}

	if (InterlockedIncrement(&_FreeCount) == CChunk::MAX_SIZE) {
		//packetLog(40040, GetCurrent_threadID(), this, block, _AllocCount, ret);
		//block->pOrigin->_threadID = 0;
		_pObjPool->_pObjectPool->Free(block->pOrigin);
	}

	return true;
}