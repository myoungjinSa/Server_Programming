#pragma once

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

#define MAX_CLIENT_COUNT 1

enum PLAYER_ID { PLAYER_1,PLAYER_2,PLAYER_3,PLAYER_4,PLAYER_5,PLAYER_6 };

#pragma pack(1)

struct CS_RUN
{
	CS_RUN(){}
	CS_RUN(byte t,byte k) : key(t),player(k){}
	byte key;
	byte player;
};

#pragma pack()

struct ClientInfo					
{
	ClientInfo(SOCKET s,byte player) :clientSocket(s),player(player) {}
	SOCKET clientSocket;			// SOCKET 구조체 크기는 64비트 정수 
	byte   player;
};