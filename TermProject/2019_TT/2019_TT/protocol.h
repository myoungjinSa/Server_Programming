#pragma once

constexpr int MAX_USER = 1000;
constexpr int MAX_STR_LENGTH = 50;
constexpr int NPC_ID_START = MAX_USER;
constexpr int NUM_NPC = 200000;
constexpr int ITEM_COUNT = 200;
constexpr int NUM_ITEM = NPC_ID_START +NUM_NPC + ITEM_COUNT;
constexpr int BOSS_COUNT = 1;
constexpr int BOSS_ID = NUM_ITEM + BOSS_COUNT;

constexpr char KEY_A = 0x41;
constexpr char KEY_B = 0x42;
constexpr char KEY_C = 0x43;
constexpr char KEY_X = 0x58;
constexpr char KEY_Z = 0x5A;

#define WORLD_WIDTH		800
#define WORLD_HEIGHT	800

#define SERVER_PORT		3500
#define DB_PORT			3501

#define CS_UP					1	
#define CS_DOWN					2
#define CS_LEFT					3
#define CS_RIGHT				4
#define CS_REQUEST_CONNECT		5
#define CS_REQUEST_POS_SAVE		6
#define CS_USE_HEAL_ITEM		7
#define CS_USE_SKILL_ITEM		8
#define CS_USE_SPEED_ITEM		9
#define CS_ATTACK				10


#define SC_LOGIN_OK				1
#define SC_PUT_PLAYER			2
#define SC_REMOVE_PLAYER		3
#define SC_POS					4
#define SC_REQUEST_ID			5
#define SC_DENY_LOGIN			6
#define SC_POS_SAVE_RESULT		7
#define SC_HP					8
#define SC_MP					9
#define SC_SPEED				10
#define SC_PUT_ITEM				11
#define SC_ITEM_EAT				12
#define SC_REMOVE_ITEM			13
#define SC_DEAD					14
#define SC_CHAT					15
#define SC_ATTACK_RESULT		16


#define SD_CONNECT				20
#define SD_POSITION_SAVE		21
#define DS_CONNECT_RESULT		22
#define DS_POSITION_SAVE_RESULT 23

enum QUERY_TYPE
{
	DB_CONNECT,
	DB_POSITION_SAVE
};



#pragma pack(push ,1)

struct sc_packet_pos {
	char size;
	char type;
	unsigned char x, y;
	unsigned int id;
};
struct sc_packet_chat
{
	char size;
	char type;
	int	 id;
	wchar_t message[MAX_STR_LENGTH];
};

struct sc_packet_item_put
{
	char size;
	char type;
	char kind;
	char padding;
	unsigned short x;
	unsigned short y;
	unsigned int id;

};

struct sc_packet_item_pos
{
	char size;
	char type;
	char kind;
	char padding;
	unsigned short x;
	unsigned short y;
	unsigned int id;
};

struct sc_packet_item_eat
{
	char size;
	char type;
	char kind;
	char padding;
	unsigned int id;
};
struct sc_packet_remove_item
{
	char size;
	char type;
	char padding1;
	char padding2;
	unsigned int id;
};
struct sc_packet_remove_player {
	char size;
	char type;
	char padding1;
	char padding2;
	unsigned int id;
};

struct sc_packet_dead
{
	char size;
	char type;
	char padding1;
	char padding2;
	unsigned int id;
};
struct sc_packet_request_id
{
	char size;
	char type;
};

struct sc_packet_deny_login
{
	char size;
	char type;
};
struct sc_packet_login_ok {
	char size;
	char type;
	unsigned short id;
};

struct sc_packet_hp
{
	char size;
	char type;
	unsigned char hp;
	char padding;
	unsigned int id;

};
struct sc_packet_mp
{
	char size;
	char type;
	unsigned char mp;
	char padding;
	unsigned int id;
};

struct sc_packet_speed
{
	char size;
	char type;
	char speed;
	char padding;
	unsigned int id;
};
struct sc_packet_put_player {
	char size;
	char type;
	unsigned char x, y;
	unsigned int id;
	unsigned char hp;
	unsigned char mp;
	unsigned char power;
};

struct sc_packet_save_result
{
	char size;
	char type;
	bool isSave;
};

struct sc_packet_attack_result
{
	char size;
	char type;
	unsigned short hp;
	unsigned int id;
	unsigned int npc_id;
	bool isHit;

};

struct cs_packet_connect
{
	char size;
	char type;
	unsigned short id;
};

struct cs_packet_up {
	char	size;
	char	type;
};

struct cs_packet_down {
	char	size;
	char	type;
};

struct cs_packet_left {
	char	size;
	char	type;
};

struct cs_packet_right {
	char	size;
	char	type;
};

struct cs_packet_attack
{
	char size;
	char type;
	unsigned short id;
	int power;
};


struct cs_packet_pos_save
{
	char     size;
	char     type;
	unsigned short     id;
	unsigned char    posX;
	unsigned char    posY;
};

struct cs_packet_use_item_heal
{
	char size;
	char type;
	unsigned short id;

};

struct cs_packet_use_item_skill
{
	char size;
	char type;
	unsigned short id;
};

struct cs_packet_use_item_speed
{
	char size;
	char type;
	unsigned short id;
};
//server to database
struct sd_packet_connect
{
	char size;
	char type;
	unsigned short id;
};

struct sd_packet_pos_save
{
	char size;
	char type;
	
	unsigned short  id;
	unsigned char pos_x;
	unsigned char pos_y;
	unsigned char hp;
	unsigned char mp;
};
struct ds_packet_connect_result
{
	char size;
	char type;
	unsigned char pos_x;
	unsigned char pos_y;
	unsigned char hp;
	unsigned char mp;
	bool access;
	
};

struct ds_packet_save_result
{
	char size;
	char type;
	bool is_save;
};
#pragma pack (pop)