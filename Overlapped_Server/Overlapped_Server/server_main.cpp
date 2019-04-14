/*
## ���� ���� : 1 v n - overlapped callback
1. socket()            : ���ϻ���
2. bind()            : ���ϼ���
3. listen()            : ���Ŵ�⿭����
4. accept()            : ������
5. read()&write()
WIN recv()&send    : ������ �а���
6. close()
WIN closesocket    : ��������
*/


#include <iostream>
#include <map>
#include <vector>

using namespace std;

#include <winsock2.h>

#pragma comment(lib, "Ws2_32.lib")

#define MAX_BUFFER        1024
#define SERVER_PORT       3500

#define CS_UP				0x01
#define CS_DOWN				0x02
#define CS_LEFT				0x03
#define CS_RIGHT			0x04
#define CS_IDLE				0x05

#define FRAME_WIDTH  1200
#define FRAME_HEIGHT 800

#define MAXCLIENT  10

enum PLAYER_INFO {PLAYER_1,PLAYER_2,PLAYER_3,PLAYER_4,PLAYER_5,PLAYER_6};
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
	PlayerInfo(byte p,float x,float y) : player(p),position(x,y){}
	byte  player;				//�÷��̾� ����
	POS position;
	
};
struct CS_RUN
{
	
	CS_RUN(){}
	CS_RUN(byte t, byte k)
		: key(t),player(k)
	{
	}
	byte key;
	byte player;
};

struct SC_RUN
{
	
	/*SC_RUN(){}
	SC_RUN(PlayerInfo p):player(p) {}*/
	PlayerInfo player[MAXCLIENT];
	byte playerCount;
	byte index;
	byte xStep;
	byte yStep;
};


#pragma pack()


struct SOCKETINFO				//������ ����
{
	WSAOVERLAPPED overlapped;				//���� �浹���Ͼ�� ������ ���ϸ��� OVERLAPPED ����ü�� �־����.
	WSABUF dataBuffer;						//�����츦 ����ϰ� �����Ƿ� WSABUF �� �ʿ���.
	SOCKET socket;
	int receiveBytes;
	int sendBytes;

	CS_RUN cs_run;
	SC_RUN sc_run;
	
	PlayerInfo	player_Info;
	//POS		position;

	//HP,�÷��̾� ������ ���ʿ� �����.
};

map <SOCKET, SOCKETINFO> clientsMap;			//���ϸ��� � ���������� ������ ������ �Ǿ����.
std::vector<PlayerInfo> m_vPlayer;

//Ÿ���� ������ ����. 
void CALLBACK recv_callback(DWORD Error, DWORD dataBytes, LPWSAOVERLAPPED overlapped, DWORD lnFlags);
void CALLBACK send_callback(DWORD Error, DWORD dataBytes, LPWSAOVERLAPPED overlapped, DWORD lnFlags);

const int fStartX = 7500;
const int fStartY = 7500;

	//Ŭ���̾�Ʈ�� ������ ����
const int fWidthStep = FRAME_WIDTH / 8;
const int fHeightStep = FRAME_HEIGHT / 8;

