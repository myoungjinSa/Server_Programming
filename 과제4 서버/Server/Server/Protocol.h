#pragma once

//C++11에서는 CONSTEXPR 권장
//constexpr int MAX_USER = 10;
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment(lib,"ws2_32")


#include<winSock2.h>



#define MAX_USER			10
#define WORLD_WIDTH			800
#define WORLD_HEIGHT		800

#define MAX_BUFFER        1024
#define SERVER_PORT			3500



constexpr char CS_UP = 0x01;
constexpr char CS_DOWN = 0x02;
constexpr char CS_LEFT = 0x03;
constexpr char CS_RIGHT = 0x04;
constexpr char CS_IDLE = 0x05;


constexpr char SC_LOGIN_OK = 0x01;
constexpr char SC_PUTPLAYER = 0x02;
constexpr char SC_REMOVE_PLAYER = 0x03;
constexpr char SC_POS = 0x04;
constexpr char SC_NPC_PUT_PLAYER = 0x05;
constexpr char SC_NPC_POS = 0x06;
constexpr char SC_NPC_REMOVE = 0x07;


constexpr int NPC_ID_START = 100;
constexpr int NUM_NPC = 200'000;


constexpr int WORLDX = 100;
constexpr int WORLDY = 100;

constexpr float fStartX = 10;
constexpr float fStartY = 6;
#pragma pack(push,1)

typedef struct POSITION
{
	POSITION(){}
	POSITION(float x,float y)
	: x(x),y(y)
	{}
	float x;
	float y;
}POS;

struct PlayerInfo
{
	PlayerInfo() {};
	PlayerInfo(byte p,float x,float y) : player_id(p),position(x,y){}
	byte  player_id;				//플레이어 정보
	POS position;
	
};


struct CS_PACKET_UP
{
	unsigned char size;
	unsigned char type;
};

struct CS_PACKET_DOWN
{
	unsigned char size;
	unsigned char type;
};
struct CS_PACKET_LEFT
{
	unsigned char size;
	unsigned char type;
};
struct CS_PACKET_RIGHT
{
	unsigned char size;
	unsigned char type;
};


struct SC_PACKET_LOGIN_OK
{
	char size;
	char type;
	int id;
};

struct SC_PACKET_PUT_PLAYER
{
	char size;
	char type;
	int id;
	char playerCount;
	short x,y;
};

struct SC_PACKET_REMOVE_PLAYER
{
	char size;
	char type;
	int id;
};

struct SC_PACKET_POS
{
	char size;
	char type;
	int id;
	char dir;
	short x, y;
};

struct SC_PACKET_NPC_PUT_PLAYER
{
	char size;
	char type;
	int id;
	short x, y;
};
struct SC_PACKET_NPC_POS
{
	char size;
	char type;
	int id;
	short x, y;
};

struct SC_PACKET_NPC_REMOVE
{
	char size;
	char type;
	int id;
};
#pragma pack(pop)


