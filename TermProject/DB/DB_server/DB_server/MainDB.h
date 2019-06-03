#pragma once
#include <sqlext.h>

typedef struct Client_Info
{
	Client_Info(int x,int y,int h)
		: pos_x(x),pos_y(y),hp(h)
	{
	}
	Client_Info(){}
	int	pos_x;
	int pos_y;
	int hp;
}CLIENTS_INFO;
#define NAME_LEN 50

class MainDB
{
private:
	SQLHENV henv;
	SQLHDBC hdbc;
	SQLHSTMT hstmt = 0;
	SQLRETURN retcode;
	SQLWCHAR szName[NAME_LEN];
	SQLINTEGER posX,posY, userID,hp;
	SQLLEN cbName = 0, cbPosX = 0, cbPosY = 0,cbUser_ID = 0,cbHp = 0;		//ฤÝน้

	SQLCHAR SqlState[6], SQLStmt[100], Msg[SQL_MAX_MESSAGE_LENGTH];
	SQLINTEGER NativeError;
	SQLSMALLINT Loop, MsgLen;
	SQLRETURN rc1, rc2;

public:
	MainDB();
	~MainDB();

	void Show_error();
	void HandleDiagnosticRecord(SQLHANDLE hHandle, SQLSMALLINT hType, RETCODE RetCode);
	bool Initialize();
	
	unsigned char GetPosX() { return (unsigned char)posX; }
	unsigned char GetPosY() { return (unsigned char)posY; }
	unsigned char GetHp() { return (unsigned char)hp; }

	//void Connect_DB();
	bool ConnectID(const std::wstring& wstr);
	bool ConnectIDandGetPos(const std::wstring& wstr);
	bool SaveUserPos(const std::wstring& wstr, CLIENTS_INFO& client);
	

};