#pragma once

#include "Global.h"
#include "Protocol.h"
#include "GameTimer.h"

constexpr float START_X = 0.0f;
constexpr float START_Z = 0.0f;



struct OVERLAPPED_EX
{
	WSAOVERLAPPED overlapped;				//���� �浹���Ͼ�� ������ ���ϸ��� OVERLAPPED ����ü�� �־����.
	WSABUF dataBuffr;
	char messageBuffer[MAX_BUFFER];
	bool is_recv;
};

struct SOCKETINFO				//������ ����
{
	//mutex   access_lock;
	bool	conneceted;					//���� �����尡 ���ÿ� �����Ѵ�. 
	char	packet_buf[MAX_BUFFER];		//�������� �� �ȵǾ� ���� ��� �ʿ��� // �ΰ��� worker �����尡 ���ÿ� �ǵ帮�� ����
	DWORD    m_dwDirection;			

	OVERLAPPED_EX overlapped;			//��� �� �������� �� ������ �ϳ��� �а� ���� �Ѵ�. �ѹ� ����� �ٲ��� ����, //�ΰ��� worker �����尡 ���ÿ� �ǵ帮�� ����
	SOCKET	socket;
	
	int		prev_size;
	//int		x, y;
	
	PLAYER  player;						//�÷��̾� ����

	//������ �����ð����� �� client���� packet�� �����Ѵ�. 
	//1. ���� => ���� ��� �ð�(ex 08�� 10�� 50�� .10) => Ŭ���̾�Ʈ
	//2. Ŭ���̾�Ʈ => �����κ��� ���� �ð� �״��(08�� 10.50�� .10 )=>����
	//3. result= Ŭ���̾�Ʈ�κ��� packet�� ���� �ð�(08�� 10�� 50�� .52) - ���ʷ� ������ packet�� ���� �ð�(08�� 10�� 50�� .10)
	//4. frameLatency = reslut * 0.5f; => ������ ���� packet�� Ŭ���̾�Ʈ���� �����ϴ� �ð��� ����

	//�̷��� �����κ��� �� client������ latency Time �� ��������� �� �ð��� �������� ��� ������ �����ؾ� ��.
	//1. client 1 : 100ms
	//2. client 2 : 120ms
	//client 1 => 500ms �ڿ� 50m ������ ���� => ����
	//�� ����� 100ms���� ���۵� ���̴�. �׷��Ƿ�
	// ���� => client2���� client1�� 280ms(500 - 100 - 120)�ڿ� 50ms ������ ����
	
	
	float	frameLatency;				//������ Ŭ���̾�Ʈ 

	XMFLOAT3 xmf3LastRightVector;
	XMFLOAT3 xmf3LastLookVector;
	XMFLOAT3 xmf3LastUpVector;
		
	//POS		position;

	//HP,�÷��̾� ������ ���ʿ� �����.
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
	
	//�� ����ü�� ���� �����尡 �б�,���� ������ ��� �ϱ� ������ LOCK�� �ʿ���.
	SOCKETINFO clients[MAX_USER];			//���ϸ��� � ���������� ������ ������ �Ǿ����.

	SOCKET m_listen_Socket;
	const std::string SERVER_IP = "127.0.0.1";
	
	
	
	PHYSICS m_physicsInfo;				//�÷��̾�� ��ġ��꿡 ���������� ������ ���� ����

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
	
	//Ŭ���̾�Ʈ�� ����Ʈ�� �Ʒ��� �������°��� �����ִ� ó��
	void ProcessClientHeight(SOCKETINFO& clients);

	//������ ���
	float& ProcessFriction(SOCKETINFO& clients,float& fLength);

	void Process_Packet(char id, char* buf);

	void Disconnect(int id);
		
	void Worker_Thread();
	void Do_Accept();

public:
	HANDLE g_iocp;		//g_iocp�� ���� �����尡 �б� ���۸� �ϱ� ������ mutex�� �ʿ����
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
enum PLAYER_STATE { NONE,ICE,BREAK};	//�÷��̾� ����
	enum STATE_TYPE {Init,Run,Over};
};

