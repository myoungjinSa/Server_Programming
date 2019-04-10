#pragma once

#include "Global.h"
#include "Protocol.h"
#include "GameTimer.h"

constexpr float START_X = 0.0f;
constexpr float START_Z = 0.0f;



struct OVERLAPPED_EX
{
	WSAOVERLAPPED overlapped;				//서로 충돌이일어나기 때문에 소켓마다 OVERLAPPED 구조체가 있어야함.
	WSABUF dataBuffr;
	char messageBuffer[MAX_BUFFER];
	bool is_recv;
};

struct SOCKETINFO				//소켓의 정보
{
	//mutex   access_lock;
	bool	conneceted;					//여러 쓰레드가 동시에 접근한다. 
	char	packet_buf[MAX_BUFFER];		//재조립이 다 안되어 있을 경우 필요함 // 두개의 worker 쓰레드가 동시에 건드리지 않음
	DWORD    m_dwDirection;			

	OVERLAPPED_EX overlapped;			//어느 한 순간에는 한 쓰레드 하나만 읽고 쓰고 한다. 한번 만들면 바꾸지 않음, //두개의 worker 쓰레드가 동시에 건드리지 않음
	SOCKET	socket;
	
	int		prev_size;
	//int		x, y;
	
	PLAYER  player;						//플레이어 정보

	//서버가 일정시간마다 각 client에게 packet을 전송한다. 
	//1. 서버 => 전송 당시 시간(ex 08시 10분 50초 .10) => 클라이언트
	//2. 클라이언트 => 서버로부터 받은 시간 그대로(08시 10.50초 .10 )=>서버
	//3. result= 클라이언트로부터 packet을 받은 시간(08시 10분 50초 .52) - 최초로 서버가 packet을 보낸 시간(08시 10분 50초 .10)
	//4. frameLatency = reslut * 0.5f; => 서버가 보낸 packet이 클라이언트에게 도착하는 시간을 산출

	//이렇게 서버로부터 각 client까지의 latency Time 을 계산했으면 이 시간을 기준으로 모든 정보를 공유해야 함.
	//1. client 1 : 100ms
	//2. client 2 : 120ms
	//client 1 => 500ms 뒤에 50m 지점에 도착 => 서버
	//이 명령은 100ms전에 전송된 것이다. 그러므로
	// 서버 => client2에게 client1은 280ms(500 - 100 - 120)뒤에 50ms 지점에 도착
	
	
	float	frameLatency;				//서버와 클라이언트 

	XMFLOAT3 xmf3LastRightVector;
	XMFLOAT3 xmf3LastLookVector;
	XMFLOAT3 xmf3LastUpVector;
		
	//POS		position;

	//HP,플레이어 정보는 이쪽에 들어가면됨.
};

typedef struct PhysicsInfo
{
	XMFLOAT3     				m_xmf3Gravity;
	float           			m_fMaxVelocityXZ ;
	float           			m_fMaxVelocityY;
	float           			m_fFriction;

}PHYSICS;

class CAnimation;
class CHeightMapImage;
class CServerFramework
{
private:
	
	//이 구조체는 여러 쓰레드가 읽기,쓰기 동작을 모두 하기 때문에 LOCK이 필요함.
	SOCKETINFO clients[MAX_USER];			//소켓마다 어떤 소켓정보를 쓰는지 매핑이 되어야함.

	SOCKET m_listen_Socket;
	const std::string SERVER_IP = "127.0.0.1";
	
	
	
	PHYSICS m_physicsInfo;				//플레이어들 위치계산에 공통적으로 적용할 변수 집합

	CHeightMapImage*	m_pHeightMap;

	
	std::unique_ptr<CAnimation> m_pAnimationInfo;
public:

	CServerFramework();
	virtual ~CServerFramework();

	void Error_Display(char *msg, int err_no);
	void Initialize();

	char GetNewId();
	void Send_Packet(int key, char *packet);
	void Send_Remove_Player_Packet(char to, char id);
	void Send_Login_Packet(char to);
	void Send_Put_Player_Packet(char to, char object);
	void Send_Pos_Packet(char to, char object);
	void Send_Change_Animation_Packet(char to, char object);
	void Send_Timer_Packet(char to);

	void Network_Initialize();
	void Do_Recv(char id);
	
	void DeleteHeightmapInfo();
	void SetHeightmapInfo(LPCSTR filename, int nWidth, int nLength, XMFLOAT3 xmf3Scale);


	//void Initialize_WorldMatrix(SOCKETINFO& clients);
	void InitializePhysics();
	void SetClient_FirstPosition(SOCKETINFO& clients);
	void SetDirection(SOCKETINFO& clients,UCHAR& key);
	void RotateModel(SOCKETINFO& clients, float x, float y, float z);
	void RotateClientsAxisY(SOCKETINFO& clients,float fTimeElapsed);
	float& UpdateClientPos(SOCKETINFO& clients,float fTimeElapsed,const UCHAR& key);


	const byte& GetClientCurrentAnimation(SOCKETINFO& clients) const { return clients.player.animationNum; }
	
	//클라이언트가 하이트맵 아래로 떨어지는것을 막아주는 처리
	void ProcessClientHeight(SOCKETINFO& clients);

	//마찰력 계산
	float& ProcessFriction(SOCKETINFO& clients,float& fLength);

	void Process_Packet(char id, char* buf);

	void Disconnect(int id);
		
	void Worker_Thread();
	void Do_Accept();

public:
	HANDLE g_iocp;		//g_iocp는 여러 쓰레드가 읽기 동작만 하기 때문에 mutex가 필요없다
	float m_fGameTime;
	CGameTimer m_GameTimer;
	enum KEY
	{
		KEY_IDLE		= 0x00,
		KEY_RIGHT		= 0x01,
		KEY_LEFT		= 0x02,
		KEY_UP			= 0x03,
		KEY_DOWN		= 0x04,
		KEY_CTRL		= 0x05,
		KEY_ALT			= 0x06,
		KEY_SHIFT		= 0x07,
		KEY_SPACE		= 0x08,
		KEY_Z			= 0x09
	};
	enum ITEM { NOHAVE =0,HAMMER,GOLD_HAMMER,GOLD_TIMER,BOMB};
	enum PLAYER_NUM { PLAYER1 ,PLAYER2 ,PLAYER3 ,PLAYER4 ,PLAYER5 ,PLAYER6};
enum PLAYER_STATE { NONE,ICE,BREAK};	//플레이어 상태
	enum STATE_TYPE {Init,Run,Over};
};