// �� ������ Ŭ���̾�Ʈ ����
int clientCount = 0;
int main()
{
	// Winsock Start - windock.dll �ε�
	WSADATA WSAData;
	if (WSAStartup(MAKEWORD(2, 2), &WSAData) != 0)
	{
		cout << "Error - Can not load 'winsock.dll' file\n";
		return 1;
	}

	// 1. ���ϻ���  
	SOCKET listenSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);		//OVERLAPPED���� �ݵ�� WSA_FLAG_OVERLAPPED �� ����ؾ���.
	if (listenSocket == INVALID_SOCKET)
	{
		printf("Error - Invalid socket\n");
		return 1;
	}

	// �������� ��ü����
	SOCKADDR_IN serverAddr;
	memset(&serverAddr, 0, sizeof(SOCKADDR_IN));
	serverAddr.sin_family = PF_INET;
	serverAddr.sin_port = htons(SERVER_PORT);
	serverAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);			//� �ּҰ� ������ �� �ްڴ� - INADDR_ANY

	// 2. ���ϼ���
	if (bind(listenSocket, (struct sockaddr*)&serverAddr, sizeof(SOCKADDR_IN)) == SOCKET_ERROR)
	{
		printf("Error - Fail bind\n");
		// 6. ��������
		closesocket(listenSocket);
		// Winsock End
		WSACleanup();
		return 1;
	}

	// 3. ���Ŵ�⿭����
	if (listen(listenSocket, 5) == SOCKET_ERROR)		//listen(socket, backlog) 
	{													//backlog -> ������ ������� ��⿭ ��
		printf("Error - Fail listen\n");
		
		// 6. ��������
		closesocket(listenSocket);
		
		
		// Winsock End
		WSACleanup();
		return 1;
	}

	SOCKADDR_IN clientAddr;
	int addrLen = sizeof(SOCKADDR_IN);
	memset(&clientAddr, 0, addrLen);
	SOCKET clientSocket;
	DWORD flags;

	while (1)
	{
		clientSocket = accept(listenSocket, (struct sockaddr *)&clientAddr, &addrLen);
		if (clientSocket == INVALID_SOCKET)
		{
			printf("Error - Accept Failure\n");
			return 1;
		}

		clientsMap[clientSocket] = SOCKETINFO{};			//�� SOCKET�� Ű������ �ϴ� �� �����̳ʿ� ���� ���� �ʱ�ȭ
		memset(&clientsMap[clientSocket], 0x00, sizeof(struct SOCKETINFO));
		
		clientsMap[clientSocket].socket = clientSocket;
		
		clientsMap[clientSocket].player_Info.position.x = fStartX;//fWidthStep * 1.0f +(fWidthStep * clientCount);
		clientsMap[clientSocket].player_Info.position.y = fStartY;//fHeightStep * 1.0f + (fHeightStep * clientCount);
		//���Կ����ڰ� �������������ں��� ������.
		clientsMap[clientSocket].player_Info.player = clientCount;
		clientsMap[clientSocket].sc_run.index = clientCount;
		//clientsMap[clientSocket].cs_run.player = clientCount;
		clientsMap[clientSocket].dataBuffer.len = sizeof(CS_RUN);
		clientsMap[clientSocket].dataBuffer.buf =(char*)&clientsMap[clientSocket].cs_run.key;
		flags = 0;

		
	
		m_vPlayer.emplace_back(clientsMap[clientSocket].player_Info);
		clientCount++;
		// ��ø ��Ĺ�� �����ϰ� �Ϸ�� ����� �Լ��� �Ѱ��ش�. - �̺�Ʈ�� ������� �����Ŷ� 0�� �־��൵ ��
		clientsMap[clientSocket].overlapped.hEvent = (HANDLE)clientsMap[clientSocket].socket;
		

		//ACCEPT ���ڸ��� RECV�� ����, -> 
		if (WSARecv(clientsMap[clientSocket].socket, &clientsMap[clientSocket].dataBuffer, 1, &clientsMap[clientSocket].dataBuffer.len, &flags, &(clientsMap[clientSocket].overlapped), recv_callback))	//�÷��׿��� NULL���� �ȵ�, -> �÷��װ� 0�ΰ��ƴѰ� �˻縦�ϱ⶧����
		{
			//WSA_IO_PENDING: An overlapped operation was successfully initiated and completion will be indicated at a later time. 
			if (WSAGetLastError() != WSA_IO_PENDING)	//IOPENDING������ ���°� ����
			{
				printf("Error - IO pending Failure\n");
				return 1;
			}
		}
		else {
			//�����- ���⼭ ó���ϸ� ���α׷����� ����������,���� ���ڰ� NULL�̱⶧���� ����� ���� ����.
			cout << "Non Overlapped Recv return.\n";
			return 1;
		}
	}

	// 6-2. ���� ��������
	closesocket(listenSocket);

	// Winsock End
	WSACleanup();

	return 0;
}

