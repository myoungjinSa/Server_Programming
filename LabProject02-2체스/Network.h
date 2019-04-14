#pragma once



constexpr int WM_SOCKET = WM_USER + 1;

//#define KEY_IDLE	0x00
//#define KEY_RIGHT	0x01
//#define KEY_LEFT	0x02
//#define KEY_UP		0x03
//#define KEY_DOWN	0x04


//#define MAXCLIENT  10
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
	//SC_RUN(){}
	//SC_RUN(PlayerInfo p):player(p) {}
	PlayerInfo player[MAX_USER];
	byte playerCount;
	byte index;
	byte xStep;
	byte yStep;
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
class CGameFramework;

class CNetwork 
{
public:
	CNetwork();
	~CNetwork();

	void err_quit(const char *);
	void err_display(const char*);

	int recvn(char *buf, int len, int flags);

	void Initialize(HWND window,CGameFramework* client);
	
	void ReadPacket(SOCKET sock);
	
	void ShutDown();
	void ClientError();
	
	int GetClientNum() { return m_clientCount; }

	void SetClientNum(int num) { m_clientCount = num; }
	enum PLAYER_INFO {PLAYER_1,PLAYER_2,PLAYER_3,PLAYER_4,PLAYER_5,PLAYER_6};

	std::vector<PlayerInfo> m_vPlayer;

	WSABUF m_Send_Wsabuf;
	
	WSABUF m_Recv_Wsabuf;

	char   m_Send_Buffer[MAX_BUFFER];
	char   m_Recv_Buffer[MAX_BUFFER];
	SOCKET m_socket;
private:
	
	SOCKADDR_IN m_serverAddr;

	const std::string SERVERIP = "127.0.0.1";
	const u_short SERVERPORT = 3500;
	CGameFramework* m_gameClient{nullptr};

	

	

	char  m_packet_buffer[MAX_BUFFER];

	
	DWORD m_In_Packet_Size{ 0 };

	int   m_Saved_Packet_Size{ 0 };

	int m_clientCount{ 0 };
	bool m_bSendPacket{ false };
	
};