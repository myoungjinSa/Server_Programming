#include "stdafx.h"
#include "Network.h"
#include <iostream>

using namespace std;

CNetwork::CNetwork()
	:m_socket(0),
	m_serverAddr(),
	m_csRunPacket(0,0)
{

}

CNetwork::~CNetwork()
{

}

void CNetwork::err_quit(const char* msg)
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

void CNetwork::err_display(const char* msg)
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


int CNetwork::recvn(char *buf,int len,int flags)
{
	int received;

	char *ptr = buf;
	int left = len;

	while(left>0)
	{
		received = recv(m_socket, ptr, left, flags);
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


void CNetwork::Initialize()
{
	int retval = 0;
	
	//윈속 초기화
	WSADATA wsa;
	if(WSAStartup(MAKEWORD(2,2),&wsa) != 0)
	{
		return;
	}

	//socket
	m_socket = WSASocket(AF_INET, SOCK_STREAM, 0,NULL,0,WSA_FLAG_OVERLAPPED);
	if (m_socket == INVALID_SOCKET)
		err_quit("socket()");

	std::string s;
	cout << "서버 IP주소를 입력해주세요(127.0.0.1)" << "\n";
	cin >> s;

	//connect
	memset(&m_serverAddr,0, sizeof(m_serverAddr));
	m_serverAddr.sin_family = AF_INET;
	m_serverAddr.sin_port = htons(SERVERPORT);
	m_serverAddr.sin_addr.s_addr = inet_addr(s.c_str());

	retval = connect(m_socket, (SOCKADDR*)&m_serverAddr, sizeof(m_serverAddr));
	if (retval == SOCKET_ERROR) {
		
		closesocket(m_socket);

		WSACleanup();
		err_quit("connect()");
		return ;
	}
	else
	{
		
	}

	
}

void CNetwork::Destroy()
{
	closesocket(m_socket);

	WSACleanup();
}

void CNetwork::SendPacket()
{
	int retval = 0;

	SOCKETINFO* socketInfo;

	socketInfo = (struct SOCKETINFO*)malloc(sizeof(struct SOCKETINFO));
	memset((void*)socketInfo, 0x00, sizeof(struct SOCKETINFO));
	socketInfo->dataBuffer.len = sizeof(CS_RUN);
	socketInfo->dataBuffer.buf = (char*)&m_csRunPacket.key;

	
	retval = send(m_socket, socketInfo->dataBuffer.buf, socketInfo->dataBuffer.len, 0);

	if (retval == SOCKET_ERROR)
	{
		err_display("send()");
		return;
	}
	m_csRunPacket.key = KEY_IDLE;
		
	
	
	
}

void CNetwork::RecvPacket()
{
	int retval = 0;

	
	retval = recvn((char*)&m_scRunPacket.player, sizeof(SC_RUN), 0);
	if (retval == SOCKET_ERROR)
	{
		err_display("recvn( )");
		return;
	}
	
	m_csRunPacket.player = m_scRunPacket.index;
	
	m_clientCount = (int)m_scRunPacket.playerCount;

	//cout <<"클라이언트 접속 수: "<<m_clientCount << endl;

}