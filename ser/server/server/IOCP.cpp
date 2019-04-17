#include "IOCP.h"
#include "Protocol.h"

// Ŭ�� Ű�� ã��	=> ���� Ŭ�󿡼� �а�, ���⸦ �ϱ� ������ ����ȭ�� �ؾ���.
SOCKETINFO clients[MAX_USER];
// IOCP ��ü �ڵ� (�б� ���� => ���� x) => ����ȭ �� �ʿ� ����
HANDLE g_IOCP;

mutex g_Mutex;

int main()
{
	// ��Ŀ ������ ����
	vector<thread> worker_threads;

	_wsetlocale(LC_ALL, L"korean");
	// �����߻��� �ѱ۷� ��µǵ��� ���
	wcout.imbue(locale("korean"));

	Initialize();

	// IOCP ��ü ����
	g_IOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);

	// ������ Ǯ
	for (int i = 0; i < THREAD_COUNT; ++i)
		worker_threads.emplace_back(thread{ Worker_Thread });

	// accept ������ ����
	thread accept_thread{ Do_Accept };

	// ������ ���� ���
	for (auto& threads : worker_threads)
		threads.join();

	accept_thread.join();
	
	CloseHandle(g_IOCP);
}

void Error_Display(const char* msg, int err_no)
{
	WCHAR* lpMsgBuf;
	FormatMessage
	(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		err_no,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,
		0,
		NULL
	);
	cout << msg;
	wcout << L"���� : " << lpMsgBuf << endl;

	// �����߻��� ���ѷ����� ���߰���
	//while (true);

	LocalFree(lpMsgBuf);
}

void Initialize()
{
	// SOCKINFO �ʱ�ȭ
	for (auto& client : clients)
	{
		client.over = { 0 };
		client.socket = { 0 };
		for(int i = 0; i < MAX_BUFFER; ++i)
			client.packet_buf[i] = { 0 };
		client.connected = false;
		client.sendBytes = 0;
		client.prev_size = 0;
		client.x = START_X;
		client.y = START_Y;
		client.viewList.clear();
	}
}

void Worker_Thread()
{
	while (true)
	{
		DWORD io_byte;
		unsigned long key;
		OVER_EX* lpover_ex;
		// lpover�� recv���� send���� ������ �־�� ��.
		BOOL is_error = GetQueuedCompletionStatus(g_IOCP, &io_byte, &key, reinterpret_cast<LPWSAOVERLAPPED*>(&lpover_ex), INFINITE);
		
		//  GetQueuedCompletionStatus( )�� �������� �ƴ��� Ȯ���Ѵ�
		if (is_error == FALSE)
			Error_Display(" GetQueuedCompletionStatus()", WSAGetLastError());

		// Ŭ��� �������ٸ� (Ŭ�� ������ ��)
		if (io_byte == 0)
		{
			Disconnect(key);
			cout << key << "�� Ŭ���̾�Ʈ ����" << endl;
		}

		// recv�� send�� ������ �� ó��
		if (lpover_ex->is_recv)
		{
			// ���� ��Ŷ ũ��
			int rest_size = io_byte;
			char* p = lpover_ex->messageBuffer;
			char packet_size = 0;

			// ���� ��Ŷ�� ���� ���
			if (clients[key].prev_size > 0)
				packet_size = clients[key].packet_buf[0];

			while (rest_size > 0)
			{
				if (packet_size == 0)
					packet_size = p[0];

				int required = packet_size - clients[key].prev_size;
				// ��Ŷ�� ���� �� �ִٸ�,
				if (rest_size >= required)
				{
					memcpy(clients[key].packet_buf + clients[key].prev_size, p, required);
					Process_Packet(key, clients[key].packet_buf);
					rest_size -= required;
					p += required;
					packet_size = 0;
				}
				// ��Ŷ�� ���� �� ���ٸ�,
				else
				{
					memcpy(clients[key].packet_buf + clients[key].prev_size, p, rest_size);
					rest_size = 0;
				}
			}
			Do_Recv(key);
		}
		else
		{
			// send�� ��,
			delete lpover_ex;
		}

	}
}

