#include "TestServer.h"

int updateTime = timeGetTime();
mutex g_mutex;
//mutex npc_mutex;
SOCKETINFO clients[NPC_ID_START + NUM_NPC];	
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
	/*for(auto& cl : clients)
	{
		cl.connected = false;
		cl.viewList.clear();
	}*/

	for(int i=0;i<MAX_USER;++i)
	{
		clients[i].connected = false;
		clients[i].viewList.clear();
	
	}

	for(int i=NPC_ID_START;i<NPC_ID_START + NUM_NPC;++i)
	{
		clients[i].x = rand() % WORLD_WIDTH;
		clients[i].y = rand() % WORLD_HEIGHT;
	}

}


int CServerFramework::GetNewId()
{
	while (true)
	{
		for (int i = 0; i < MAX_USER; ++i)
		{
			if (clients[i].connected == false)
			{
				clients[i].connected = true;
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
		over->command = COMMAND::SEND;
		
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

void CServerFramework::Send_Remove_Player_Packet(int to,int id)
{
	SC_PACKET_REMOVE_PLAYER packet;

	packet.id = id;
	packet.size = sizeof(packet);
	packet.type = SC_REMOVE_PLAYER;

	Send_Packet(to, reinterpret_cast<char*>(&packet));

}

void CServerFramework::Send_Login_Packet(int to)
{
	SC_PACKET_LOGIN_OK packet;

	packet.id = to;
	packet.size = sizeof(packet);
	packet.type = SC_LOGIN_OK;

	Send_Packet(to, reinterpret_cast<char*>(&packet));
}

void CServerFramework::Send_Put_Player_Packet(int to,int object)
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

void CServerFramework::Send_Pos_Packet(int to, int object)
{
	SC_PACKET_POS packet;

	packet.id = object;
	packet.size = sizeof(packet);
	packet.type = SC_POS;

	packet.x = clients[object].x;
	packet.y = clients[object].y;
	

	Send_Packet(to, reinterpret_cast<char*>(&packet));
}

void CServerFramework::Send_Npc_Put_Packet(int to,int object)
{
	SC_PACKET_NPC_PUT_PLAYER packet;

	packet.id = object;
	packet.size = sizeof(packet);
	packet.type = SC_NPC_PUT_PLAYER;

	packet.x = clients[object].x;
	packet.y = clients[object].y;


	Send_Packet(to, reinterpret_cast<char*>(&packet));

}

void CServerFramework::Send_Npc_Pos_Packet(int to,int object)
{
	SC_PACKET_NPC_POS packet;

	packet.id = object;
	packet.size = sizeof(packet);
	packet.type = SC_NPC_POS;


	packet.x = clients[object].x;
	packet.y = clients[object].y;


	Send_Packet(to, reinterpret_cast<char*>(&packet));

}

void CServerFramework::Send_Npc_Remove_Packet(int to , int object)
{
	SC_PACKET_REMOVE_PLAYER packet;

	packet.id = object;
	packet.size = sizeof(packet);
	packet.type = SC_NPC_REMOVE;

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

void CServerFramework::Do_Recv(int id)
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



void CServerFramework::Process_Packet(int id,char* buf)
{
	CS_PACKET_UP * packet = reinterpret_cast<CS_PACKET_UP*>(buf);


	char x = clients[id].x;
	char y = clients[id].y;
	
	switch(packet->type)
	{
	case CS_UP:
		--y;
		//dir = CS_UP;
		std::cout << "UP" << "\n";
		if (y < 0)
			y = 0;
		break;
	case CS_DOWN:
		++y;
		//dir = CS_DOWN;
		std::cout << "DOWN" << "\n";
		if (y >= WORLD_HEIGHT) 
			y = WORLD_HEIGHT-1;
		break;

	case CS_LEFT:
		//x -= fWidthStep;
		if (x > 0)
			x--;
		//dir = CS_LEFT;
		std::cout << "LEFT" << "\n";
		
		break;
	case CS_RIGHT:
		
		//dir = CS_RIGHT;
		std::cout << "RIGHT" << "\n";
		if (x < WORLD_WIDTH - 1)
			x++;
		break;
	default:
		cout << "Unknown Packet: Type error\n";
		while (true);
	}
	
	//g_mutex.lock();
	clients[id].x = x;
	clients[id].y = y;

	g_mutex.lock();

	unordered_set<int> old_viewList = clients[id].viewList;
	unordered_set<int> npc_oldViewList = clients[id].npcViewList;
	g_mutex.unlock();

	

	unordered_set<int> npc_viewList;
	unordered_set<int> new_viewList;



	for(int i =0;i<MAX_USER;++i)
	{
		if((clients[i].connected == true)&&(Is_Near_Object(id,i) == true) && (i!=id))
		{
			new_viewList.insert(i);
		}

	}

	for(int i=NPC_ID_START ; i<NUM_NPC;++i)
	{
		if(Is_Near_Object(id,i) == true)
		{
			npc_viewList.insert(i);
		}
	}

	for(auto npc : npc_viewList)
	{
		g_mutex.lock();
		if (npc_oldViewList.count(npc) == 0)
		{
			g_mutex.unlock();
			g_mutex.lock();
			clients[id].npcViewList.insert(npc);
			g_mutex.unlock();
			Send_Npc_Put_Packet(id, npc);
		}
		else if(npc_oldViewList.count(npc)!=0)
		{
			g_mutex.unlock();
			Send_Npc_Pos_Packet(id, npc);
		}

	}

		// Put Object
	// 나와 근처에 있는 오브젝트들에 대해
	for (auto client : new_viewList)
	{
		// 상대와 나에게 각각 지워주어야함
		if (old_viewList.count(client) == 0)
		{
			g_mutex.lock();
			clients[id].viewList.insert(client);
			g_mutex.unlock();
			Send_Put_Player_Packet(id, client);

			if (clients[client].viewList.count(id) != 0)
			{
				Send_Pos_Packet(client, id);
				
			}
			else
			{
				g_mutex.lock();
				clients[client].viewList.insert(id);
				g_mutex.unlock();
				Send_Put_Player_Packet(client, id);
			}
		}

		// old_viewList에 new_viewList에 있는 클라ID가 있을 때,
		else if (old_viewList.count(client) != 0)
		{
			// 상대방의 viewlist에 내가 있으면
			g_mutex.lock();
			if (clients[client].viewList.count(id) != 0)
			{
				g_mutex.unlock();
				Send_Pos_Packet(client, id);
			}
			// 상대방의 viewList에 내가 없으면
			else
			{
				g_mutex.unlock();
				g_mutex.lock();
				clients[client].viewList.insert(id);
				g_mutex.unlock();
				Send_Put_Player_Packet(client, id);				
			}
		}
	}


	
	
	// 안보이는 npc리스트
   unordered_set<int> npcRemoveList;
   vector<int> npcRemoveVecID;

   // 안보이는 플레이어 ID 리스트
   unordered_set<int> removedIDList;
   vector<int> removeVecID;
   for (int i = 0; i < MAX_USER; ++i)
   {
      // i에 해당하는 클라가 접속해있고, 
      // 나하고, 상대하고 근처에 없고, 
      // 또한 나하고 id하고 같지 않을 때,
	   //int user = i % MAX_USER;
      if (clients[id].connected == true
         && Is_Near_Object(id, i) == false
         && i != id)
      {
         removedIDList.insert(i);
		 removeVecID.emplace_back(i);

      }

	  //if(clients[user].connected == true 
		 // && Is_Near_Object(user,i) == false
		 // && (i >= NPC_ID_START))
	  //{
		 // npcRemoveList.insert(i);
		 // npcRemoveVecID.emplace_back(i);
	  //}
   }


	//for(int i=NPC_ID_START ; i<NUM_NPC;++i)
	//{
	//	if(Is_Near_Object(id,i) == false)
	//	{
	//		npcRemoveList.insert(i);
	//	}
	//}
  // for (const int& npc : npcRemoveVecID)
  // {

	 //if(npcRemoveList.count(npc) !=0 )
	 //{
		// npc_oldViewList.erase(npc);
		// Send_Npc_Remove_Packet(id, npc);
		// g_mutex.lock();
		// if(clients[id].npcViewList.count(npc)!=0)
		//{
		//	 g_mutex.unlock();
		//	 g_mutex.lock();
		//	 clients[id].npcViewList.erase(npc);
		//	 g_mutex.unlock();


		//}
		// g_mutex.unlock();
		//// npc_mutex.lock();

	 //}
  // }
  // 


   for(const int& i: removeVecID)
   {
	   if (i == id)
		   continue;
	   if(removedIDList.count(i)!=0)
	   {

		   old_viewList.erase(i);

		   Send_Remove_Player_Packet(id, i);
		   g_mutex.lock();
		   if(clients[i].viewList.count(id)!=0)
		   {
			    g_mutex.unlock();
				g_mutex.lock();
			    clients[i].viewList.erase(id);
			    g_mutex.unlock();
				g_mutex.lock();
			    Send_Remove_Player_Packet(i, id);
				g_mutex.unlock();
				g_mutex.lock();
		   }
		   g_mutex.unlock();
		   
	   }
   }
   //npcRemoveVecID.clear();
   removeVecID.clear();

	g_mutex.lock();
    clients[id].viewList = old_viewList;
	//clients[id].npcViewList = npc_oldViewList;
    g_mutex.unlock();
   
	
   


   for (int i = 0; i < MAX_USER; ++i)
   {
	   
		   if (clients[i].connected == true)
		   {
			      Send_Pos_Packet(i, id);
			   for (int j = NPC_ID_START; j < NUM_NPC; j++)
			   {
				   if(Is_Near_Object(j,i)==true)
				   {
					   Send_Npc_Put_Packet(i, j);
					   Send_Npc_Pos_Packet(i, j);
				   }
				   

				 
			   }
		   }
	   
	
   }
   

 /*  for (int i = 0; i < NUM_NPC ; ++i)
   {
	   int npc_id = i + NPC_ID_START;
	   if (clients[i].connected == true) {

		   Send_Pos_Packet(i, npc_id);
		  
	   }
   }*/

}



bool CServerFramework::Is_Near_Object(int a, int b)
{
	if (VIEW_RADIUS < abs(clients[a].x - clients[b].x))
		return false;
	if (VIEW_RADIUS < abs(clients[a].y - clients[b].y))
		return false;
	return true;
}

bool CServerFramework::Is_Move_Npc(int a, int b)
{
	if (MOVE_RADIUS < abs(clients[a].x - clients[b].x))
		return false;
	if (MOVE_RADIUS < abs(clients[a].y - clients[b].y))
		return false;
	return true;
}

void CServerFramework::Disconnect(int id)
{
	//g_mutex.lock();
	for(int i=0;i<MAX_USER;++i)
	{
		if (clients[i].connected == false) 
		{
			continue;
			//Send_Remove_Player_Packet(i, id);
		}
		g_mutex.lock();
		if(clients[i].viewList.count(id) != 0)
		{
			g_mutex.unlock();
			Send_Remove_Player_Packet(i, id);
		}
		else
		{
			g_mutex.unlock();
		}
	}
	
	closesocket(clients[id].socket);
	g_mutex.lock();
	clients[id].viewList.clear();
	g_mutex.unlock();
	clients[id].connected = false;
	
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

		int new_id = GetNewId();

		//memset(&clients[new_id], 0x00, sizeof(struct SOCKETINFO));
		
		//clients[new_id] = new SOCKETINFO;
		
		clients[new_id].socket = clientSocket;
		
		clients[new_id].overlapped.dataBuffr.len = MAX_BUFFER;
		clients[new_id].overlapped.dataBuffr.buf = clients[clientSocket].packet_buf;
		clients[new_id].overlapped.command = COMMAND::RECV;
		
		clients[new_id].x = fStartX;
		clients[new_id].y = fStartY;
		clients[new_id].prev_size = 0;
		ZeroMemory(&clients[new_id].overlapped.overlapped, sizeof(WSAOVERLAPPED));


		clients[new_id].viewList.clear();
		flags = 0;
		
		CreateIoCompletionPort(reinterpret_cast<HANDLE>(clientSocket), g_iocp, new_id, 0);

		clients[new_id].connected = true;

		
		Send_Login_Packet(new_id);
		Send_Put_Player_Packet(new_id, new_id);
		for(int i =0;i<MAX_USER;++i)
		{
			if (clients[i].connected == false)
			{
				continue;
			}
			if (i == new_id)
			{
				continue;
			}
			if (Is_Near_Object(i, new_id) == true)
			{

				clients[i].viewList.insert(new_id);
				Send_Put_Player_Packet(i, new_id);
			}
		}
		
		for(int i=0;i<MAX_USER;++i)
		{
			if(clients[i].connected == false)
			{
				continue;
			}
			if (i == new_id)
			{
				continue;
			}
			if (Is_Near_Object(new_id, i) == true)
			{
				clients[i].viewList.insert(i);
				Send_Put_Player_Packet(new_id, i);
				
			}
			
		}

		for(int i = 0 ;i< NUM_NPC;++i)
		{
			int npc_id = i + NPC_ID_START;
			if(true == Is_Near_Object(npc_id,new_id))
			{
				clients[new_id].viewList.insert(npc_id);
				Send_Npc_Put_Packet(new_id, npc_id);
				//Add_Timer(npc_id, COMMAND::MOVE, 1000);
			}
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
void CServerFramework::Add_Timer(int id,COMMAND c, int sleepTime)
{
	//timeBeginPeriod(1);
	//timeBeginPeriod(sleepTime); // 최소의 타이머 해상도를 결정한다. 



	//timeEndPeriod(sleepTime);   // timeBeginPeriod 에 의해 만들어진 타이머 해상도를 제거한다. 


	//timeEndPeriod(1);
}

void CServerFramework::Timer_Thread()
{
	
	timeBeginPeriod(1);
	
	while(1)
	{
		if(timeGetTime()- updateTime >= 1000)
		{
			updateTime = timeGetTime();
			for(int i=0;i<MAX_USER;++i)
			{
				if(clients[i].connected)
				{
					//IOCP를 사용할 경우 IOCP가 main loop가 되기 때문에 socket I/O 이외에도 모든 다른 작업할 내용을 추가 할 때 쓰인다. 
					//커널 Queue이벤트 추가함수
					OVERLAPPED_EX *lpOver_ex = new OVERLAPPED_EX;
					ZeroMemory(&(lpOver_ex->overlapped), sizeof(WSAOVERLAPPED));
					lpOver_ex->command = COMMAND::MOVE;


					for(int j=0;j<NUM_NPC;++j)
					{
						int npc_id = j + NPC_ID_START;
						if (Is_Near_Object(i, npc_id) == true)
						{
							Move_NPC(npc_id);
							Send_Pos_Packet(i, npc_id);
						}
					}
					//cout << "id - " << i << "\n";
					PostQueuedCompletionStatus(g_iocp, 1, i, &lpOver_ex->overlapped);
				}
			}

		}
	}

	timeEndPeriod(1);
}

void CServerFramework::Move_NPC(int npc)
{
	//g_mutex.lock();
	char x = clients[npc].x;
	char y = clients[npc].y;

	
	if (npc % 2 == 0)
	{
		x++;
		y++;
		if (x > WORLDX)
			x = WORLDX;
		if (y > WORLDY)
			y = WORLDY;
		
	}
	else
	{
		x--;
		y--;
		if (x < 0)
			x = 0;
		if (y < 0)
			y = 0;
	}


	g_mutex.lock();
	clients[npc].x = x;
	clients[npc].y = y;
	g_mutex.unlock();
	//Send_Npc_Pos_Packet(id, npc);
}
void CServerFramework::Worker_Thread()
{
	while(true)
	{
		DWORD io_byte;
		ULONGLONG key;
		
		OVERLAPPED_EX *lpOver_ex;
		
		BOOL is_Error = GetQueuedCompletionStatus(g_iocp, &io_byte, &key, reinterpret_cast<LPWSAOVERLAPPED*>(&lpOver_ex), INFINITE);
		
		if(is_Error == FALSE)
		{
			int err_no = WSAGetLastError();
			if (err_no != 64)
				Error_Display(const_cast<char*>("GetQueuedCompletionStatus Error"), err_no);
			else {
				Disconnect(key);
				continue;
			}
		}

		if( io_byte == 0)
		{
			Disconnect(key);
			continue;
		}

		if(lpOver_ex->command == COMMAND::RECV) 
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
		else if(lpOver_ex->command == COMMAND::MOVE)
		{

			
					for (int i = 0; i < NUM_NPC; ++i)
					{
						int npc_id = i + NPC_ID_START;

						for (int j = 0; j < MAX_USER; ++j)
						{
							if (clients[j].connected)
							{
								if (Is_Move_Npc(j, npc_id))
								{

									Send_Npc_Pos_Packet(j, npc_id);
								}
							}
						}

					}
			
			//for (int i = 0; i < MAX_USER; ++i) 
			//{
			//	for (auto npc : new_viewList)
			//	{
			//		// 상대와 나에게 각각 지워주어야함
			//		if (old_viewList.count(npc) == 0)
			//		{
			//			g_mutex.lock();
			//			clients[i].viewList.insert(npc);
			//			g_mutex.unlock();
			//			Move_NPC(key, npc);
			//			Send_Npc_Put_Packet(key, npc);
			//		}
			//		else
			//		{
			//			Move_NPC(key, npc);
			//			Send_Npc_Pos_Packet(key, npc);
			//		}

			//	}
			//}

			// 안보이는 플레이어 ID 리스트
		/*	unordered_set<int> removedIDList;
			vector<int> removeVecID;
			
			for (int i = 0; i < NUM_NPC; ++i)
			{
				int npc_id = i + NPC_ID_START;
				 if (clients[i].connected == true
					&& Is_Near_Object(npc_id, i) == false
					 && (i>=NPC_ID_START))
				 {
					 removedIDList.insert(npc_id);
					 removeVecID.emplace_back(npc_id);
				 }
			}
			 

			 for(const int& npc: removeVecID)
			{
			
			   if(removedIDList.count(npc)!=0)
			   {
				   g_mutex.lock();
				   old_viewList.erase(npc);
				   g_mutex.unlock();
				   Send_Npc_Remove_Packet(key,npc);
		   
				}
			  }
			  removeVecID.clear();*/

				/*Move_NPC(key, npc_id);
				if(Is_Near_Object(npc_id,key) == true)
				{
					g_mutex.lock();
					clients[key].viewList.insert(npc_id);
					g_mutex.unlock();
					
				}

				Send_Npc_Pos_Packet(key, npc_id);
			}*/

				
			// 안보이는 플레이어 ID 리스트
			//unordered_set<int> removedIDList;
			//vector<int> removeVecID;


		
			//removeVecID.clear();
		

			delete lpOver_ex;


		}
		else 
		{
			//delete
			delete lpOver_ex;


		}


	}
}


