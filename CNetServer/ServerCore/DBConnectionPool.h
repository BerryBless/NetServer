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

/*
사용법
	// Connect DB
	bool ret = _DBPool.Connect(1, L"DRIVER={MySQL ODBC 8.0 Unicode Driver};\
				SERVER=localhost;\
				DATABASE=testserverDB;\
				USER=testuser;\
				PASSWORD=1234;\
				port=33066\
				stmt = SET NAMES 'utf8'");
	if (ret == false) {
		int *p = nullptr;
		*p = 10;
	}

	// Create Table
	{
		DBConnection *dbConn = _DBPool.Pop();
		WCHAR query[] = L"DROP TABLE IF EXISTS test";

		ASSERT_CRASH(dbConn->Execute(query));
		_DBPool.Push(dbConn);
	}
	{
		DBConnection *dbConn = _DBPool.Pop();
		WCHAR query[] = L"CREATE TABLE test(	id INT NOT NULL PRIMARY KEY AUTO_INCREMENT,	gold INT NULL);";

		ASSERT_CRASH(dbConn->Execute(query));
		_DBPool.Push(dbConn);
	}

	// Add Data
	for (INT i = 0; i < 10; i++) {
		DBConnection *dbConn = _DBPool.Pop();
		// 기존에 바인딩 된 정보 날림
		dbConn->Unbind();

		// 넘길 인자 바인딩
		INT gold = 100;
		SQLLEN len = 0;

		// 넘길 인자 바인딩
		ASSERT_CRASH(dbConn->BindParam(1, SQL_C_LONG, SQL_INTEGER, sizeof(gold), &gold, &len));

		// SQL 실행
		ASSERT_CRASH(dbConn->Execute(L"INSERT INTO test(gold) VALUES(?)"));

		_DBPool.Push(dbConn);
	}
	// Read
	{
		DBConnection *dbConn = _DBPool.Pop();
		// 기존에 바인딩 된 정보 날림
		dbConn->Unbind();

		INT gold = 100;
		SQLLEN len = 0;
		// 넘길 인자 바인딩
		ASSERT_CRASH(dbConn->BindParam(1, SQL_C_LONG, SQL_INTEGER, sizeof(gold), &gold, &len));

		// 받을 인자 바인딩
		INT outId = 0;
		SQLLEN outIdLen = 0;
		ASSERT_CRASH(dbConn->BindCol(1, SQL_C_LONG, sizeof(outId), &outId, &outIdLen));

		INT outGold = 0;
		SQLLEN outGoldLen = 0;
		ASSERT_CRASH(dbConn->BindCol(2, SQL_C_LONG, sizeof(outGold), &outGold, &outGoldLen));

		// SQL 실행
		ASSERT_CRASH(dbConn->Execute(L"SELECT id, gold FROM test WHERE gold = (?)"));

		while (dbConn->Fetch()) {
			cout << "Id: " << outId << " Gold : " << outGold << endl;
		}

		_DBPool.Push(dbConn);
	}
*/