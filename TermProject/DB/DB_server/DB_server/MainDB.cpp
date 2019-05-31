#include "stdafx.h"
#include "MainDB.h"

#include "../../../2019_TT/2019_TT/protocol.h"

using namespace std;

MainDB::MainDB()
	:henv{nullptr},
	hdbc{nullptr},
	hstmt{0},
	retcode{0},
	userID{0},
	cbUser_ID{0}
{

}

MainDB::~MainDB()
{

}
void MainDB::Show_error()
{
	printf("error\n");
}

void MainDB::HandleDiagnosticRecord (SQLHANDLE hHandle, SQLSMALLINT hType, RETCODE RetCode) 
{
	SQLSMALLINT iRec = 0; SQLINTEGER  iError; WCHAR       wszMessage[1000]; WCHAR       wszState[SQL_SQLSTATE_SIZE+1];

	if (RetCode == SQL_INVALID_HANDLE) 
	{
		fwprintf(stderr, L"Invalid handle!\n");
		return; 
	} 
	while (SQLGetDiagRec(hType, hHandle, ++iRec, wszState, &iError, wszMessage, (SQLSMALLINT)(sizeof(wszMessage) / sizeof(WCHAR)), (SQLSMALLINT *)NULL) == SQL_SUCCESS) 
	{ 
		// Hide data truncated.. 
		if (wcsncmp(wszState, L"01004", 5))
		{ 
			fwprintf(stderr, L"[%5.5s] %s (%d)\n", wszState, wszMessage, iError);
		}
	}
}
bool MainDB::Initialize()
{
	bool ret = false;
	setlocale(LC_ALL, "korean");
	

	retcode = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv);

	if(retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
	{
		retcode = SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER*)SQL_OV_ODBC3, SQL_IS_INTEGER);
		if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
			ret = true;
		else
			ret = false;
	}
	return ret;
}

