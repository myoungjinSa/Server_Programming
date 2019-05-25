
#include "stdafx.h"
#include "MainDB.h"
//#include "Network.h"
#include "../../../2019_TT/2019_TT/protocol.h"
#include <vector>
#include <map>
#include <thread>

using namespace std;


vector<int> connect_id_list;
#define MAX_BUFFER        1024



SOCKET g_MainServerSocket;
struct OVER_EX
{
	WSAOVERLAPPED	overlapped;
	WSABUF			dataBuffer;
	char			messageBuffer[MAX_BUFFER];
	QUERY_TYPE		query;
	//bool			is_recv;
};


struct SOCKETINFO
{
	OVER_EX			over;
	SOCKET        socket;
	int				receiveBytes;
	int				sendBytes;

	sd_packet_connect* sd_connect;
	MainDB*			db_pointer;
};

map<SOCKET, SOCKETINFO> clients;

void CALLBACK recv_callback(DWORD Error, DWORD dataBytes, LPWSAOVERLAPPED overlapped, DWORD lnFlags);
void CALLBACK send_callback(DWORD Error, DWORD dataBytes, LPWSAOVERLAPPED overlapped, DWORD lnFlags);
void ConnectToServer();
sd_packet_connect *pSDC = NULL;
ds_packet_connect_result *pDSCR = NULL;

MainDB main_db;
void initialize()
{
	connect_id_list.clear();
}
void err_quit(const char* msg)
{
	LPVOID lpMsgBuf;

	FormatMessage
	(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,
		0,
		NULL
	);

	MessageBox(NULL, (LPCTSTR)lpMsgBuf, (LPCTSTR)msg, MB_ICONERROR);
	LocalFree(lpMsgBuf);
	exit(1);
}

void err_display(const char* msg)
{
	LPVOID lpMsgBuf;
	FormatMessage
	(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,
		0,
		NULL
	);
	//cout << msg << (char*)lpMsgBuf << endl;

	LocalFree(lpMsgBuf);
}




int main()
{

	
	main_db.Initialize();
	WSADATA WSAData;

	if(WSAStartup(MAKEWORD(2,2),&WSAData) != 0)
	{
		cout << "Error - Can not load 'winsock.dll' file\n";
		return 1;
	}

	SOCKET listenSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if(listenSocket == INVALID_SOCKET)
	{
		printf("Error - Invalid socket\n");
		return 1;
	}

	//서버 정보 객체 설정
	SOCKADDR_IN serverAddr;
	memset(&serverAddr, 0, sizeof(SOCKADDR_IN));
	serverAddr.sin_family = PF_INET;
	serverAddr.sin_port = htons(DB_PORT);
	serverAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

	if (::bind(listenSocket, (struct sockaddr*)&serverAddr, sizeof(SOCKADDR_IN)) == SOCKET_ERROR)
	{
		printf("Error - Fail bind\n");
		// 6. 소켓종료
		closesocket(listenSocket);
		// Winsock End
		WSACleanup();
		return 1;
	}
		// 3. 수신대기열생성
	if (listen(listenSocket, 5) == SOCKET_ERROR)
	{
		printf("Error - Fail listen\n");
		// 6. 소켓종료
		closesocket(listenSocket);
		// Winsock End
		WSACleanup();
		return 1;
	}

	SOCKADDR_IN clientAddr;
	int addrLen = sizeof(SOCKADDR_IN);
	memset(&clientAddr, 0, addrLen);
	SOCKET clientSocket;
	DWORD flags;

	while (1)
	{
		clientSocket = accept(listenSocket, (struct sockaddr *)&clientAddr, &addrLen);
		if (clientSocket == INVALID_SOCKET)
		{
			printf("Error - Accept Failure\n");
			return 1;
		}

		cout << "db접속" << endl;
		clients[clientSocket] = SOCKETINFO{};
		memset(&clients[clientSocket], 0x00, sizeof(struct SOCKETINFO));
		clients[clientSocket].socket = clientSocket;
		clients[clientSocket].sd_connect = new sd_packet_connect;
		clients[clientSocket].over.dataBuffer.len = MAX_BUFFER;
		clients[clientSocket].over.dataBuffer.buf = clients[clientSocket].over.messageBuffer;
		//clients[clientSocket].db_pointer = &main_db;
		flags = 0;

		// 중첩 소캣을 지정하고 완료시 실행될 함수를 넘겨준다.
		clients[clientSocket].over.overlapped.hEvent = (HANDLE)clients[clientSocket].socket;

		if (WSARecv(clients[clientSocket].socket, &clients[clientSocket].over.dataBuffer, 1, NULL, &flags, &(clients[clientSocket].over.overlapped), recv_callback))
		{
			if (WSAGetLastError() != WSA_IO_PENDING)
			{
				printf("Error - IO pending Failure\n");
				return 1;
			}
		}
		else {
			cout << "Non Overlapped Recv return.\n";
			return 1;
		}
	}

	delete clients[clientSocket].sd_connect;
	// 6-2. 리슨 소켓종료
	closesocket(listenSocket);

	// Winsock End
	WSACleanup();

	return 0;

}

