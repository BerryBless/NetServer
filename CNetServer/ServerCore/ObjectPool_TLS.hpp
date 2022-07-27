#pragma once
#include "ObjectPool.hpp"
#include <wtypes.h>

#define CHUNK_CHECKSUM 0xBBBBBBBB


template <typename DATA>
class ObjectPool_TLS {
	friend class CChunk;
private:
	struct st_Chunk_Block;
	struct st_FreeCount;

	class CChunk {
	public:
		DATA *Alloc(void);
		bool	Free(DATA *pData);

		enum {
			MAX_SIZE = 500
		};

		st_Chunk_Block				_ObjectArr[MAX_SIZE];
		int							_AllocCount = 0;
		DWORD						_threadID;
		ObjectPool_TLS *			_pObjPool = nullptr;
		alignas(64) st_FreeCount	_FreeCount;
	};

	struct st_FreeCount {
		union {
			LONG counter = 0;
			struct {
				SHORT freeCount;
				SHORT isFreed;
			} freeStatus;
		};
	};

	struct st_Chunk_Block {
		DATA			data;
		void *			code;
		CChunk *		pOrigin;
		unsigned int	checkSum_over = CHUNK_CHECKSUM;
	};

public:
	ObjectPool_TLS(bool bPlacementNew = false, bool sizeCheck = false);
	virtual	~ObjectPool_TLS();
	DATA *Alloc(void);
	void	Free(DATA *pData);
	int		GetCapacity(void);
	DWORD	GetSize(void);
private:
	ObjectPool<CChunk> *_pObjectPool;

	DWORD			_Size;
	DWORD			_tlsIdx;
	bool			_bPlacementNew;

};

template<typename DATA>
inline ObjectPool_TLS<DATA>::ObjectPool_TLS(bool bPlacementNew, bool sizeCheck) {
	_Size = 0;
	_bPlacementNew = bPlacementNew;
	//_pObjectPool = new ObjectPool<CChunk>(0, bPlacementNew);
	_pObjectPool = (ObjectPool<CChunk>*)_aligned_malloc(sizeof(ObjectPool<CChunk>), 64);
	new (_pObjectPool) ObjectPool<CChunk>;
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
	if (chunk == nullptr || chunk->_threadID != GetCurrentThreadId() || chunk->_AllocCount == CChunk::MAX_SIZE)
	{
		chunk = _pObjectPool->Alloc();
		chunk->_threadID = GetCurrentThreadId();
		chunk->_pObjPool = this;
		chunk->_AllocCount = 0;
		chunk->_FreeCount.freeStatus.freeCount = 0;
		chunk->_FreeCount.freeStatus.isFreed = 0;
		TlsSetValue(_tlsIdx, chunk);
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

	st_Chunk_Block *block = (st_Chunk_Block *) pData;

	block->pOrigin->Free(pData);

}
template<typename DATA>
inline int ObjectPool_TLS<DATA>::GetCapacity(void) {
	return _pObjectPool->GetCapacity() * CChunk::MAX_SIZE;
}
template<typename DATA>
inline DWORD ObjectPool_TLS<DATA>::GetSize(void) {
	return _pObjectPool->GetSize();
}
template<typename DATA>
inline DATA *ObjectPool_TLS<DATA>::CChunk::Alloc(void) {
	if (_AllocCount == CChunk::MAX_SIZE) return nullptr;
	st_Chunk_Block *pBlock = (st_Chunk_Block *) &_ObjectArr[_AllocCount++];

	pBlock->pOrigin = this;
	pBlock->code = this;
	pBlock->checkSum_over = CHUNK_CHECKSUM;

	return (DATA *) pBlock;
}
template<typename DATA>
inline bool ObjectPool_TLS<DATA>::CChunk::Free(DATA *pData) {
	st_Chunk_Block *block = (st_Chunk_Block *) pData;
	st_FreeCount freeCounter;

	if (block->code != this ||
		block->checkSum_over != CHUNK_CHECKSUM) {
		CRASH();
		return false;
	}

	InterlockedIncrement((LONG *) &_FreeCount);

	freeCounter.counter = CChunk::MAX_SIZE;
	freeCounter.freeStatus.isFreed = 1;
	if (InterlockedCompareExchange(&_FreeCount.counter, freeCounter.counter, CChunk::MAX_SIZE) == CChunk::MAX_SIZE) {
		block->pOrigin->_threadID = 0;
		_pObjPool->_pObjectPool->Free(block->pOrigin);
	}

	return true;
}

