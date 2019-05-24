#pragma once
#include <sqlext.h>

#define NAME_LEN 50
class MainDB
{
private:
	SQLHENV henv;
	SQLHDBC hdbc;
	SQLHSTMT hstmt = 0;
	SQLRETURN retcode;
	SQLWCHAR szName[NAME_LEN];
	SQLINTEGER szLevel, userID;
	SQLLEN cbName = 0, cbLevel = 0, cbUser_ID = 0;		//ฤÝน้

public:
	MainDB() {}
	~MainDB(){}

	void Show_error();
	void HandleDiagnosticRecord(SQLHANDLE hHandle, SQLSMALLINT hType, RETCODE RetCode);
	bool Initialize();
	void Connect_DB();
	

};