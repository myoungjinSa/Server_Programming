#pragma once

#include <iostream>
#include <map>
#include <unordered_set>
#include <mutex>
#include "Protocol.h"

#define VIEW_RADIUS		3  //서로 3칸안에 있으면 보이는것

using namespace std;

#include <winsock2.h>

#pragma comment(lib, "Ws2_32.lib")


struct OVERLAPPED_EX
{
	WSAOVERLAPPED overlapped;				//서로 충돌이일어나기 때문에 소켓마다 OVERLAPPED 구조체가 있어야함.
	WSABUF dataBuffr;
	char messageBuffer[MAX_BUFFER];
	bool is_recv;
};

struct SOCKETINFO				//소켓의 정보
{
	//mutex   access_lock;
	
	OVERLAPPED_EX overlapped;			//어느 한 순간에는 한 쓰레드 하나만 읽고 쓰고 한다. 한번 만들면 바꾸지 않음, //두개의 worker 쓰레드가 동시에 건드리지 않음
	SOCKET socket;
	char packet_buf[MAX_BUFFER];		//재조립이 다 안되어 있을 경우 필요함 // 두개의 worker 쓰레드가 동시에 건드리지 않음
	bool	connected;					//여러 쓰레드가 동시에 접근한다. 
	
	int prev_size;
	float x, y;

	unordered_set<int> viewList;
	//POS		position;

	//HP,플레이어 정보는 이쪽에 들어가면됨.
};


class CServerFramework
{
private:
	
	//이 구조체는 여러 쓰레드가 읽기,쓰기 동작을 모두 하기 때문에 LOCK이 필요함.
	SOCKETINFO clients[MAX_USER];			//소켓마다 어떤 소켓정보를 쓰는지 매핑이 되어야함.

	SOCKET m_listen_Socket;

public:

	CServerFramework();
	virtual ~CServerFramework();

	void Error_Display(char *msg, int err_no);
	void Initialize();

	char GetNewId();
	void Send_Packet(int key, char *packet);
	void Send_Remove_Player_Packet(char to, char id);
	void Send_Login_Packet(char to);
	void Send_Put_Player_Packet(char to, char object);
	void Send_Pos_Packet(char to, char object);

	void Network_Initialize();
	void Do_Recv(char id);
	
	bool Is_Near_Object(int a, int b);

	void Process_Packet(char id, char* buf);

	void Disconnect(int id);
		
	void Worker_Thread();
	void Do_Accept();

public:
	HANDLE g_iocp;		//g_iocp는 여러 쓰레드가 읽기 동작만 하기 때문에 mutex가 필요없다
	unsigned int m_playerCount;
	
};

//void Worker_Thread();