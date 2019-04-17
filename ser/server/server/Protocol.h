#pragma once

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 3500

// �ִ� ������ ��
constexpr int MAX_USER = 10;

// Ű ��ũ��
#define CS_UP			1 
#define CS_DOWN	2 
#define CS_LEFT		3 
#define CS_RIGHT	4

// �÷��̾� �α���
#define SC_LOGIN_OK				1
// �÷��̾ ���ӿ� ��ġ�ض�
#define SC_PUT_PLAYER			2
// ���� �÷��̾�
#define SC_REMOVE_PLAYER	3
// �÷��̾� ��ġ
#define SC_POS						4  

#pragma pack(push, 1)

// Ŭ�� -> ����
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

// ���� -> Ŭ��

struct SC_Packet_Login_OK
{
	char size;
	char type;
	// �α��� ���̵�
	char id;
};
struct SC_Packet_Put_Player
{
	char size;
	char type;
	char id;
	// �÷��̾� ��ǥ
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