char Get_New_ID()
{
	// ���̵� ���� ������, �������鼭 ��ٸ�����
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

void Do_Accept()
{
	// Winsock Start - windock.dll �ε�
	WSADATA WSAData;
	if (WSAStartup(MAKEWORD(2, 2), &WSAData) != 0)
	{
		cout << "Error - Can not load 'winsock.dll' file\n";
		return;
	}

	// 1. ���ϻ��� 
	// Overlapped I/O�� �ϱ� ���ؼ� WSASocket ������ ���� ���� WSA_FLAG_OVERLAPPED�� �־���
	// WSASocketA,W ��������, ��Ƽ����Ʈ -> �����ڵ�� ����

	SOCKET listenSocket = WSASocketW(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (listenSocket == INVALID_SOCKET)
	{
		cout << "Error - Invalid socket\n";
		return;
	}

	// �������� ��ü����
	SOCKADDR_IN serverAddr;
	memset(&serverAddr, 0, sizeof(SOCKADDR_IN));
	serverAddr.sin_family = PF_INET;
	serverAddr.sin_port = htons(SERVER_PORT);
	// INADDR_ANY : � �ּҵ� �ްڴ�.
	serverAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

	// 2. ���ϼ���
	if (::bind(listenSocket, (struct sockaddr*)&serverAddr, sizeof(SOCKADDR_IN)) == SOCKET_ERROR)
	{
		cout << "Error - Fail bind\n";
		// 6. ��������
		closesocket(listenSocket);
		// Winsock End
		WSACleanup();
		return;
	}

	// 3. ���Ŵ�⿭����
	if (listen(listenSocket, 5) == SOCKET_ERROR)
	{
		cout << "Error - Fail listen\n";
		// 6. ��������
		closesocket(listenSocket);
		// Winsock End
		WSACleanup();
		return;
	}

	SOCKADDR_IN clientAddr;
	int addrLen = sizeof(SOCKADDR_IN);
	memset(&clientAddr, 0, addrLen);
	SOCKET clientSocket;
	DWORD flags;

	while (true)
	{
		clientSocket = accept(listenSocket, (sockaddr*)&clientAddr, &addrLen);
		if (clientSocket == INVALID_SOCKET)
		{
			cout << "Error - Accept Failure\n";
			return;
		}
		// �� ���̵� ��������
		char new_id = Get_New_ID();

		// Ŭ�������� 0���� �ʱ�ȭ
		clients[new_id].socket = clientSocket;
		clients[new_id].over.dataBuffer.len = MAX_BUFFER;
		clients[new_id].over.dataBuffer.buf = clients[clientSocket].over.messageBuffer;
		clients[new_id].over.is_recv = true;
		flags = 0;

		// Recv�ϱ����� IOCP�� ��������
		// Ŭ�� ������ �ڵ�� ����ȯ
		CreateIoCompletionPort(reinterpret_cast<HANDLE>(clientSocket), g_IOCP, new_id, 0);

		clients[new_id].connected = true;

		// �ʰ� �������� �����־����
		Send_Login_Ok_Packet(new_id);
		
		// ���ӵ� �ٸ� �������Ե� ������ ��
		for (int i = 0; i < MAX_USER; ++i)
		{
			if (clients[i].connected == false) 
				continue;

			// ���� ������ ��, ���̴� �ֵ鿡�Ը� ����
			if (Is_Near_Object(i, new_id) == true)
				Send_Put_Player_Packet(i, new_id);
		}

		for (int i = 0; i < MAX_USER; ++i)
		{
			// ������ �ٸ� �ֵ� ������ �����־�� ��
			if (clients[i].connected == false) 
				continue;
			
			// �������� �Ⱥ�������
			if (new_id == i) 
				continue;

			// ������ ���̴� �ֵ鸸 ������ ����
			if (Is_Near_Object(i, new_id) == true)
				Send_Put_Player_Packet(new_id, i);
		}
		Do_Recv(new_id);
	}

	// 6-2. ���� ��������
	closesocket(listenSocket);

	// Winsock End
	WSACleanup();
}

void Do_Recv(char id)
{
	DWORD flags = 0;
	SOCKET client_s = clients[id].socket;
	OVER_EX* over = &clients[id].over;

	// WSASend(���信 ����)�� �ݹ��� ���
	over->dataBuffer.len = MAX_BUFFER;
	over->dataBuffer.buf = over->messageBuffer;

	ZeroMemory(&(over->overlapped), sizeof(WSAOVERLAPPED));

	if (WSARecv(client_s, &over->dataBuffer, 1, nullptr, &flags, &(over->overlapped), nullptr) == SOCKET_ERROR)
	{
		int error_no = WSAGetLastError();
		if (error_no != WSA_IO_PENDING)
		{
			Error_Display("WSARecv() Error - ", error_no);
		}
	}

}

// ������
void Process_Packet(char id, char* buf)
{
	CS_Packet_Up* packet = reinterpret_cast<CS_Packet_Up*>(buf);
	char x = clients[id].x;
	char y = clients[id].y;

	switch (packet->type)
	{
	case CS_UP:
		--y;
		if (y < 0)
			y = 0;
		break;

	case CS_DOWN:
		++y;
		if (y >= WORLD_HEIGHT)
			y = WORLD_HEIGHT - 1;
		break;

	case CS_LEFT:
		if (x > 0)
			x--;
		break;

	case CS_RIGHT:
		if (x < WORLD_WIDTH - 1)
			x++;
		break;

	default:
		cout << "Unknown Packet Type" << endl;
		while (true);
	}
	clients[id].x = x;
	clients[id].y = y;

	g_Mutex.lock();

	// �̵� �� viewList
	unordered_set<int> old_viewList = clients[id].viewList;

	// �̵� �� viewList
	unordered_set<int> new_viewList;
	for (int i = 0; i < MAX_USER; ++i)
	{
		// i�� �ش��ϴ� Ŭ�� �������ְ�, 
		// ���ϰ�, ����ϰ� ��ó�� �ִ���, 
		// ���� ���ϰ� id�ϰ� ���� ���� ��,
		if (clients[i].connected == true 
			&& Is_Near_Object(id, i) == true 
			&& i != id)
		{
			new_viewList.insert(i);
		}
	}

	// Put Object
	// ���� ��ó�� �ִ� ������Ʈ�鿡 ����
	for (auto client : new_viewList)
	{
		// ���� ������ ���� �����־����
		if (old_viewList.count(client) == 0)
		{
			clients[id].viewList.insert(client);
			Send_Put_Player_Packet(id, client);

			if (clients[client].viewList.count(id) != 0)
			{
				Send_Pos_Packet(client, id);
			}
			else
			{
				clients[client].viewList.insert(id);
				Send_Put_Player_Packet(client, id);
			}
		}

		// old_viewList�� new_viewList�� �ִ� Ŭ��ID�� ���� ��,
		else if (old_viewList.count(client) != 0)
		{
			// viewList�� �ش��ϴ� id�� ������,
			if (clients[client].viewList.count(id) != 0)
			{
				Send_Pos_Packet(client, id);
			}
			// viewList�� �ش��ϴ� id�� ������,
			else
			{
				clients[client].viewList.insert(id);
				Send_Put_Player_Packet(client, id);				
			}
		}
	}
	// �Ⱥ��̴� �÷��̾� ID ����Ʈ
	unordered_set<int> removedIDList;
	for (int i = 0; i < MAX_USER; ++i)
	{
		// i�� �ش��ϴ� Ŭ�� �������ְ�, 
		// ���ϰ�, ����ϰ� ��ó�� ����, 
		// ���� ���ϰ� id�ϰ� ���� ���� ��,
		if (clients[i].connected == true
			&& Is_Near_Object(id, i) == false
			&& i != id)
		{
			removedIDList.insert(i);
		}
	}
	// removedIDList�� ��� ��ü�� ����
	for (int i = 0; i < removedIDList.size(); ++i)
	{
		// �����ؾ��ϴ� ID ����Ʈ���� �ش��ϴ� ID�� ���Ͽ� ã��,
		// �� �丮��Ʈ���� ��븦 ����
		if (removedIDList.count(i) != 0)
		{
			old_viewList.erase(i);
			// ������ ��븦 ������ ����
			Send_Remove_Player_Packet(id, i);

			// ��� viewList�� �� id�� ������,
			if (clients[i].viewList.count(id) != 0)
			{
				// ��� viewlist���� ����
				clients[i].viewList.erase(id);
				// ������� ���� ������ ����
				Send_Remove_Player_Packet(i, id);
			}
		}
	}

	clients[id].viewList = old_viewList;
	g_Mutex.unlock();

	// ������ �������� ��ġ�� ������
	for (int i = 0; i < MAX_USER; ++i)
	{
		if (clients[i].connected == true)
			Send_Pos_Packet(i, id);
	}
}

void 	Send_Login_Ok_Packet(char to)
{
	SC_Packet_Login_OK packet;
	packet.id = to;
	packet.size = sizeof(SC_Packet_Login_OK);
	packet.type = SC_LOGIN_OK;

	Send_Packet(to, reinterpret_cast<char*>(&packet));
}

void 	Send_Put_Player_Packet(char to, char obj)
{
	SC_Packet_Put_Player packet;
	packet.id = obj;
	packet.size = sizeof(SC_Packet_Put_Player);
	packet.type = SC_PUT_PLAYER;
	packet.x = clients[obj].x;
	packet.y = clients[obj].y;

	Send_Packet(to, reinterpret_cast<char*>(&packet));
}

// ��������, ������Ʈ��
void 	Send_Pos_Packet(char to, char obj)
{
	SC_Packet_Pos packet;
	packet.id = obj;
	packet.size = sizeof(SC_Packet_Pos);
	packet.type = SC_POS;
	packet.x = clients[obj].x;
	packet.y = clients[obj].y;

	Send_Packet(to, reinterpret_cast<char*>(&packet));

}
void Send_Remove_Player_Packet(char to, char id)
{
	SC_Packet_Remove_Player packet;
	packet.id = id;
	packet.size = sizeof(SC_Packet_Remove_Player);
	packet.type = SC_REMOVE_PLAYER;

	Send_Packet(to, reinterpret_cast<char*>(&packet));
}

void Send_Packet(char key, char* packet)
{
	SOCKET client_s = clients[key].socket;
	OVER_EX* over = new OVER_EX;

	over->dataBuffer.len = packet[0];
	over->dataBuffer.buf = over->messageBuffer;
	// ��Ŷ�� ������ ���ۿ� ����
	memcpy(over->messageBuffer, packet, packet[0]);

	ZeroMemory(&(over->overlapped), sizeof(WSAOVERLAPPED));
	over->is_recv = false;

	if (WSASend(client_s, &over->dataBuffer, 1, nullptr, 0, &(over->overlapped), nullptr) == SOCKET_ERROR)
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			cout << "Error - Fail WSARecv(error_code : " << WSAGetLastError() << endl;
		}
	}
}

void Disconnect(int id)
{
	g_Mutex.lock();

	for (int i = 0; i < MAX_USER; ++i)
	{
		if (clients[i].connected == false)
			continue;
		if (clients[i].viewList.count(id) != 0)
			Send_Remove_Player_Packet(i, id);
	}
	closesocket(clients[id].socket);
	clients[id].viewList.clear();
	clients[id].connected = false;

	g_Mutex.unlock();
}

// �����̿� �ִ��� �Ÿ��� ����ؼ� �Ǵ���.
// ������ true, �ָ� false
bool Is_Near_Object(int a, int b)
{
	if (VIEW_DISTANCE < abs(clients[a].x - clients[b].x))
		return false;

	if (VIEW_DISTANCE < abs(clients[a].y - clients[b].y))
		return false;

	return true;
}