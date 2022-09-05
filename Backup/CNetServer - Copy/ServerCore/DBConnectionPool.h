#pragma once
#include "DBConnection.h"
class DBConnectionPool {
public:
	DBConnectionPool();
	~DBConnectionPool();

	bool					Connect(INT connectionCount, const WCHAR *connectionString);
	void					Clear();

	DBConnection			*Pop();
	void					Push(DBConnection *connection);

private:
	SRWLOCK _lock;
	SQLHENV					_environment = SQL_NULL_HANDLE;
	Stack<DBConnection *>	_connections;
};

