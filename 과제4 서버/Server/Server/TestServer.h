#pragma once

#include <iostream>
#include <map>
#include "Protocol.h"

using namespace std;

#include <winsock2.h>

#pragma comment(lib, "Ws2_32.lib")


struct OVERLAPPED_EX
{
	WSAOVERLAPPED overlapped;				//���� �浹���Ͼ�� ������ ���ϸ��� OVERLAPPED ����ü�� �־����.
	WSABUF dataBuffr;
	char messageBuffer[MAX_BUFFER];
	bool is_recv;
};

struct SOCKETINFO				//������ ����
{
	//mutex   access_lock;
	bool	conneceted;					//���� �����尡 ���ÿ� �����Ѵ�. 

	OVERLAPPED_EX overlapped;			//��� �� �������� �� ������ �ϳ��� �а� ���� �Ѵ�. �ѹ� ����� �ٲ��� ����, //�ΰ��� worker �����尡 ���ÿ� �ǵ帮�� ����
	SOCKET socket;
	char packet_buf[MAX_BUFFER];		//�������� �� �ȵǾ� ���� ��� �ʿ��� // �ΰ��� worker �����尡 ���ÿ� �ǵ帮�� ����
	int prev_size;
	float x, y;

	//POS		position;

	//HP,�÷��̾� ������ ���ʿ� �����.
};


class CServerFramework
{
private:
	
	//�� ����ü�� ���� �����尡 �б�,���� ������ ��� �ϱ� ������ LOCK�� �ʿ���.
	SOCKETINFO clients[MAX_USER];			//���ϸ��� � ���������� ������ ������ �Ǿ����.

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
	void Send_Pos_Packet(char to, char object,char data);

	void Network_Initialize();
	void Do_Recv(char id);
	

	void Process_Packet(char id, char* buf);

	void Disconnect(int id);
		
	void Worker_Thread();
	void Do_Accept();

public:
	HANDLE g_iocp;		//g_iocp�� ���� �����尡 �б� ���۸� �ϱ� ������ mutex�� �ʿ����
	unsigned int m_playerCount;
};

//void Worker_Thread();