bool MainDB::ConnectID(const wstring& wstr)
{
	bool ret = false;
	retcode = SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc);
	if(retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
	{
		SQLSetConnectAttr(hdbc, SQL_LOGIN_TIMEOUT, (SQLPOINTER)5, 0);
		retcode = SQLConnect(hdbc, (SQLWCHAR*)L"server_DB", SQL_NTS, (SQLWCHAR*)L"smj1005", SQL_NTS, (SQLWCHAR*)L"1234",SQL_NTS);

		if(retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
		{
			retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);

			printf("DB Access ok!!\n");

			wstring exec(L"EXEC  connect_id @ID =");
			wstring res = exec + wstr;
			//retcode = SQLExecDirect(hstmt, (SQLWCHAR*)L"SELECT [ID] FROM dbo.ID ", SQL_NTS);
			//retcode = SQLExecDirect(hstmt, (SQLWCHAR*)L"SELECT * FROM dbo.ID  WHERE ID = 32152", SQL_NTS);
			retcode = SQLExecDirect(hstmt, (SQLWCHAR *)res.c_str(), SQL_NTS);           
			  	
			if(retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
			{
				retcode = SQLBindCol(hstmt, 1, SQL_C_LONG, &userID, 100, &cbUser_ID);
			//	retcode = SQLBindCol(hstmt, 2, SQL_C_CHAR, szName, NAME_LEN, &cbName);  
              //  retcode = SQLBindCol(hstmt, 3, SQL_C_LONG, &szLevel, 100 , &cbLevel);   
  
				// Fetch and print each row of data. On an error, display a message and exit.  
                for (int i=0 ; ; i++) {  
                   retcode = SQLFetch(hstmt);  
                   if (retcode == SQL_ERROR || retcode == SQL_SUCCESS_WITH_INFO)  
                      Show_error();  
                   if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
				   {
					   ret = true;
					   break;
				   }
                      //wprintf(L"%d: %d %S %d\n", i + 1, userID, szName, szLevel);  
                   else  
				   {
					   ret = false;
					   break;
				   }
                      /*break;*/  
                }  
			}
			else
			{
				 HandleDiagnosticRecord(hstmt, SQL_HANDLE_STMT, retcode);
			 }

			// Process data  
            if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) 
			{  
               SQLCancel(hstmt);  
               SQLFreeHandle(SQL_HANDLE_STMT, hstmt);  
            }  
			 SQLDisconnect(hdbc);  
		}
		 SQLFreeHandle(SQL_HANDLE_DBC, hdbc);  
	}
	 SQLFreeHandle(SQL_HANDLE_ENV, henv);  

	 return ret;
}
bool MainDB::ConnectIDandGetPos(const std::wstring& wstr)
{
	bool ret = false;
	
	//Allocate environment handle
	retcode = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv);

	
   // Set the ODBC version environment attribute
	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
	{
		retcode = SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER*)SQL_OV_ODBC3, SQL_IS_INTEGER);
		 // Allocate connection handle  
		if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
		{
			retcode = SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc);
			 
			// Set login timeout to 5 seconds  
			if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
			{
				SQLSetConnectAttr(hdbc, SQL_LOGIN_TIMEOUT, (SQLPOINTER)5, 0);
				
				// Connect to data source 
				retcode = SQLConnect(hdbc, (SQLWCHAR*)L"server_DB", SQL_NTS, (SQLWCHAR*)L"smj1005", SQL_NTS, (SQLWCHAR*)L"1234",SQL_NTS);

				 // Allocate statement handle  
				if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
				{
					retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);

					printf("DB Access ok!!\n");

					
					wstring exec(L"EXEC [dbo].[GetPos] @ID =");
					wstring res = exec + wstr;

					retcode = SQLExecDirect(hstmt, (SQLWCHAR *)res.c_str(), SQL_NTS);     
					if(retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
					{
						retcode = SQLBindCol(hstmt, 1, SQL_C_LONG, &userID, 100, &cbUser_ID);
						retcode = SQLBindCol(hstmt, 2, SQL_C_LONG, &posX, 100, &cbPosX);
						retcode = SQLBindCol(hstmt, 3, SQL_C_LONG, &posY, 100, &cbPosY);
						//	retcode = SQLBindCol(hstmt, 2, SQL_C_CHAR, szName, NAME_LEN, &cbName);  
				      //  retcode = SQLBindCol(hstmt, 3, SQL_C_LONG, &szLevel, 100 , &cbLevel);   
  
						// Fetch and print each row of data. On an error, display a message and exit.  
				    for(int i=0 ; ; i++)
					{  
						 retcode = SQLFetch(hstmt);  
						 if (retcode == SQL_ERROR || retcode == SQL_SUCCESS_WITH_INFO)  
					     Show_error();  
					     if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
						 {
							 ret = true;
							 wprintf(L"%d: %d %d %d\n", i + 1, userID, posX, posY);  
							   break;
						 }
						 else  
						 {
							  ret = false;
							   break;
						 }
                      /*break;*/  
					  }  
					}
					else
					{
						 HandleDiagnosticRecord(hstmt, SQL_HANDLE_STMT, retcode);
					 }

					// Process data  
					if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
					{
						SQLCancel(hstmt);
						SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
					}
					SQLDisconnect(hdbc);
				}
				SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
			}
		}
		SQLFreeHandle(SQL_HANDLE_ENV, henv);

	}
	 return ret;
	
}
bool MainDB::SaveUserPos(const std::wstring& wstr, CLIENTS_INFO& client)
{


	bool ret = false;
	
	//Allocate environment handle
	retcode = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv);

	
   // Set the ODBC version environment attribute
	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
	{
		retcode = SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER*)SQL_OV_ODBC3, SQL_IS_INTEGER);
		 // Allocate connection handle  
		if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
		{
			retcode = SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc);
			 
			// Set login timeout to 5 seconds  
			if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
			{
				SQLSetConnectAttr(hdbc, SQL_LOGIN_TIMEOUT, (SQLPOINTER)5, 0);
				
				// Connect to data source 
				retcode = SQLConnect(hdbc, (SQLWCHAR*)L"server_DB", SQL_NTS, (SQLWCHAR*)L"smj1005", SQL_NTS, (SQLWCHAR*)L"1234",SQL_NTS);

				 // Allocate statement handle  
				if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
				{
					retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);

					printf("DB Access ok!!\n");

					wstring exec(L"EXEC SavePos @ID= ");

					wstring res = exec + wstr;
					wstring w_posX = to_wstring(client.pos_x);
					wstring w_posY = to_wstring(client.pos_y);
					wstring sp = res + L",@PosX= " + w_posX + L",@PosY= " + w_posY;
					//retcode = SQLExecDirect(hstmt, (SQLWCHAR*)L"SELECT [ID] FROM dbo.ID ", SQL_NTS);
					//retcode = SQLExecDirect(hstmt, (SQLWCHAR*)L"SELECT * FROM dbo.ID  WHERE ID = 32152", SQL_NTS);
					
					//wstring s = L"UPDATE dbo.Table_ID_Pos SET " + w_posX + L" = @PosX ," + w_posY + L"= @PosY WHERE " + wstr + L"= @ID";
					//retcode = SQLExecDirect(hstmt, (SQLWCHAR*)s.c_str(), SQL_NTS);
					retcode = SQLExecDirect(hstmt, (SQLWCHAR *)sp.c_str(), SQL_NTS);

					if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
					{
						//retcode = SQLBindCol(hstmt, 1, SQL_C_LONG, &userID, 100, &cbUser_ID);
						//retcode = SQLBindCol(hstmt, 2, SQL_C_LONG, &posX, 100, &cbPosX);
						//retcode = SQLBindCol(hstmt, 3, SQL_C_LONG, &posY, 100, &cbPosY);

						if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
						{
							ret = true;
						}

					}
					else
					{
						//ret = false;
						HandleDiagnosticRecord(hstmt, SQL_HANDLE_STMT, retcode);
					}

					// Process data  
					if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
					{
						SQLCancel(hstmt);
						SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
					}
					SQLDisconnect(hdbc);
				}
				SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
			}
		}
		SQLFreeHandle(SQL_HANDLE_ENV, henv);

	}
	 return ret;
}
//void MainDB::Connect_DB()
//{
//	retcode = SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc);
//	if(retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
//	{
//		SQLSetConnectAttr(hdbc, SQL_LOGIN_TIMEOUT, (SQLPOINTER)5, 0);
//		retcode = SQLConnect(hdbc, (SQLWCHAR*)L"server_DB", SQL_NTS, (SQLWCHAR*)L"myoungjin", SQL_NTS, (SQLWCHAR*)L"1234",SQL_NTS);
//
//		if(retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
//		{
//			retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
//
//			printf("DB Access ok!!\n");
//
//			//retcode = SQLExecDirect(hstmt, (SQLWCHAR*)L"SELECT [ID] FROM dbo.ID ", SQL_NTS);
//			retcode = SQLExecDirect(hstmt, (SQLWCHAR*)L"SELECT * FROM dbo.ID  WHERE ID = 32152", SQL_NTS);
//		
//			if(retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
//			{
//				retcode = SQLBindCol(hstmt, 1, SQL_C_LONG, &userID, 100, &cbUser_ID);
//				//retcode = SQLBindCol(hstmt, 2, SQL_C_CHAR, szName, NAME_LEN, &cbName);  
//               // retcode = SQLBindCol(hstmt, 3, SQL_C_LONG, &szLevel, 100 , &cbLevel);   
//  
//				// Fetch and print each row of data. On an error, display a message and exit.  
//                for (int i=0 ; ; i++) {  
//                   retcode = SQLFetch(hstmt);  
//                   if (retcode == SQL_ERROR || retcode == SQL_SUCCESS_WITH_INFO)  
//                      Show_error();  
//                   if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)  
//                    //  wprintf(L"%d: %d %S %d\n", i + 1, userID, szName, szLevel);  
//                   else  
//                      break;  
//                }  
//			}
//			else
//			{
//				 HandleDiagnosticRecord(hstmt, SQL_HANDLE_STMT, retcode);
//			 }
//
//			// Process data  
//            if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) 
//			{  
//               SQLCancel(hstmt);  
//               SQLFreeHandle(SQL_HANDLE_STMT, hstmt);  
//            }  
//			 SQLDisconnect(hdbc);  
//		}
//		 SQLFreeHandle(SQL_HANDLE_DBC, hdbc);  
//	}
//	 SQLFreeHandle(SQL_HANDLE_ENV, henv);  
//	 system("pause");
//}
