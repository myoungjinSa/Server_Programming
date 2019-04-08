#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS
//소켓

#pragma comment(lib, "ws2_32.lib")
#include <WinSock2.h>
#include "DirectX.h"

constexpr int MAX_USER = 6;

constexpr int SERVER_PORT = 3500;
constexpr int MAX_BUFFER = 1024;

constexpr char CS_UP = 0x01;
constexpr char CS_DOWN = 0x02;
constexpr char CS_LEFT = 0x03;
constexpr char CS_RIGHT = 0x04;

constexpr char SC_LOGIN_OK = 0x01;
constexpr char SC_PUTPLAYER = 0x02;
constexpr char SC_REMOVE_PLAYER = 0x03;
constexpr char SC_POS = 0x04;

#pragma pack(push,1)


struct PLAYER
{
	bool isBomber;			//플레이어들의 역할
	//XMFLOAT4X4 worldMatrix;	//위치 회전 정보 
	XMFLOAT3 xmf3Position;
	XMFLOAT3 xmf3Right;
	XMFLOAT3 xmf3Look;
	XMFLOAT3 xmf3Up;
	float pitch;			//모델의 회전 (pitch,yaw,roll)
	float yaw;
	float roll;
	XMFLOAT3 velocity;		//속도
	//XMFLOAT3 pos;			//플레이어의 위치
	//XMFLOAT4 dir;			//플레이어들 방향(쿼터니언)
	byte  animationNum;		//애니메이션 번호
	float animationTime;	//애니메이션 시간 정보
	byte  usedItem;			//사용되는 아이템 정보
	byte  playerState;		//플레이어 상태
	DWORD direction;		//방향 정보

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
};

struct SC_PACKET_REMOVE_PLAYER
{
	char size;
	char type;
	char id;
};


struct SC_INGAME_PACKET
{
	char	size;
	char	type;
	byte	id;
	PLAYER	player_Packet;
	byte	roundCount;    //몇 라운드인지
	float	timer;		//게임 시간
	bool	bomb;			//폭탄이 터졌는지

};

#pragma pack(pop)