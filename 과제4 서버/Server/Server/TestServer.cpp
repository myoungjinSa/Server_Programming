#include "TestServer.h"
#include <iostream>
#include <map>


CServerFramework::CServerFramework()
{
}

CServerFramework::~CServerFramework()
{
}

void CServerFramework::Error_Display(char *msg,int err_no)
{
	WCHAR *lpMsgBuf;

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, err_no,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL
	);
	std::cout << msg;
	wcout << L"에러" << lpMsgBuf << std::endl;
	while (true);
	LocalFree(lpMsgBuf);

}

void CServerFramework::Initialize()
{
	for(auto& cl : clients)
	{
		cl.conneceted = false;
	}
}


char CServerFramework::GetNewId()
{
	while (true)
	{
		for (int i = 0; i < MAX_USER; ++i)
		{
			if (clients[i].conneceted == false)
			{
				clients[i].conneceted = true;
				m_playerCount = i + 1;
				return i;
			}
		}
	}
}

void CServerFramework::Send_Packet(int key,char *packet)
{
	SOCKET client_s = clients[key].socket;		//32비트이기 때문에 소켓인덱스 
	DWORD flags = 0;

	OVERLAPPED_EX *over = reinterpret_cast<OVERLAPPED_EX*>(malloc(sizeof(OVERLAPPED_EX)));


	{	
		over->dataBuffr.len = packet[0];
		over->dataBuffr.buf = over->messageBuffer;
		memcpy(over->messageBuffer, packet, packet[0]);
		ZeroMemory(&(over->overlapped), sizeof(WSAOVERLAPPED));
		over->is_recv = false;
		
		if (WSASend(client_s, &over->dataBuffr, 1,NULL, 0, &(over->overlapped), NULL) == SOCKET_ERROR)
		{
			if (WSAGetLastError() != WSA_IO_PENDING)
			{
				cout<<"Error - Fail WSARecv(error_code : %d)\n";
				cout << WSAGetLastError() << "\n";
			}
		}
	}

}

void CServerFramework::Send_Remove_Player_Packet(char to,char id)
{
	SC_PACKET_REMOVE_PLAYER packet;

	packet.id = id;
	packet.size = sizeof(packet);
	packet.type = SC_REMOVE_PLAYER;

	Send_Packet(to, reinterpret_cast<char*>(&packet));

}

void CServerFramework::Send_Login_Packet(char to)
{
	SC_PACKET_LOGIN_OK packet;

	packet.id = to;
	packet.size = sizeof(packet);
	packet.type = SC_LOGIN_OK;

	Send_Packet(to, reinterpret_cast<char*>(&packet));
}

void CServerFramework::Send_Put_Player_Packet(char to,char object)
{
	SC_PACKET_PUT_PLAYER packet;

	packet.id = object;
	packet.size = sizeof(packet);
	packet.type = SC_PUTPLAYER;


	//packet.playerCount = GetNewId();
	packet.x = clients[object].x;
	packet.y = clients[object].y;
	packet.playerCount = m_playerCount;

	Send_Packet(to, reinterpret_cast<char*>(&packet));
}

void CServerFramework::Send_Pos_Packet(char to, char object,char data)
{
	SC_PACKET_POS packet;

	packet.id = object;
	packet.size = sizeof(packet);
	packet.type = SC_POS;

	packet.x = clients[object].x;
	packet.y = clients[object].y;
	packet.dir = data;

	Send_Packet(to, reinterpret_cast<char*>(&packet));
}

