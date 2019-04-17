#pragma once

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 3500

// 최대 동접자 수
constexpr int MAX_USER = 10;

// 키 매크로
#define CS_UP			1 
#define CS_DOWN	2 
#define CS_LEFT		3 
#define CS_RIGHT	4

// 플레이어 로그인
#define SC_LOGIN_OK				1
// 플레이어를 게임에 배치해라
#define SC_PUT_PLAYER			2
// 나간 플레이어
#define SC_REMOVE_PLAYER	3
// 플레이어 위치
#define SC_POS						4  

#pragma pack(push, 1)

// 클라 -> 서버
struct CS_Packet_Up
{
	char	size;
	char type;
};
struct CS_Packet_Down
{
	char	size;
	char type;
};
struct CS_Packet_Left
{
	char	size;
	char type;
};
struct CS_Packet_Right
{
	char	size;
	char type;
};

// 서버 -> 클라

struct SC_Packet_Login_OK
{
	char size;
	char type;
	// 로그인 아이디
	char id;
};
struct SC_Packet_Put_Player
{
	char size;
	char type;
	char id;
	// 플레이어 좌표
	char x, y;
};
struct SC_Packet_Remove_Player
{
	char size;
	char type;
	char id;
};
struct SC_Packet_Pos
{
	char size;
	char type;
	char id;
	char x, y;
};

#pragma pack(pop)