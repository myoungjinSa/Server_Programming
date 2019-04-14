#include "stdafx.h"
#include "GameFramework.h"
#include "Network.h"
#include <iostream>

using namespace std;

CNetwork::CNetwork()
	:m_socket(0),
	m_serverAddr()

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


void CNetwork::Initialize(HWND window,CGameFramework* client)
{
	int retval = 0;
	
	//윈속 초기화
	WSADATA wsa;
	if(WSAStartup(MAKEWORD(2,2),&wsa) != 0)
	{
		return;
	}

	//socket
	m_socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP,NULL,0,0);
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

	retval = WSAConnect(m_socket, (SOCKADDR*)&m_serverAddr, sizeof(m_serverAddr),NULL,NULL,NULL,NULL);
	WSAAsyncSelect(m_socket, window, WM_SOCKET, FD_CLOSE | FD_READ);

	
	m_Send_Wsabuf.buf = m_Send_Buffer;
	m_Send_Wsabuf.len = MAX_BUFFER;

	m_Recv_Wsabuf.buf = m_Recv_Buffer;
	m_Recv_Wsabuf.len = MAX_BUFFER;

	if (client) 
	{
		m_gameClient = client;
	}
	

	
}
void CNetwork::ClientError()
{
	exit(-1);
}
void CNetwork::ShutDown()
{
	for(int i=0; i<MAX_USER;++i)
	{
		//클라이언트 강제 종료	
	}

	WSACleanup();
}

void CNetwork::ReadPacket(SOCKET sock)
{
	DWORD iobyte, ioflag = 0;

	int ret = WSARecv(sock, &m_Recv_Wsabuf, 1, &iobyte, &ioflag, NULL, NULL);
	if(ret)
	{
		int err_code = WSAGetLastError();
		cout << "Recv Error[" << err_code << "]\n";
	}

	BYTE *ptr = reinterpret_cast<BYTE*>(m_Recv_Buffer);

	while(0 != iobyte)
	{
		if(0 == m_In_Packet_Size)
		{
			m_In_Packet_Size = ptr[0];
		}
		if( iobyte + m_Saved_Packet_Size >= m_In_Packet_Size)
		{
			memcpy(m_packet_buffer + m_Saved_Packet_Size, ptr, m_In_Packet_Size - m_Saved_Packet_Size);
			
			if(m_gameClient)
			{

				m_gameClient->ProcessPacket(m_packet_buffer);

			}

			ptr += m_In_Packet_Size - m_Saved_Packet_Size;
			iobyte -= m_In_Packet_Size - m_Saved_Packet_Size;
			m_In_Packet_Size = 0;
			m_Saved_Packet_Size = 0;

		}
		else
		{
			memcpy(m_packet_buffer + m_Saved_Packet_Size, ptr, iobyte);
			m_Saved_Packet_Size += iobyte;
			iobyte = 0;
		}
	}

}
