#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS
//����

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
	bool isBomber;			//�÷��̾���� ����
	//XMFLOAT4X4 worldMatrix;	//��ġ ȸ�� ���� 
	XMFLOAT3 xmf3Position;
	XMFLOAT3 xmf3Right;
	XMFLOAT3 xmf3Look;
	XMFLOAT3 xmf3Up;
	float pitch;			//���� ȸ�� (pitch,yaw,roll)
	float yaw;
	float roll;
	XMFLOAT3 velocity;		//�ӵ�
	//XMFLOAT3 pos;			//�÷��̾��� ��ġ
	//XMFLOAT4 dir;			//�÷��̾�� ����(���ʹϾ�)
	byte  animationNum;		//�ִϸ��̼� ��ȣ
	float animationTime;	//�ִϸ��̼� �ð� ����
	byte  usedItem;			//���Ǵ� ������ ����
	byte  playerState;		//�÷��̾� ����
	DWORD direction;		//���� ����

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
	byte	roundCount;    //�� ��������
	float	timer;		//���� �ð�
	bool	bomb;			//��ź�� ��������

};

#pragma pack(pop)