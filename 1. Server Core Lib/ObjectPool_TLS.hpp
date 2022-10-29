#pragma once
#include "ObjectPool.hpp"
#include <wtypes.h>

template <typename DATA>
class ObjectPool_TLS {
private:
	constexpr static int MAX_CHUNK_SIZE = 1000;
	struct CHUNK;
	struct CHUNK_BLOCK {
		DATA	data;
		CHUNK *pOrigin;
	};
	struct CHUNK {
	public:
		DATA *Alloc(void) {
			CHUNK_BLOCK *pBlock = (CHUNK_BLOCK *) &_ObjectArr[_AllocCount];
			pBlock->pOrigin = this;
			++_AllocCount;
			return (DATA *) pBlock;
		}
		void Clear() {
			_AllocCount = MAX_CHUNK_SIZE;
			_AllocCount = 0;
			_FreeCount = 0;
		}


		DWORD					_AllocCount = 0;
		DWORD					_FreeCount = 0;
		DWORD					_CHUNKSize = 0;
		CHUNK_BLOCK				_ObjectArr[MAX_CHUNK_SIZE];
	};

public:
	ObjectPool_TLS(bool bPlacementNew = false) {
		_totalUseCount = 0;
		_bPlacementNew = bPlacementNew;
		_tlsIdx = TlsAlloc();

	}
	virtual	~ObjectPool_TLS() {
		TlsFree(_tlsIdx);
	}
	DATA *Alloc(void) {
		CHUNK *pChunk = (CHUNK *) TlsGetValue(_tlsIdx);

		if (pChunk == nullptr) {
			pChunk = ChunkAlloc();
			TlsSetValue(_tlsIdx, pChunk);
		}

		DATA *ret = pChunk->Alloc();
		if (pChunk->_AllocCount >= MAX_CHUNK_SIZE)
			TlsSetValue(_tlsIdx, ChunkAlloc());

		if (_bPlacementNew) {
			new (ret) DATA();
		}

		InterlockedIncrement64(&_totalUseCount);
		return ret;
	}
	void	Free(DATA *pData) {
		if (_bPlacementNew) {
			pData->~DATA();
		}

		CHUNK_BLOCK *pBlock = (CHUNK_BLOCK *) pData;
		CHUNK *pChunk = pBlock->pOrigin;

		if (InterlockedIncrement(&pChunk->_FreeCount) == MAX_CHUNK_SIZE)
			ChunkFree(pChunk);

		InterlockedDecrement64(&_totalUseCount);
	}

	int		GetCapacity(void) { return _objectPool.GetCapacity() * MAX_CHUNK_SIZE; }
	DWORD	GetSize(void) { return _totalUseCount;/*_objectPool->GetSize(); */ }
private:
	CHUNK *ChunkAlloc() {
		CHUNK *pChunk = _objectPool.Alloc();
		if (pChunk == nullptr) {
			return nullptr;
		}
		pChunk->Clear();

		return pChunk;
	}
	void ChunkFree(CHUNK *pChunk) {
		pChunk->Clear();
		_objectPool.Free(pChunk);
	}
private:
	CLF_ObjectPool<CHUNK> _objectPool;
	//ObjectPool<CHUNK> *_pObjectPool;
	DWORD			_tlsIdx;
	bool			_bPlacementNew;
	alignas(64) LONG64 _totalUseCount;
};
