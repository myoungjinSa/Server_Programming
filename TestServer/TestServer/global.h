#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#pragma comment(lib,"ws2_32")

#include <iostream>
#include <WinSock2.h>
#include <Windows.h>
#include <vector>

using namespace std;

#define KEY_IDLE	0x00
#define KEY_RIGHT	0x01
#define KEY_LEFT	0x02
#define KEY_UP		0x03
#define KEY_DOWN	0x04



#define MAX_CLIENT_COUNT 1

enum PLAYER_ID { PLAYER_1,PLAYER_2,PLAYER_3,PLAYER_4,PLAYER_5,PLAYER_6 };

#pragma pack(1)

struct CS_RUN
{
	CS_RUN(){}
	CS_RUN(byte t,byte k) 
		: key(t),player(k)
	{}
	byte key;
	byte player;

};

struct SC_RUN
{
	SC_RUN(){}
	SC_RUN(float x,float y) :posX(x),posY(y){}
	float posX;
	float posY;

};


#pragma pack()

typedef struct POSITION
{
	POSITION(float x,float y)
	: x(x),y(y)
	{}
	float x;
	float y;
}POS;

struct ClientInfo					
{
	ClientInfo(SOCKET s,byte player,float x,float y)
		:clientSocket(s),player(player),pos(x,y) {}
	SOCKET clientSocket;			// SOCKET ����ü ũ��� 64��Ʈ ���� 
	byte   player;
	POS   pos;
};