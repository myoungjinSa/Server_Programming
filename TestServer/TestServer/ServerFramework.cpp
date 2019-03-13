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
		err_quit("bind()");
	}
	retval = listen(m_listenSocket, MAX_CLIENT_COUNT);
	if (retval == SOCKET_ERROR)
	{
		err_quit("listen()");
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
		m_vClientSocket.emplace_back(m_clientSocket, PLAYER_1,m_fWidthStep*4.0f,m_fHeightStep*4.0f);
		m_GameTimer.Start();
	}

	return m_clientSocket;
}


void CServerFramework::RecvPacket()
{
	int retval = 0;
	size_t packetSize = 0;

	
	while (true) {
		m_GameTimer.Tick(60.0f);
		CS_RUN cs_runPacket;
		retval = recvn(m_vClientSocket[PLAYER_1].clientSocket, (char*)&cs_runPacket, sizeof(cs_runPacket), 0);
		if (retval == SOCKET_ERROR)
		{
			err_display("recvn( )");
			return;
		}
	
		
		Update(cs_runPacket);
		SendPacket();
	}
	

}

void CServerFramework::SendPacket()
{
	int retval = 0;
	size_t packetSize = 0;


	SC_RUN sc_runPacket;
	sc_runPacket.posX = m_vClientSocket[PLAYER_1].pos.x;
	sc_runPacket.posY = m_vClientSocket[PLAYER_1].pos.y;
	
	retval = send(m_vClientSocket[PLAYER_1].clientSocket, (char*)&sc_runPacket, sizeof(sc_runPacket),0);
	if(retval == SOCKET_ERROR)
	{
		err_display("send()");
		return;
	}


}

void CServerFramework::ProcessKeyInput(const byte& keyInput)
{
	
	switch(keyInput)
	{
	case KEY_IDLE:

		//cout << "IDLE" << endl;
		break;
	case KEY_RIGHT:
		m_vClientSocket[PLAYER_1].pos.x += m_fWidthStep;
		//cout << "RIGHT" << endl;
		break;
	case KEY_LEFT:
		m_vClientSocket[PLAYER_1].pos.x -= m_fWidthStep;
		//cout << "LEFT" << endl;
		break;
	case KEY_UP:
		m_vClientSocket[PLAYER_1].pos.y -= m_fHeightStep;
		//cout << "UP" << endl;
		break;
	case KEY_DOWN:
		m_vClientSocket[PLAYER_1].pos.y += m_fHeightStep;
		//cout << "DOWN" << endl;
		break;
	default:
		break;
	}
}

void CServerFramework::Update(const CS_RUN& cs_runPacket)
{
	
	ProcessKeyInput(cs_runPacket.key);

}

void CServerFramework::Destroy()
{
	m_GameTimer.Stop();
	for(auto iter = m_vClientSocket.begin();iter != m_vClientSocket.end();++iter)
	{
		closesocket(iter->clientSocket);
	}
	m_vClientSocket.clear();

	closesocket(m_listenSocket);
	WSACleanup();
}