//�ݹ��Լ��� � ���Ͽ��� �°����� ��ü������ �˼��־����.
//���� �Լ������� RECV�� �ϸ� �ȵ�. �̹� ���ۿ� �����Ͱ� �°��̹Ƿ�
void CALLBACK recv_callback(DWORD Error, DWORD dataBytes, LPWSAOVERLAPPED overlapped, DWORD lnFlags)
{
	SOCKET client_s = reinterpret_cast<int>(overlapped->hEvent);		//32��Ʈ�̱� ������ �����ε��� 

	
	DWORD sendBytes = 0;
	DWORD receiveBytes = 0;
	DWORD flags = 0;

	if (dataBytes == 0)						//Client�� closeSocketȣ�����ϸ� dataBytes �� 0 �� ä�� recv�� ȣ��ȴ�. 
	{										//�׷��Ƿ� ������ �ش� Ŭ���̾�Ʈ ������ closeSocketó�����ָ��.
		closesocket(clientsMap[client_s].socket);
		
		clientCount--;
		cout << clientCount << endl;

		m_vPlayer.clear();
		
		return;
	}  // Ŭ���̾�Ʈ�� closesocket�� ���� ���

	DWORD dir = CS_IDLE;
	switch(clientsMap[client_s].cs_run.key)
	{
	case CS_IDLE:
		//cout << "IDLE" << endl;
		
		break;
	case CS_UP:
		m_vPlayer[clientsMap[client_s].cs_run.player].position.y = -fHeightStep;
		clientsMap[client_s].sc_run.yStep = fHeightStep;
		clientsMap[client_s].sc_run.xStep = 0.0f;
		break;
	case CS_DOWN:
		m_vPlayer[clientsMap[client_s].cs_run.player].position.y = fHeightStep;
		clientsMap[client_s].sc_run.yStep = -fHeightStep;
		clientsMap[client_s].sc_run.xStep = 0.0f;
		break;
	case CS_RIGHT:
		m_vPlayer[clientsMap[client_s].cs_run.player].position.x = fWidthStep;
		clientsMap[client_s].sc_run.xStep = fWidthStep;
		clientsMap[client_s].sc_run.yStep = 0.0f;
		break;
	case CS_LEFT:
		m_vPlayer[clientsMap[client_s].cs_run.player].position.x = -fWidthStep;
		clientsMap[client_s].sc_run.xStep = -fWidthStep;
		clientsMap[client_s].sc_run.yStep = 0.0f;
		
		break;
	default:
		break;
	}

	

	for(int i=0;i<m_vPlayer.size();++i)
	{
		clientsMap[client_s].sc_run.player[i].position = m_vPlayer[i].position;
		clientsMap[client_s].sc_run.player[i].player =m_vPlayer[i].player;
	
	}

		
	clientsMap[client_s].sc_run.playerCount = clientCount;
	clientsMap[client_s].dataBuffer.len = sizeof(SC_RUN);
	clientsMap[client_s].dataBuffer.buf =(char*)&clientsMap[client_s].sc_run.player;

	
	memset(&(clientsMap[client_s].overlapped), 0x00, sizeof(WSAOVERLAPPED));


	clientsMap[client_s].overlapped.hEvent = (HANDLE)client_s;

	// ������ wsaSend�� �ϸ� ������ recv�� �ؾ���. 
	// recv�� ����,overlapped����ü ������ �־ recv�� ����ؼ� ó���ϰ� �ؾ���.
	// ���� ������ �״�� send�ϰ� ��.
	if (WSASend(client_s, &(clientsMap[client_s].dataBuffer), 1, &clientsMap[client_s].dataBuffer.len, 0, &(clientsMap[client_s].overlapped), send_callback) == SOCKET_ERROR)
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			printf("Error - Fail WSASend(error_code : %d)\n", WSAGetLastError());
		}
	}

}

void CALLBACK send_callback(DWORD Error, DWORD dataBytes, LPWSAOVERLAPPED overlapped, DWORD lnFlags)
{
	DWORD sendBytes = 0;
	DWORD receiveBytes = 0;
	DWORD flags = 0;

	SOCKET client_s = reinterpret_cast<int>(overlapped->hEvent);

	if (dataBytes == 0) 
	{
		closesocket(clientsMap[client_s].socket);
		clientCount--;

		m_vPlayer.clear();
		return;
	}  // Ŭ���̾�Ʈ�� closesocket�� ���� ���

	{	//data�� ���۵Ȱ��
		// WSASend(���信 ����)�� �ݹ��� ���
		
		//clientsMap[client_s].socket = clientsMap[client_s].socket;

		clientsMap[client_s].dataBuffer.len = sizeof(CS_RUN);
		
		clientsMap[client_s].dataBuffer.buf = (char*)&clientsMap[client_s].cs_run.key;

		memset(&(clientsMap[client_s].overlapped), 0x00, sizeof(WSAOVERLAPPED));
		clientsMap[client_s].overlapped.hEvent = (HANDLE) client_s;
		if (WSARecv(client_s, &clientsMap[client_s].dataBuffer, 1, &clientsMap[client_s].dataBuffer.len, &flags, &(clientsMap[client_s].overlapped), recv_callback) == SOCKET_ERROR)
		{
			if (WSAGetLastError() != WSA_IO_PENDING)
			{
				printf("Error - Fail WSARecv(error_code : %d)\n", WSAGetLastError());
			}
		}
	}
}
