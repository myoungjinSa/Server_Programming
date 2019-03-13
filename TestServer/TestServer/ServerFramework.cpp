#include "ServerFramework.h"



CServerFramework::CServerFramework()
	:addrlen(0),
	 m_AcceptRequest(0)
{
	int retval = 0;

	//윈속 초기화
	WSADATA wsa;

	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		return;
	}
	cout << "윈속 초기화 성공" << endl;

	//대기 소켓 생성
	m_listenSocket = socket(AF_INET, SOCK_STREAM, 0);

	if(m_listenSocket == INVALID_SOCKET)
	{
		err_quit("socket()");
		return;
	}

	SOCKADDR_IN server_addr;
	ZeroMemory(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(serverPort);

	retval = ::bind(m_listenSocket, (SOCKADDR*)&server_addr, sizeof(server_addr));
	if(retval == SOCKET_ERROR)
	{
		err_quit("listen()");
		return;
	}

	cout << "대기 소켓 생성 완료" << endl;

	m_vClientSocket.reserve(MAX_CLIENT_COUNT);
	
}

CServerFramework::~CServerFramework()
{
	Destroy();
}

void CServerFramework::err_quit(const char* msg)
{
	LPVOID lpMsgBuf;

	FormatMessage
	(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPSTR)&lpMsgBuf,
		0,
		NULL
	);

	MessageBox(NULL, (LPCTSTR)lpMsgBuf, (LPCTSTR)msg, MB_ICONERROR);
	LocalFree(lpMsgBuf);
	exit(1);
}

void CServerFramework::err_display(const char* msg)
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
	cout << msg << (char*)lpMsgBuf << endl;

	LocalFree(lpMsgBuf);
}

int CServerFramework::recvn(SOCKET s,char* buf,int len,int flags)
{
	int received;

	char *ptr = buf;
	int left = len;

	while(left > 0)
	{
		received = recv(s, ptr, left, flags);
		if(received == SOCKET_ERROR)
		{
			return SOCKET_ERROR;
		}
		else if(received == 0)
		{
			break;
		}
		left -= received;
		ptr += received;
	}

	return (len - left);
}

SOCKET& CServerFramework::AcceptClient()
{
	addrlen = sizeof(m_clientAddr);
	m_clientSocket = accept(m_listenSocket, (SOCKADDR*)&m_clientAddr, &addrlen);
	if(m_clientSocket == INVALID_SOCKET)
	{
		err_display("accept()");
	}

	cout << endl << "[ TCP 서버 ] 클라이언트 접속 - IP : " << inet_ntoa(m_clientAddr.sin_addr)
		<< ", 포트 번호 : " << ntohs(m_clientAddr.sin_port) << endl;


	if(m_vClientSocket.size() == 0 )
	{
		m_vClientSocket.emplace_back(m_clientSocket, PLAYER_1);
	}

	return m_clientSocket;
}

void CServerFramework::RecvPacket()
{
	int retval = 0;
	size_t packetSize = 0;

	while(true)
	{
		CS_RUN cs_runPacket;
		retval = recvn(m_vClientSocket[PLAYER_1].clientSocket, (char*)&cs_runPacket, sizeof(cs_runPacket), 0);
		if (retval == SOCKET_ERROR)
		{
			err_display("recvn( )");
			return;
		}
	}
}

void CServerFramework::ProcessKeyInput(byte& keyInput)
{
	switch(keyInput)
	{
	case VK_UP:

		break;
	case VK_DOWN:
		break;
	case VK_RIGHT:
		break;
	case VK_LEFT:
		break;
	default:
		break;
	}
}

void CServerFramework::Destroy()
{
	for(auto iter = m_vClientSocket.begin();iter != m_vClientSocket.end();++iter)
	{
		closesocket(iter->clientSocket);
	}
	m_vClientSocket.clear();

	closesocket(m_listenSocket);
	WSACleanup();
}