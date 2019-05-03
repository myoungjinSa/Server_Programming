#pragma once


#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "Ws2_32.lib")


#include <iostream>
#include <map>
#include <queue>
#include <unordered_set>
#include <mutex>


#include "Protocol.h"

#define VIEW_RADIUS		3  //���� 3ĭ�ȿ� ������ ���̴°�
#define MOVE_RADIUS		7  //NPC�� 4ĭ �ȿ� ������ ����������.

using namespace std;

#include <winsock2.h>



enum class COMMAND
{
	SEND =0 ,
	RECV ,
	MOVE
};
struct OVERLAPPED_EX
{
	WSAOVERLAPPED overlapped;				//���� �浹���Ͼ�� ������ ���ϸ��� OVERLAPPED ����ü�� �־����.
	WSABUF dataBuffr;
	char messageBuffer[MAX_BUFFER];
	COMMAND command;
};

struct SOCKETINFO				//������ ����
{
	//mutex   access_lock;
	
	OVERLAPPED_EX overlapped;			//��� �� �������� �� ������ �ϳ��� �а� ���� �Ѵ�. �ѹ� ����� �ٲ��� ����, //�ΰ��� worker �����尡 ���ÿ� �ǵ帮�� ����
	SOCKET socket;
	char packet_buf[MAX_BUFFER];		//�������� �� �ȵǾ� ���� ��� �ʿ��� // �ΰ��� worker �����尡 ���ÿ� �ǵ帮�� ����
	bool	connected;					//���� �����尡 ���ÿ� �����Ѵ�. 
	
	int prev_size;
	float x, y;
	
	//mutex view_mutex;
	unordered_set<int> viewList;
	unordered_set<int> npcViewList;
	//POS		position;

	//HP,�÷��̾� ������ ���ʿ� �����.
};



class NPC
{
	int x, y;
	double timer;
};

class USER : public NPC
{
	SOCKETINFO socket;

};

	

class CServerFramework
{
private:
	
	//�� ����ü�� ���� �����尡 �б�,���� ������ ��� �ϱ� ������ LOCK�� �ʿ���.

		//���ϸ��� � ���������� ������ ������ �Ǿ����.
	
	//SOCKETINFO clients[MAX_USER];			//���ϸ��� � ���������� ������ ������ �Ǿ����.
	//NPC  npcs[NUM_NPC];
	
//bool CServerFramework::Is_Near_Object_npc(int a, int b)
//{
//	if (VIEW_RADIUS < abs(npcs[a].x - npcs[b].x))
//		return false;
//	if (VIEW_RADIUS < abs(npcs[a].y - npcs[b].y))
//		return false;
//	return true;
//}

	//�Ʒ��� ���� ������ ���꼺�� ����

	//NPC* clients[NPC_ID_START + NUM_NPC];

//	bool CServerFramework::Is_Near_Object(int a, int b)
//{
	//	if (VIEW_RADIUS < abs(clients[a]->x - clients[b]->x))
	//		return false;
	//	if (VIEW_RADIUS < abs(clients[a]->y - clients[b]->y))
	//		return false;
	//	return true;
//}

	SOCKET m_listen_Socket;
	
public:

	CServerFramework();
	virtual ~CServerFramework();

	void Error_Display(char *msg, int err_no);
	void Initialize();

	int GetNewId();
	void Send_Packet(int key, char *packet);
	void Send_Remove_Player_Packet(int to, int id);
	void Send_Login_Packet(int to);
	void Send_Put_Player_Packet(int to, int object);
	void Send_Pos_Packet(int to, int object);
	void Send_Npc_Put_Packet(int to,int object);
	void Send_Npc_Pos_Packet(int to, int object);
	void Send_Npc_Remove_Packet(int to, int object);


	void Network_Initialize();
	void Do_Recv(int id);
	
	bool Is_Near_Object(int a, int b);
	bool Is_Move_Npc(int a, int b);

	void Process_Packet(int id, char* buf);

	void Disconnect(int id);
	
	void Move_NPC(int npc);
	void Add_Timer(int id,COMMAND c, int sleepTime);
	void Timer_Thread();
	void Worker_Thread();
	void Do_Accept();

public:
	HANDLE g_iocp;		//g_iocp�� ���� �����尡 �б� ���۸� �ϱ� ������ mutex�� �ʿ����
	unsigned int m_playerCount;
	priority_queue<int,vector<int>> pq;
};

//void Worker_Thread();