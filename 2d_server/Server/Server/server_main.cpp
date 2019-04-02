/*
## ���� ���� : 1 v n - overlapped callback
1. socket()            : ���ϻ���
2. bind()            : ���ϼ���
3. listen()            : ���Ŵ�⿭����
4. accept()            : ������
5. read()&write()
WIN recv()&send    : ������ �а���
6. close()
WIN closesocket    : ��������
*/


#include <iostream>
#include <map>
#include <vector>
#include <thread>
#include "Protocol.h"

using namespace std;

#include <winsock2.h>

#pragma comment(lib, "Ws2_32.lib")




#define START_X		4
#define START_Y		4

struct OVERLAPPED_EX
{
	WSAOVERLAPPED overlapped;				//���� �浹���Ͼ�� ������ ���ϸ��� OVERLAPPED ����ü�� �־����.
	WSABUF dataBuffr;
	char messageBuffer[MAX_BUFFER];
	bool is_recv;
};

struct SOCKETINFO				//������ ����
{
	bool	conneceted;				
	OVERLAPPED_EX overlapped;
	SOCKET socket;
	char packet_buf[MAX_BUFFER];		//�������� �� �ȵǾ� ���� ��� �ʿ���
	int prev_size;
	int x, y;

	//POS		position;

	//HP,�÷��̾� ������ ���ʿ� �����.
};

SOCKETINFO clients[MAX_USER];			//���ϸ��� � ���������� ������ ������ �Ǿ����.

HANDLE g_iocp;



void Error_Display(char *msg,int err_no)
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
	wcout << L"����" << lpMsgBuf << std::endl;
	while (true);
	LocalFree(lpMsgBuf);

}

void Initialize()
{
	for(auto& cl : clients)
	{
		cl.conneceted = false;
	}
}

void Network_Initialize()
{

}

char Get_New_Id()
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
void send_packet(int key,char *packet)
{
	SOCKET client_s = clients[key].socket;		//32��Ʈ�̱� ������ �����ε��� 
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

void send_remove_player_packet(char to,char id)
{
	SC_PACKET_REMOVE_PLAYER packet;

	packet.id = id;
	packet.size = sizeof(packet);
	packet.type = SC_REMOVE_PLAYER;

	send_packet(to, reinterpret_cast<char*>(&packet));

}
void send_login_packet(char to)
{
	SC_PACKET_LOGIN_OK packet;

	packet.id = to;
	packet.size = sizeof(packet);
	packet.type = SC_LOGIN_OK;

	send_packet(to, reinterpret_cast<char*>(&packet));
}
void send_put_player_packet(char to, char object)
{
	SC_PACKET_POS packet;

	packet.id = object;
	packet.size = sizeof(packet);
	packet.type = SC_PUTPLAYER;

	packet.x = clients[object].x;
	packet.y = clients[object].y;

	send_packet(to, reinterpret_cast<char*>(&packet));
}
void send_pos_packet(char to, char object)
{
	SC_PACKET_POS packet;

	packet.id = object;
	packet.size = sizeof(packet);
	packet.type = SC_POS;

	packet.x = clients[object].x;
	packet.y = clients[object].y;

	send_packet(to, reinterpret_cast<char*>(&packet));
}

void do_recv(char id)
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

void do_accept()
{
		// Winsock Start - windock.dll �ε�
	WSADATA WSAData;
	if (WSAStartup(MAKEWORD(2, 2), &WSAData) != 0)
	{
		cout << "Error - Can not load 'winsock.dll' file\n";
		return ;
	}

	// 1. ���ϻ���  
	SOCKET listenSocket = WSASocketW(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);		//OVERLAPPED���� �ݵ�� WSA_FLAG_OVERLAPPED �� ����ؾ���.
	if (listenSocket == INVALID_SOCKET)
	{
		cout<<"Error - Invalid socket"<<"\n";
		return ;
	}

	// �������� ��ü����
	SOCKADDR_IN serverAddr;
	memset(&serverAddr, 0, sizeof(SOCKADDR_IN));
	serverAddr.sin_family = PF_INET;
	serverAddr.sin_port = htons(SERVER_PORT);
	serverAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);			//� �ּҰ� ������ �� �ްڴ� - INADDR_ANY

	// 2. ���ϼ���
	if (::bind(listenSocket, (struct sockaddr*)&serverAddr, sizeof(SOCKADDR_IN)) == SOCKET_ERROR)
	{
		printf("Error - Fail bind\n");
		// 6. ��������
		closesocket(listenSocket);
		// Winsock End
		WSACleanup();
		return ;
	}

	// 3. ���Ŵ�⿭����
	if (listen(listenSocket, 5) == SOCKET_ERROR)		//listen(socket, backlog) 
	{													//backlog -> ������ ������� ��⿭ ��
		printf("Error - Fail listen\n");
		
		// 6. ��������
		closesocket(listenSocket);
		
		
		// Winsock End
		WSACleanup();
		return ;
	}

	SOCKADDR_IN clientAddr;
	int addrLen = sizeof(SOCKADDR_IN);
	memset(&clientAddr, 0, addrLen);
	SOCKET clientSocket;
	DWORD flags;

	while (true)
	{
		clientSocket = accept(listenSocket, (struct sockaddr *)&clientAddr, &addrLen);
		if (clientSocket == INVALID_SOCKET)
		{
			cout<<"Error - Accept Failure"<<"\n";
			return ;
		}

		char new_id = Get_New_Id();


	
		memset(&clients[new_id], 0x00, sizeof(struct SOCKETINFO));
		
		clients[new_id].socket = clientSocket;
		
		
		clients[new_id].overlapped.dataBuffr.len = MAX_BUFFER;
		clients[new_id].overlapped.dataBuffr.buf = clients[new_id].packet_buf;
		clients[new_id].overlapped.is_recv = true;
		
		clients[new_id].x = START_X;
		clients[new_id].y = START_Y;
		flags = 0;

		
		CreateIoCompletionPort(reinterpret_cast<HANDLE>(clientSocket), g_iocp, new_id, 0);

		clients[new_id].conneceted = true;


		send_login_packet(new_id);
		for(int i =0;i<MAX_USER;++i)
		{
			if (clients[i].conneceted == false)
			{
				continue;
			}
			send_put_player_packet(i, new_id);
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
			send_put_player_packet(new_id, i);
		}
		do_recv(new_id);

		//ACCEPT ���ڸ��� RECV�� ����, -> 
		//if (WSARecv(clients[new_id].socket, &clients[new_id].overlapped.dataBuffr, 1, NULL, &flags,&(clients[new_id].overlapped) , 0))	//�÷��׿��� NULL���� �ȵ�, -> �÷��װ� 0�ΰ��ƴѰ� �˻縦�ϱ⶧����
		//{
		//	//WSA_IO_PENDING: An overlapped operation was successfully initiated and completion will be indicated at a later time. 
		//	if (WSAGetLastError() != WSA_IO_PENDING)	//IOPENDING������ ���°� ����
		//	{
		//		cout<<"Error - IO pending Failure"<<"\n";
		//		return ;
		//	}
		//}
		//else {
		//	//�����- ���⼭ ó���ϸ� ���α׷����� ����������,���� ���ڰ� NULL�̱⶧���� ����� ���� ����.
		//	cout << "Non Overlapped Recv return.\n";
		//	return ;
		//}
	}

	// 6-2. ���� ��������
	closesocket(listenSocket);

	// Winsock End
	WSACleanup();

	return ;
}



