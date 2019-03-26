#pragma once

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment(lib,"ws2_32")


#include<winSock2.h>



#define KEY_IDLE	0x00
#define KEY_RIGHT	0x01
#define KEY_LEFT	0x02
#define KEY_UP		0x03
#define KEY_DOWN	0x04


#define MAXCLIENT  10
#pragma pack(1)

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
	byte  player_id;				//플레이어 정보
	POS position;
	
};

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
	//SC_RUN(){}
	//SC_RUN(PlayerInfo p):player(p) {}
	PlayerInfo player[MAXCLIENT];
	byte playerCount;
	byte index;
	
};

#pragma pack()

struct SOCKETINFO
{
	WSAOVERLAPPED overlapped;
	WSABUF dataBuffer;
	int receiveBytes;
	int sendBytes;

	CS_RUN cs_run;
	SC_RUN sc_run;
};

class CNetwork 
{
public:
	CNetwork();
	~CNetwork();

	void err_quit(const char *);
	void err_display(const char*);

	int recvn(char *buf, int len, int flags);

	void Initialize();
	void Destroy();
	void SendPacket();
	void RecvPacket();

	const CS_RUN& getCSRunPacket() { return m_csRunPacket; }
	const SC_RUN& getSCRunPacket() { return m_scRunPacket; }

	void SetCSRunPacket(CS_RUN& cs_packet) { m_csRunPacket = cs_packet; }

	int GetClientNum() { return m_clientCount; }

	enum PLAYER_INFO {PLAYER_1,PLAYER_2,PLAYER_3,PLAYER_4,PLAYER_5,PLAYER_6};

	std::vector<PlayerInfo> m_vPlayer;
private:
	SOCKET m_socket;
	SOCKADDR_IN m_serverAddr;

	const std::string SERVERIP = "127.0.0.1";
	const u_short SERVERPORT = 3500;

	CS_RUN m_csRunPacket;
	SC_RUN m_scRunPacket;

	int m_clientCount{ 0 };
	bool m_bSendPacket{ false };
	
};