void CServerFramework::Network_Initialize()
{
		// Winsock Start - windock.dll 로드
	WSADATA WSAData;
	if (WSAStartup(MAKEWORD(2, 2), &WSAData) != 0)
	{
		cout << "Error - Can not load 'winsock.dll' file\n";
		return ;
	}

	// 1. 소켓생성  
	m_listen_Socket = WSASocketW(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);		//OVERLAPPED모델은 반드시 WSA_FLAG_OVERLAPPED 를 사용해야함.
	if (m_listen_Socket == INVALID_SOCKET)
	{
		cout<<"Error - Invalid socket"<<"\n";
		return ;
	}

	// 서버정보 객체설정
	SOCKADDR_IN serverAddr;
	memset(&serverAddr, 0, sizeof(SOCKADDR_IN));
	serverAddr.sin_family = PF_INET;
	serverAddr.sin_port = htons(SERVER_PORT);
	serverAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);			//어떤 주소가 오든지 다 받겠다 - INADDR_ANY

	// 2. 소켓설정
	if (::bind(m_listen_Socket, (struct sockaddr*)&serverAddr, sizeof(SOCKADDR_IN)) == SOCKET_ERROR)
	{
		printf("Error - Fail bind\n");
		// 6. 소켓종료
		closesocket(m_listen_Socket);
		// Winsock End
		WSACleanup();
		return ;
	}

	// 3. 수신대기열생성
	if (listen(m_listen_Socket, 5) == SOCKET_ERROR)		//listen(socket, backlog) 
	{													//backlog -> 들어오는 연결수락 대기열 수
		printf("Error - Fail listen\n");
		
		// 6. 소켓종료
		closesocket(m_listen_Socket);
		
		
		// Winsock End
		WSACleanup();
		return ;
	}
}

void CServerFramework::Do_Recv(char id)
{
	DWORD flags = 0;

	SOCKET client_s = clients[id].socket;

	OVERLAPPED_EX *over = &clients[id].overlapped;

	{	
		over->dataBuffr.len = MAX_BUFFER;
		over->dataBuffr.buf = over->messageBuffer;
		
		ZeroMemory(&(over->overlapped), sizeof(WSAOVERLAPPED));
		
		flags = 0;
		if (WSARecv(client_s, &over->dataBuffr, 1,NULL, &flags, &(over->overlapped), NULL) == SOCKET_ERROR)
		{
			int err_no = WSAGetLastError();

			if (err_no != WSA_IO_PENDING)
			{
				Error_Display(const_cast<char*>("RECV ERRROR"), err_no);
			
			}
		}
	}
}



void CServerFramework::Process_Packet(char id,char* buf)
{
	CS_PACKET_UP * packet = reinterpret_cast<CS_PACKET_UP*>(buf);


	short x = clients[id].x;
	short y = clients[id].y;
	char dir = CS_IDLE;

	switch(packet->type)
	{
	case CS_UP:
		y -= fHeightStep;
		dir = CS_UP;
		std::cout << "UP" << "\n";
		if (y < 0) y = 0;
		break;
	case CS_DOWN:
		y += fHeightStep;
		dir = CS_DOWN;
		std::cout << "DOWN" << "\n";
		if (y > WORLD_HEIGHT) y = WORLD_HEIGHT;
		break;

	case CS_LEFT:
		x -= fWidthStep;
		dir = CS_LEFT;
		std::cout << "LEFT" << "\n";
		if (x < 0) x = 0;
		break;
	case CS_RIGHT:
		x += fWidthStep;
		dir = CS_RIGHT;
		std::cout << "RIGHT" << "\n";
		if (x > WORLD_WIDTH - 1) x++;
		break;
	default:
		cout << "Unknown Packet: Type error\n";
		while (true);
	}
	clients[id].x = x;
	clients[id].y = y;

	for(int i=0;i<MAX_USER ;++i)
	{
		if (clients[i].conneceted == true) {

			Send_Pos_Packet(i, id,dir);
			Send_Pos_Packet(id, i,dir);
		}
	}
}


void CServerFramework::Disconnect(int id)
{
	for(int i=0;i<MAX_USER;++i)
	{
		if (clients[i].conneceted == false) 
		{
			Send_Remove_Player_Packet(i, id);
		}

	}
	closesocket(clients[id].socket);
	clients[id].conneceted = false;
}

