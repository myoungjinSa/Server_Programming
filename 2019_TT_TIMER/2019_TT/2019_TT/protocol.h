#pragma once

constexpr int MAX_USER = 10;

constexpr int NPC_ID_START = 100;
constexpr int NUM_NPC = 200000;

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



#define SC_LOGIN_OK				1
#define SC_PUT_PLAYER			2
#define SC_REMOVE_PLAYER		3
#define SC_POS					4
#define SC_REQUEST_ID			5
#define SC_DENY_LOGIN			6
#define SC_POS_SAVE_RESULT		7


#define SD_CONNECT				10
#define SD_POSITION_SAVE		11
#define DS_CONNECT_RESULT		12
#define DS_POSITION_SAVE_RESULT 13

enum QUERY_TYPE
{
	DB_CONNECT,
	DB_POSITION_SAVE
};



#pragma pack(push ,1)

struct sc_packet_pos {
	char size;
	char type;
	short x, y;
	short id;
};

struct sc_packet_remove_player {
	char size;
	char type;
	short id;
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
	short id;
};

struct sc_packet_put_player {
	char size;
	char type;
	short x, y;
	short id;
};

struct sc_packet_save_result
{
	char size;
	char type;
	bool isSave;
};

struct cs_packet_connect
{
	char size;
	char type;
	short id;
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

struct cs_packet_pos_save
{
	char     size;
	char     type;
	short     id;
	short    posX;
	short    posY;
};


//server to database
struct sd_packet_connect
{
	char size;
	char type;
	short id;
};

struct sd_packet_pos_save
{
	char size;
	char type;
	
	short  id;
	short pos_x;
	short pos_y;
};
struct ds_packet_connect_result
{
	char size;
	char type;
	bool access;
	short pos_x;
	short pos_y;
};

struct ds_packet_save_result
{
	char size;
	char type;
	bool is_save;
};
#pragma pack (pop)