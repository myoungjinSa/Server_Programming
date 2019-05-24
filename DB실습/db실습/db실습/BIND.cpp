// SQLBindCol_ref.cpp  
// compile with: odbc32.lib  
#include <windows.h>  
#include <stdio.h>  
  
#include <iostream>
#define UNICODE  
#include <sqlext.h>  
  
#define NAME_LEN 50  
#define PHONE_LEN 20  
  
using namespace std;
void show_error() {  
   printf("error\n"); 
  
}  

void HandleDiagnosticRecord (SQLHANDLE hHandle, SQLSMALLINT hType, RETCODE RetCode) 
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
  
int main() {  
   SQLHENV henv;  
   SQLHDBC hdbc;  
   SQLHSTMT hstmt = 0;  
   SQLRETURN retcode;  
   SQLWCHAR szName[NAME_LEN];
   SQLINTEGER szLevel, userID;  
   SQLLEN cbName = 0, cbLevel = 0, cbUser_ID = 0;		//콜백
  

   setlocale(LC_ALL, "korean");
   // Allocate environment handle  
   retcode = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv);  
  
   // Set the ODBC version environment attribute  
   if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {  
      retcode = SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER*)SQL_OV_ODBC3, 0);   
  
      // Allocate connection handle  
      if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {  
         retcode = SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc);  
  
         // Set login timeout to 5 seconds  
         if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {  
            SQLSetConnectAttr(hdbc, SQL_LOGIN_TIMEOUT, (SQLPOINTER)5, 0);  
  
            // Connect to data source  
            retcode = SQLConnect(hdbc, (SQLWCHAR*) L"myGame", SQL_NTS, (SQLWCHAR*) NULL, 0, NULL, 0); // 윈도우 인증이 아닌 ID,PW를 입력해야함.	  
  
            // Allocate statement handle  
            if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {   
               retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);   
  
			   printf("DB Access ok!!\n");

			   //아래처럼 sql을 직접 쓰는것보다 Stored Procedure를 사용하는게 낫다
			   retcode = SQLExecDirect(hstmt, (SQLWCHAR *) L"EXEC get_users", SQL_NTS);  
               //retcode = SQLExecDirect(hstmt, (SQLWCHAR *) L"SELECT [ID], [Name], [Level] FROM MyGame ORDER BY 2,1,3", SQL_NTS);  
               if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {  
  
                  // Bind columns 1, 2, and 3  
                  retcode = SQLBindCol(hstmt, 1, SQL_C_LONG, &userID, 100, &cbUser_ID);  
                  retcode = SQLBindCol(hstmt, 2, SQL_C_CHAR, szName, NAME_LEN, &cbName);  
                  retcode = SQLBindCol(hstmt, 3, SQL_C_LONG, &szLevel, 100 , &cbLevel);   
  
                  // Fetch and print each row of data. On an error, display a message and exit.  
                  for (int i=0 ; ; i++) {  
                     retcode = SQLFetch(hstmt);  
                     if (retcode == SQL_ERROR || retcode == SQL_SUCCESS_WITH_INFO)  
                        show_error();  
                     if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)  
                        wprintf(L"%d: %d %S %d\n", i + 1, userID, szName, szLevel);  
                     else  
                        break;  
                  }  
               }  
			   else
			   {
				   HandleDiagnosticRecord(hstmt, SQL_HANDLE_STMT, retcode);
			   }
  
				
               // Process data  
               if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {  
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
   system("pause");
}  