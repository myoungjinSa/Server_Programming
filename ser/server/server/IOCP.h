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

// 맵 가로 크기
#define WORLD_WIDTH		100
// 맵 세로 크기
#define WORLD_HEIGHT		100
// 플레이어 시작 위치
#define START_X					4
#define START_Y					4

// 볼 수 있는 시야 : 시야처리를 할 때, 네모로 할 것인지, 원으로 할 것인지
// 2D게임은 시야가 네모, 3D게임은 시야가 원
#define VIEW_DISTANCE		2

// 확장 오버렙트 구조체
struct OVER_EX
{
	WSAOVERLAPPED	overlapped;
	WSABUF					dataBuffer;
	char							messageBuffer[MAX_BUFFER];
	bool							is_recv;
	bool							is_send;
};

// 클라이언트 정보 (I/O를 하기위해 필요한 정보)
struct SOCKETINFO
{
	////////////////////////////////////////////////////////////////////////////////////
	// [ 한번에 한 스레드에서만 읽음 ]
	// Worker_Thread에서 읽고 쓰기를 함. => 그러나 한 스레드에서만 사용함.
	OVER_EX	over;
	SOCKET	socket;
	// Worker_Thread에서 한 스레드에서만 사용함
	char			packet_buf[MAX_BUFFER];

	////////////////////////////////////////////////////////////////////////////////////
	// [ 여러 스레드에서 동시 다발적으로 읽고, 쓰기를 함. ]
	// 따라서 mutex를 사용하여 lock을 사용해야 함
	//mutex	access_lock;
	bool			connected;
	int				prev_size;
	// 플레이어 정보가 들어감.
	int				sendBytes;
	// 접속한 플레이어의 x, y 좌표
	int				x;
	int				y;

	unordered_set<int> viewList;
};

// 에러출력
void Error_Display(const char* msg, int err_no);
// SOCKETINFO 초기화
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