void process_packet(char id, char* buf)
{
	CS_PACKET_UP * packet = reinterpret_cast<CS_PACKET_UP*>(buf);


	char x = clients[id].x;
	char y = clients[id].y;


	switch(packet->type)
	{
	case CS_UP:
		--y;
		if (y < 0) y = 0;
		break;
	case CS_DOWN:
		++y;
		if (y > WORLD_HEIGHT) y = WORLD_HEIGHT;
		break;

	case CS_LEFT:
		x--;
		if (x < 0) x = 0;
		break;
	case CS_RIGHT:
		++x;
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

			send_pos_packet(i, id);
		}
	}


}
void disconnect(int id)
{
	for(int i=0;i<MAX_USER;++i)
	{
		if (clients[i].conneceted == false) 
		{
			send_remove_player_packet(i, id);
		}

	}
	closesocket(clients[id].socket);
	clients[id].conneceted = false;
}

void worker_thread()
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
			disconnect(key);
		}

		if(lpOver_ex->is_recv) 
		{
			int rest_size = io_byte;
			char packet_size = 0;

			//�̹� ���� ������ �������
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
					process_packet(key, clients[key].packet_buf);
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
			do_recv(key);
			
		}
		else 
		{
			//delete
			delete lpOver_ex;


		}


	}
}

int main()
{
	vector<thread> worker_threads;

	wcout.imbue(locale("korean"));		//�ѱ� ���
	_wsetlocale(LC_ALL, L"korean");


	Initialize();
	Network_Initialize();

	g_iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);


	for (int i = 0; i < 4; ++i)
	{
		worker_threads.emplace_back(thread{ worker_thread });
	}
	thread accept_thread{ do_accept };
	
	accept_thread.join();

	for (auto& th : worker_threads)
		th.join();

	CloseHandle(g_iocp);

}


