
#include "TestServer.h"
#include "Terrain.h"
#include "Animation.h"



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
	std::wcout << L"에러" << lpMsgBuf << std::endl;
	while (true);
	LocalFree(lpMsgBuf);

}

void CServerFramework::Initialize()
{
	for(auto& cl : clients)
	{
		cl.conneceted = false;
	}
	m_GameTimer.Start();
	//기본 물리 변수 초기화
	InitializePhysics();
	
	m_pAnimationInfo =  std::make_unique<CAnimation>();

	//FILE *pInFile = NULL;
	//::fopen_s(&pInFile, "../../../FreezeBomb/Resource/Models/EvilBear.bin", "rb");

	//::rewind(pInFile);


	m_pAnimationInfo->LoadAnimationInfo();

}

void CServerFramework::SetHeightmapInfo(LPCSTR filename,int nWidth, int nLength, XMFLOAT3 xmf3Scale)
{
	m_pHeightMap = new CHeightMapImage(filename, nWidth, nLength, xmf3Scale);

}
void CServerFramework::DeleteHeightmapInfo()
{
	if (m_pHeightMap)
		delete m_pHeightMap;
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
				std::cout<<"Error - Fail WSARecv(error_code : %d)\n";
				std::cout << WSAGetLastError() << "\n";
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
	SC_INGAME_PACKET packet;


	
	packet.id = object;
	packet.size = sizeof(packet);
	packet.type = SC_PUTPLAYER;
	packet.player_Packet = clients[object].player;


	Send_Packet(to, reinterpret_cast<char*>(&packet));

}

void CServerFramework::Send_Pos_Packet(char to, char object)
{
	SC_INGAME_PACKET packet;

	packet.id = object;
	packet.size = sizeof(packet);
	packet.type = SC_POS;
	//packet.player_Packet.xmf3Position = clients[object].player.xmf3Position;
	//packet.player_Packet.xmf3Look = clients[object].player.xmf3Look;
	//packet.player_Packet.xmf3Right = clients[object].player.xmf3Right;
	//packet.
	packet.player_Packet = clients[object].player;


	
	Send_Packet(to, reinterpret_cast<char*>(&packet));

}