void CServerFramework::Do_Accept()
{
	Network_Initialize();

	SOCKADDR_IN clientAddr;
	int addrLen = sizeof(SOCKADDR_IN);
	memset(&clientAddr, 0, addrLen);
	SOCKET clientSocket;
	DWORD flags;

	while (true)
	{
		clientSocket = accept(m_listen_Socket, (struct sockaddr *)&clientAddr, &addrLen);
		if (clientSocket == INVALID_SOCKET)
		{
			cout<<"Error - Accept Failure"<<"\n";
			return ;
		}

		char new_id = GetNewId();


	
		memset(&clients[new_id], 0x00, sizeof(struct SOCKETINFO));
		
		clients[new_id].socket = clientSocket;
		
		
		clients[new_id].overlapped.dataBuffr.len = MAX_BUFFER;
		clients[new_id].overlapped.dataBuffr.buf = clients[new_id].packet_buf;
		clients[new_id].overlapped.is_recv = true;
		

		clients[new_id].x = fStartX + (fWidthStep * m_playerCount);
		clients[new_id].y = fStartY + (fHeightStep * m_playerCount);
		flags = 0;

		
		CreateIoCompletionPort(reinterpret_cast<HANDLE>(clientSocket), g_iocp, new_id, 0);

		clients[new_id].conneceted = true;


		Send_Login_Packet(new_id);
		for(int i =0;i<MAX_USER;++i)
		{
			if (clients[i].conneceted == false)
			{
				continue;
			}
			Send_Put_Player_Packet(i, new_id);
		}
		
		for(int i=0;i<MAX_USER;++i)
		{
			if(clients[i].conneceted == false)
			{
				continue;
			}
			if (i == new_id)
			{
				continue;
			}
			Send_Put_Player_Packet(new_id, i);
		}
		Do_Recv(new_id);

		//ACCEPT 되자마자 RECV를 해줌, -> 
		//if (WSARecv(clients[new_id].socket, &clients[new_id].overlapped.dataBuffr, 1, NULL, &flags,&(clients[new_id].overlapped) , 0))	//플래그에는 NULL쓰면 안됨, -> 플래그가 0인가아닌가 검사를하기때문에
		//{
		//	//WSA_IO_PENDING: An overlapped operation was successfully initiated and completion will be indicated at a later time. 
		//	if (WSAGetLastError() != WSA_IO_PENDING)	//IOPENDING에러가 나는게 정상
		//	{
		//		cout<<"Error - IO pending Failure"<<"\n";
		//		return ;
		//	}
		//}
		//else {
		//	//동기식- 여기서 처리하면 프로그래밍이 지저분해짐,위의 인자가 NULL이기때문에 여기로 오지 않음.
		//	cout << "Non Overlapped Recv return.\n";
		//	return ;
		//}
	}

	// 6-2. 리슨 소켓종료
	closesocket(m_listen_Socket);

	// Winsock End
	WSACleanup();

	return ;

}
void CServerFramework::Worker_Thread()
{
	while(true)
	{
		DWORD io_byte;
		ULONG key;
		
		OVERLAPPED_EX *lpOver_ex;


		BOOL is_Error = GetQueuedCompletionStatus(g_iocp, &io_byte, &key, reinterpret_cast<LPWSAOVERLAPPED*>(&lpOver_ex), INFINITE);
		
		if(is_Error == FALSE)
		{
			Error_Display(const_cast<char*>("GetQueuedCompletionStatus Error"), WSAGetLastError());
		}

		if( io_byte == 0)
		{
			Disconnect(key);
		}

		if(lpOver_ex->is_recv) 
		{
			int rest_size = io_byte;
			char packet_size = 0;

			//이미 전에 받은게 있을경우
			if(0 < clients[key].prev_size)
			{
				packet_size = clients[key].packet_buf[0];
			}
			char *ptr = lpOver_ex->messageBuffer;
			while(rest_size > 0)
			{
				if(0 == packet_size)
				{
					packet_size = ptr[0];
				}
				int required = packet_size - clients[key].prev_size;
				if( rest_size >= required)
				{
					memcpy(clients[key].packet_buf + clients[key].prev_size, ptr, required);
					Process_Packet(key, clients[key].packet_buf);
					rest_size -= required;
					ptr += required;

					packet_size = 0;


				}
				else
				{
					memcpy(clients[key].packet_buf + clients[key].prev_size, ptr, rest_size);
					rest_size = 0;


				}
				
			}
			Do_Recv(key);
			
		}
		else 
		{
			//delete
			delete lpOver_ex;


		}


	}
}


