#pragma once

//C++11에서는 CONSTEXPR 권장
//constexpr int MAX_USER = 10;



#define MAX_USER			10
#define WORLD_WIDTH			100
#define WORLD_HEIGHT		100

#define MAX_BUFFER        1024
#define SERVER_PORT			3500


#define CS_UP				0x01
#define CS_DOWN				0x02
#define CS_LEFT				0x03
#define CS_RIGHT			0x04

#define SC_LOGIN_OK			0x01
#define SC_PUTPLAYER		0x02
#define SC_REMOVE_PLAYER	0x03
#define SC_POS				0x04


#pragma pack(push,1)
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
	char x,y;
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
	char x, y;
};

#pragma pack(pop)