void CServerFramework::Network_Initialize()
{
		// Winsock Start - windock.dll 로드
	WSADATA WSAData;
	if (WSAStartup(MAKEWORD(2, 2), &WSAData) != 0)
	{
		std::cout << "Error - Can not load 'winsock.dll' file\n";
		return ;
	}

	// 1. 소켓생성  
	m_listen_Socket = WSASocketW(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);		//OVERLAPPED모델은 반드시 WSA_FLAG_OVERLAPPED 를 사용해야함.
	if (m_listen_Socket == INVALID_SOCKET)
	{
		std::cout<<"Error - Invalid socket"<<"\n";
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
void CServerFramework::RotateModel(SOCKETINFO& clients,float x,float y,float z)
{
	if (x != 0.0f)
	{
		clients.player.pitch += x;
		if (clients.player.pitch > +89.0f) { x -= (clients.player.pitch - 89.0f); clients.player.pitch = +89.0f; }
		if (clients.player.pitch < -89.0f) { x -= (clients.player.pitch + 89.0f); clients.player.pitch = -89.0f; }
	}
	if (y != 0.0f)
	{
		clients.player.yaw += y;
		if (clients.player.yaw > 360.0f) clients.player.yaw -= 360.0f;
		if (clients.player.yaw< 0.0f) clients.player.yaw += 360.0f;
	}
	if (z != 0.0f)
	{
		clients.player.roll += z;
		if (clients.player.roll > +20.0f) { z -= (clients.player.roll - 20.0f); clients.player.roll = +20.0f; }
		if (clients.player.roll < -20.0f) { z -= (clients.player.roll + 20.0f); clients.player.roll = -20.0f; }
	}
		
	//if (y != 0.0f)
	//{
	//	XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Up), XMConvertToRadians(y));
	//	m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, xmmtxRotate);
	//	m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, xmmtxRotate);
	//}
}
void CServerFramework::RotateClientsAxisY(SOCKETINFO& clients,float fTimeElapsed)
{
	XMFLOAT3& xmf3Look = clients.player.xmf3Look;
	XMFLOAT3& xmf3Right = clients.player.xmf3Right;
	XMFLOAT3& xmf3Up = clients.player.xmf3Up;

	clients.xmf3LastLookVector = xmf3Look;
	clients.xmf3LastRightVector = xmf3Right;
	clients.xmf3LastUpVector = xmf3Up;

	if(clients.player.direction & DIR_RIGHT)
	{
		float fDotProduct = Vector3::DotProduct(xmf3Look, xmf3Right);

		float fAngle = ::IsEqual(fDotProduct, 1.0f) ? 0.0f : ((fDotProduct > 1.0f) ? XMConvertToDegrees(acos(fDotProduct)) : 90.0f);


		XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&xmf3Up), XMConvertToRadians(fAngle*fTimeElapsed));
		xmf3Look = Vector3::TransformNormal(xmf3Look, xmmtxRotate);
		xmf3Right = Vector3::TransformNormal(xmf3Right, xmmtxRotate);
		
		float cxDelta = xmf3Right.x - clients.xmf3LastRightVector.x;
		float cyDelta = xmf3Up.y - clients.xmf3LastUpVector.y;
		float czDelta = xmf3Look.z - clients.xmf3LastLookVector.z;
	

		RotateModel(clients,0.0f ,fAngle*fTimeElapsed , 0.0f);



	}
	else if(clients.player.direction & DIR_LEFT)
	{
		float fDotProduct = Vector3::DotProduct(xmf3Look, xmf3Right);

		float fAngle = ::IsEqual(fDotProduct, 1.0f) ? 0.0f : ((fDotProduct > 1.0f) ? XMConvertToDegrees(acos(fDotProduct)) : 90.0f);

		XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&xmf3Up), XMConvertToRadians(-(fAngle*fTimeElapsed)));
		xmf3Look = Vector3::TransformNormal(xmf3Look, xmmtxRotate);
		xmf3Right = Vector3::TransformNormal(xmf3Right, xmmtxRotate);

		float czDelta = xmf3Look.z - clients.xmf3LastLookVector.z;
	
		RotateModel(clients,0.0f ,-fAngle*fTimeElapsed , 0.0f);



	}


	xmf3Look = Vector3::Normalize(xmf3Look);
	xmf3Right = Vector3::CrossProduct(xmf3Up, xmf3Look, true);
	xmf3Up = Vector3::CrossProduct(xmf3Look, xmf3Right, true);


	


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

	

	//char x = clients[id].x;
	//char y = clients[id].y;
	UCHAR key;

	switch(packet->type)
	{
	case CS_UP:
		//std::cout << "클라이언트 id - " << id << "\n";
		//std::cout << "키 입력 UP\n";
		key = VK_UP;
		clients->player.direction = DIR_FORWARD;
		//--y;
		//if (y < 0) y = 0;
		break;
	case CS_DOWN:
		//std::cout << "클라이언트 id - " << id << "\n";
		//std::cout << "키 입력 DOWN\n";
		key = VK_DOWN;
		clients->player.direction = DIR_BACKWARD;
		//++y;
		//if (y > WORLD_HEIGHT) y = WORLD_HEIGHT;
		break;

	case CS_LEFT:
		//std::cout << "클라이언트 id - " << id << "\n";
		//std::cout << "키 입력 LEFT\n";
		key = VK_LEFT;
		clients->player.direction = DIR_LEFT;
		//x--;
		//if (x < 0) x = 0;
		break;
	case CS_RIGHT:
		//std::cout << "클라이언트 id - " << id << "\n";
		//std::cout << "키 입력 RIGHT\n";
		key = VK_RIGHT;
		clients->player.direction = DIR_RIGHT;
		//++x;
		//if (x > WORLD_WIDTH - 1) x++;
		break;
	default:
		std::cout << "Unknown Packet: Type error\n";
		while (true);
	}
	//clients[id].x = x;
	//clients[id].y = y;

	SetDirection(clients[id], key);

	UpdateClientPos(clients[id], m_GameTimer.GetTimeElapsed());
	
	for(int i=0;i<MAX_USER ;++i)
	{
		if (clients[i].conneceted == true) {

			Send_Pos_Packet(i, id);
		}
	}

	//값 초기화
	clients[id].player.pitch = 0.0f;
	clients[id].player.yaw = 0.0f;
	clients[id].player.roll = 0.0f;
	clients[id].player.direction = 0x00;
}


void CServerFramework::Disconnect(int id)
{
	for(int i=0;i<MAX_USER;++i)
	{
		if (clients[i].conneceted == false) 
		{
			Send_Remove_Player_Packet(i, id);
			std::cout << "diconnect : ID -" << id << "\n";
		}

	}
	closesocket(clients[id].socket);
	clients[id].conneceted = false;
}
//void CServerFramework::Initialize_WorldMatrix(SOCKETINFO& clients)
//{
//	clients.player.xmf3Position = XMFLOAT3()
//
//}

void CServerFramework::ProcessClientHeight(SOCKETINFO& clients)
{
	int z = (int)(clients.player.xmf3Position.z / m_pHeightMap->GetScale().z);
	bool bReverseQuad = ((z % 2) != 0);

	float fHeight = m_pHeightMap->GetHeight(clients.player.xmf3Position.x, clients.player.xmf3Position.z, bReverseQuad);
	if(clients.player.xmf3Position.y< fHeight)
	{
		clients.player.velocity.y = 0.0f;
		clients.player.xmf3Position.y = fHeight;
	}

}
void CServerFramework::InitializePhysics()
{
	m_physicsInfo.m_fFriction = 250.0f;
	m_physicsInfo.m_xmf3Gravity = XMFLOAT3(0.0f, -250.0f, 0.0f);
	m_physicsInfo.m_fMaxVelocityXZ = 40.0f;
	m_physicsInfo.m_fMaxVelocityY = 400.0f;
}