void CALLBACK recv_callback(DWORD Error, DWORD dataBytes, LPWSAOVERLAPPED overlapped, DWORD lnFlags)
{
	SOCKET client_s = reinterpret_cast<int>(overlapped->hEvent);

	DWORD sendBytes = 0;
	DWORD receiveBytes = 0;
	DWORD flags = 0;

	if (dataBytes == 0)
	{
		closesocket(clients[client_s].socket);
		clients.erase(client_s);
		return;
	}  // 클라이언트가 closesocket을 했을 경우


	clients[client_s].over.dataBuffer.len = sizeof(sd_packet_connect);
	clients[client_s].over.dataBuffer.buf = (char*)&clients[client_s].over.messageBuffer;
	switch(clients[client_s].over.dataBuffer.buf[1])
	{
	case SD_CONNECT:
	{
		clients[client_s].sd_connect = reinterpret_cast<sd_packet_connect*>(clients[client_s].over.messageBuffer);
		cout << "CONNECT요청이 들어왔습니다.\n";
		bool ret{ false };

		wstring wid = to_wstring(clients[client_s].sd_connect->id);
		wcout << wid << endl;
		ret=main_db.ConnectID(wid);
		if (ret)
			cout << "해당 id가 테이블에 있습니다." << endl;
		else
			cout << "해당 id가 없습니다." << endl;

		ds_packet_connect_result dsr{};

		dsr.type = DS_CONNECT_RESULT;
		dsr.size = sizeof(ds_packet_connect_result);
		dsr.access = ret;

		clients[client_s].over.dataBuffer.len = dsr.size;
		clients[client_s].over.dataBuffer.buf = (char*)&dsr;
		break;
	}
	case SD_POSITION_SAVE:

		break;
	default:
		
		cout << "잘못된 쿼리 요청입니다." << endl;
	}

	//cout << "TRACE - Receive message : "
	//	<< clients[client_s].over.messageBuffer
	//	<< " (" << dataBytes << ") bytes)\n";

	memset(&(clients[client_s].over.overlapped), 0x00, sizeof(WSAOVERLAPPED));
	clients[client_s].over.overlapped.hEvent = (HANDLE)client_s;
	if (WSASend(client_s, &(clients[client_s].over.dataBuffer), 1, &dataBytes, 0, &(clients[client_s].over.overlapped), send_callback) == SOCKET_ERROR)
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			printf("Error - Fail WSASend(error_code : %d)\n", WSAGetLastError());
		}
	}

}

void CALLBACK send_callback(DWORD Error, DWORD dataBytes, LPWSAOVERLAPPED overlapped, DWORD lnFlags)
{
	DWORD sendBytes = 0;
	DWORD receiveBytes = 0;
	DWORD flags = 0;

	SOCKET client_s = reinterpret_cast<int>(overlapped->hEvent);

	if (dataBytes == 0)
	{
		closesocket(clients[client_s].socket);
		clients.erase(client_s);
		return;
	}  // 클라이언트가 closesocket을 했을 경우

	{
		// WSASend(응답에 대한)의 콜백일 경우
		clients[client_s].over.dataBuffer.len = sizeof(sd_packet_connect);
		clients[client_s].over.dataBuffer.buf = (char*)&clients[client_s].over.messageBuffer;

		/*cout << "TRACE - Send message : "
			<< clients[client_s].over.messageBuffer
			<< " (" << dataBytes << " bytes)\n";*/
		memset(&(clients[client_s].over.overlapped), 0x00, sizeof(WSAOVERLAPPED));
		clients[client_s].over.overlapped.hEvent = (HANDLE) client_s;
		if (WSARecv(client_s, &clients[client_s].over.dataBuffer, 1, &receiveBytes, &flags, &(clients[client_s].over.overlapped), recv_callback) == SOCKET_ERROR)
		{
			if (WSAGetLastError() != WSA_IO_PENDING)
			{
				printf("Error - Fail WSARecv(error_code : %d)\n", WSAGetLastError());
			}
		}
	}
}

