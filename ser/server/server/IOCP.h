#pragma once
#pragma comment(lib, "Ws2_32.lib")

#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <unordered_set>
#include <winsock2.h>
using namespace std;

#define MAX_BUFFER			1024

#define THREAD_COUNT		4

// �� ���� ũ��
#define WORLD_WIDTH		100
// �� ���� ũ��
#define WORLD_HEIGHT		100
// �÷��̾� ���� ��ġ
#define START_X					4
#define START_Y					4

// �� �� �ִ� �þ� : �þ�ó���� �� ��, �׸�� �� ������, ������ �� ������
// 2D������ �þ߰� �׸�, 3D������ �þ߰� ��
#define VIEW_DISTANCE		2

// Ȯ�� ������Ʈ ����ü
struct OVER_EX
{
	WSAOVERLAPPED	overlapped;
	WSABUF					dataBuffer;
	char							messageBuffer[MAX_BUFFER];
	bool							is_recv;
	bool							is_send;
};

// Ŭ���̾�Ʈ ���� (I/O�� �ϱ����� �ʿ��� ����)
struct SOCKETINFO
{
	////////////////////////////////////////////////////////////////////////////////////
	// [ �ѹ��� �� �����忡���� ���� ]
	// Worker_Thread���� �а� ���⸦ ��. => �׷��� �� �����忡���� �����.
	OVER_EX	over;
	SOCKET	socket;
	// Worker_Thread���� �� �����忡���� �����
	char			packet_buf[MAX_BUFFER];

	////////////////////////////////////////////////////////////////////////////////////
	// [ ���� �����忡�� ���� �ٹ������� �а�, ���⸦ ��. ]
	// ���� mutex�� ����Ͽ� lock�� ����ؾ� ��
	//mutex	access_lock;
	bool			connected;
	int				prev_size;
	// �÷��̾� ������ ��.
	int				sendBytes;
	// ������ �÷��̾��� x, y ��ǥ
	int				x;
	int				y;

	unordered_set<int> viewList;
};

// �������
void Error_Display(const char* msg, int err_no);
// SOCKETINFO �ʱ�ȭ
void Initialize();
void Worker_Thread();
char Get_New_ID();
void Do_Accept();
void Do_Recv(char id);
void Process_Packet(char id, char* buf);
void 	Send_Login_Ok_Packet(char to);
void 	Send_Put_Player_Packet(char to, char obj);
void 	Send_Pos_Packet(char to, char obj);
void Send_Remove_Player_Packet(char to, char id);
void Send_Packet(char key, char* packet);
void Disconnect(int id);
bool Is_Near_Object(int a, int b);
