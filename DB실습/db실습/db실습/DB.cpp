//// SQLConnect_ref.cpp  
//// compile with: odbc32.lib  
//#include <windows.h>  
//#include <sqlext.h>  
//#include <iostream>
//  
//
//
//
//int main() {  
//   SQLHENV henv;  
//   SQLHDBC hdbc;  
//   SQLHSTMT hstmt;  
//   SQLRETURN retcode;  
//  
//   SQLCHAR * OutConnStr = (SQLCHAR * )malloc(255);  
//   SQLSMALLINT * OutConnStrLen = (SQLSMALLINT *)malloc(255);  
//  
//   // Allocate environment handle  
//   retcode = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv);  
//  
//   // Set the ODBC version environment attribute  
//   if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {  
//      retcode = SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (void*)SQL_OV_ODBC3, 0);   
//  
//      // Allocate connection handle  
//      if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {  
//         retcode = SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc);   
//  
//         // Set login timeout to 5 seconds  
//         if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {  
//            SQLSetConnectAttr(hdbc, SQL_LOGIN_TIMEOUT, (SQLPOINTER)5, 0);  
//  
//            // Connect to data source  
//            retcode = SQLConnect(hdbc, (SQLWCHAR*) L"NorthWind", SQL_NTS, (SQLWCHAR*) NULL, 0, NULL, 0);  
//  
//            // Allocate statement handle  
//            if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {  
//               retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);   
//  
//			   std::cout << "dbconnect ¼º°ø" << std::endl;
//               // Process data  
//               if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {  
//                  SQLFreeHandle(SQL_HANDLE_STMT, hstmt);  
//               }  
//  
//               SQLDisconnect(hdbc);  
//            }  
//  
//            SQLFreeHandle(SQL_HANDLE_DBC, hdbc);  
//         }  
//      }  
//      SQLFreeHandle(SQL_HANDLE_ENV, henv);  
//   }  
//   system("pause");
//}  