void CServerFramework::SetClient_FirstPosition(SOCKETINFO& clients)
{

	clients.player.xmf3Position = XMFLOAT3(100.0f, 0.0f, 100.0f);
	clients.player.xmf3Look = XMFLOAT3(0.0f, 0.0f, 1.0f);
	clients.player.xmf3Right = XMFLOAT3(1.0f, 0.0f, 0.0f);
	clients.player.xmf3Up = XMFLOAT3(0.0f, 1.0f, 0.0f);
}

void CServerFramework::ProcessFriction(SOCKETINFO& clients)
{
	float fLength = Vector3::Length(clients.player.velocity);
	float fDeclaration = (m_physicsInfo.m_fFriction * m_GameTimer.GetTimeElapsed());

	if(fDeclaration > fLength)
	{
		fDeclaration = fLength;
		clients.player.velocity = Vector3::Add(clients.player.velocity, Vector3::ScalarProduct(clients.player.velocity, -fDeclaration,true));
	}
	
}

void CServerFramework::UpdateClientPos(SOCKETINFO& clients, float fTimeElapsed)
{
	if (clients.player.direction == DIR_FORWARD) {
		clients.player.velocity = Vector3::Add(clients.player.velocity, clients.player.xmf3Look, 1.0f);
	}
	if( clients.player.direction == DIR_BACKWARD)
	{
		clients.player.velocity = Vector3::Add(clients.player.velocity, clients.player.xmf3Look, -1.0f);
	}
	if(clients.player.direction == DIR_LEFT || clients.player.direction == DIR_RIGHT)
	{
		RotateClientsAxisY(clients, fTimeElapsed);
	}
	clients.player.velocity = Vector3::Add(clients.player.velocity, m_physicsInfo.m_xmf3Gravity);
	float fLength = sqrtf(clients.player.velocity.x * clients.player.velocity.x + clients.player.velocity.z * clients.player.velocity.z);
	if (fLength > m_physicsInfo.m_fMaxVelocityXZ)
	{
		clients.player.velocity.x *= (m_physicsInfo.m_fMaxVelocityXZ / fLength);
		clients.player.velocity.z *= (m_physicsInfo.m_fMaxVelocityXZ / fLength);
	}

	float fLengthY = sqrtf(clients.player.velocity.y * clients.player.velocity.y);

	
	XMFLOAT3 xmf3Velocity = Vector3::ScalarProduct(clients.player.velocity, fTimeElapsed, false);
	
	clients.player.xmf3Position = Vector3::Add(clients.player.xmf3Position, clients.player.velocity);

	//clients.player.xmf3Position = xmf3Velocity;
	
	clients.player.velocity = clients.player.velocity;

	ProcessClientHeight(clients);
	ProcessFriction(clients);
	
}


void CServerFramework::DecideClientsAnimation(SOCKETINFO& clients,int& nAnimationNum,float& fTimeElapsed)
{
	
	
}
void CServerFramework::UpdateClientsAnimation(SOCKETINFO& clients,float& fTimeElapsed)
{

	
}
void CServerFramework::SetDirection(SOCKETINFO& clients,UCHAR& key)
{
	static UCHAR pKeysBuffer[256];
	
	DWORD dwDirection = 0;

	//XMFLOAT3 xmf3Shift = XMFLOAT3(0, 0, 0);
	if(key == VK_UP)
	{
		dwDirection |= DIR_FORWARD;
		clients.player.direction = dwDirection;
		
		//std::cout << "방향:UP\n";
	}
	if(key == VK_DOWN )
	{
		dwDirection |= DIR_BACKWARD;
		clients.player.direction = dwDirection;
		
		//std::cout << "방향:DOWN\n";

	}
	if( key == VK_LEFT )
	{
		dwDirection |= DIR_LEFT;
		clients.player.direction = dwDirection;
		
		//std::cout << "방향:LEFT\n";
	}
	if( key == VK_RIGHT )
	{
		dwDirection |= DIR_RIGHT;
		clients.player.direction = dwDirection;
		//std::cout << "방향:RIGHT\n";

	}

	//UpdateClientPos(clients, m_GameTimer.GetTimeElapsed());
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
			std::cout<<"Error - Accept Failure"<<"\n";
			return ;
		}

		char new_id = GetNewId();

		memset(&clients[new_id], 0x00, sizeof(struct SOCKETINFO));
		
		clients[new_id].socket = clientSocket;
		
		
		clients[new_id].overlapped.dataBuffr.len = MAX_BUFFER;
		clients[new_id].overlapped.dataBuffr.buf = clients[new_id].packet_buf;
		clients[new_id].overlapped.is_recv = true;
		
		
		flags = 0;

		
		CreateIoCompletionPort(reinterpret_cast<HANDLE>(clientSocket), g_iocp, new_id, 0);

		clients[new_id].conneceted = true;

		std::cout << "connect: ID -  " << new_id << "\n";

	
		

		Send_Login_Packet(new_id);

		
		SetClient_FirstPosition(clients[new_id]);

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


		
		m_GameTimer.Tick(0.0f);
		
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


