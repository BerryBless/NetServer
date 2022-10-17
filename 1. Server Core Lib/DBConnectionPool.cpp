#include "pch.h"
#include "DBConnectionPool.h"

DBConnectionPool::DBConnectionPool() {
	InitializeSRWLock(&_lock);
}

DBConnectionPool::~DBConnectionPool() {
	Clear();
}

bool DBConnectionPool::Connect(INT connectionCount, const WCHAR *connectionString) {
	AcquireSRWLockExclusive(&_lock);

	if (::SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &_environment) != SQL_SUCCESS) {
		ReleaseSRWLockExclusive(&_lock);
		return false;
	}
	if (::SQLSetEnvAttr(_environment, SQL_ATTR_ODBC_VERSION, reinterpret_cast<SQLPOINTER>(SQL_OV_ODBC3), 0) != SQL_SUCCESS) {
		ReleaseSRWLockExclusive(&_lock);
		return false;
	}


	for (INT i = 0; i < connectionCount; i++) {
		DBConnection *pConnection = new DBConnection();
		if (pConnection->Connect(_environment, connectionString) == false) {
			ReleaseSRWLockExclusive(&_lock);
			return false;
		}
		_connections.push(pConnection);
	}
	ReleaseSRWLockExclusive(&_lock);
	return true;
}

void DBConnectionPool::Clear() {
	AcquireSRWLockExclusive(&_lock);
	if (_environment != SQL_NULL_HANDLE) {
		::SQLFreeHandle(SQL_HANDLE_ENV, _environment);
		_environment = SQL_NULL_HANDLE;
	}
	DBConnection *pConnection = nullptr;
	while (_connections.pop(pConnection)) {
		delete pConnection;
		pConnection = nullptr;
	}

	ReleaseSRWLockExclusive(&_lock);
}

DBConnection *DBConnectionPool::Pop() {
	DBConnection *pConnection = nullptr;
	AcquireSRWLockExclusive(&_lock);
	_connections.pop(pConnection);
	ReleaseSRWLockExclusive(&_lock);
	return pConnection;
}

void DBConnectionPool::Push(DBConnection *connection) {
	AcquireSRWLockExclusive(&_lock);
	_connections.push(connection);
	ReleaseSRWLockExclusive(&_lock);
}
