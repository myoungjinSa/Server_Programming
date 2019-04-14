#pragma once

//C++11������ CONSTEXPR ����
//constexpr int MAX_USER = 10;
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment(lib,"ws2_32")


#include<winSock2.h>



#define MAX_USER			10
#define WORLD_WIDTH			15000
#define WORLD_HEIGHT		10000

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

constexpr int FRAME_WIDTH = 1200;
constexpr int FRAME_HEIGHT = 800;

constexpr int fWidthStep = FRAME_WIDTH / 8;
constexpr int fHeightStep = FRAME_HEIGHT / 8;

constexpr int WORLDX = 100;
constexpr int WORLDY = 100;

constexpr float fStartX = ( FRAME_WIDTH / 8 ) * 4;
constexpr float fStartY = (FRAME_HEIGHT / 8) * 4;
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
	byte  player_id;				//�÷��̾� ����
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
	char id;
};

struct SC_PACKET_PUT_PLAYER
{
	char size;
	char type;
	char id;
	char playerCount;
	short x,y;
};

struct SC_PACKET_REMOVE_PLAYER
{
	char size;
	char type;
	char id;
};

struct SC_PACKET_POS
{
	char size;
	char type;
	char id;
	char dir;
	short x, y;
};

#pragma pack(pop)


