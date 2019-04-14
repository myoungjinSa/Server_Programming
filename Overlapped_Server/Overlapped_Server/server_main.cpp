/*
## 소켓 서버 : 1 v n - overlapped callback
1. socket()            : 소켓생성
2. bind()            : 소켓설정
3. listen()            : 수신대기열생성
4. accept()            : 연결대기
5. read()&write()
WIN recv()&send    : 데이터 읽고쓰기
6. close()
WIN closesocket    : 소켓종료
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
	byte  player;				//플레이어 정보
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


struct SOCKETINFO				//소켓의 정보
{
	WSAOVERLAPPED overlapped;				//서로 충돌이일어나기 때문에 소켓마다 OVERLAPPED 구조체가 있어야함.
	WSABUF dataBuffer;						//윈도우를 사용하고 있으므로 WSABUF 가 필요함.
	SOCKET socket;
	int receiveBytes;
	int sendBytes;

	CS_RUN cs_run;
	SC_RUN sc_run;
	
	PlayerInfo	player_Info;
	//POS		position;

	//HP,플레이어 정보는 이쪽에 들어가면됨.
};

map <SOCKET, SOCKETINFO> clientsMap;			//소켓마다 어떤 소켓정보를 쓰는지 매핑이 되어야함.
std::vector<PlayerInfo> m_vPlayer;

//타입은 정해져 있음. 
void CALLBACK recv_callback(DWORD Error, DWORD dataBytes, LPWSAOVERLAPPED overlapped, DWORD lnFlags);
void CALLBACK send_callback(DWORD Error, DWORD dataBytes, LPWSAOVERLAPPED overlapped, DWORD lnFlags);

const int fStartX = 7500;
const int fStartY = 7500;

	//클라이언트의 프레임 설정
const int fWidthStep = FRAME_WIDTH / 8;
const int fHeightStep = FRAME_HEIGHT / 8;

// 총 접속한 클라이언트 개수
int clientCount = 0;
int main()
{
	// Winsock Start - windock.dll 로드
	WSADATA WSAData;
	if (WSAStartup(MAKEWORD(2, 2), &WSAData) != 0)
	{
		cout << "Error - Can not load 'winsock.dll' file\n";
		return 1;
	}

	// 1. 소켓생성  
	SOCKET listenSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);		//OVERLAPPED모델은 반드시 WSA_FLAG_OVERLAPPED 를 사용해야함.
	if (listenSocket == INVALID_SOCKET)
	{
		printf("Error - Invalid socket\n");
		return 1;
	}

	// 서버정보 객체설정
	SOCKADDR_IN serverAddr;
	memset(&serverAddr, 0, sizeof(SOCKADDR_IN));
	serverAddr.sin_family = PF_INET;
	serverAddr.sin_port = htons(SERVER_PORT);
	serverAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);			//어떤 주소가 오든지 다 받겠다 - INADDR_ANY

	// 2. 소켓설정
	if (bind(listenSocket, (struct sockaddr*)&serverAddr, sizeof(SOCKADDR_IN)) == SOCKET_ERROR)
	{
		printf("Error - Fail bind\n");
		// 6. 소켓종료
		closesocket(listenSocket);
		// Winsock End
		WSACleanup();
		return 1;
	}

	// 3. 수신대기열생성
	if (listen(listenSocket, 5) == SOCKET_ERROR)		//listen(socket, backlog) 
	{													//backlog -> 들어오는 연결수락 대기열 수
		printf("Error - Fail listen\n");
		
		// 6. 소켓종료
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

		clientsMap[clientSocket] = SOCKETINFO{};			//각 SOCKET을 키값으로 하는 맵 컨테이너에 소켓 정보 초기화
		memset(&clientsMap[clientSocket], 0x00, sizeof(struct SOCKETINFO));
		
		clientsMap[clientSocket].socket = clientSocket;
		
		clientsMap[clientSocket].player_Info.position.x = fStartX;//fWidthStep * 1.0f +(fWidthStep * clientCount);
		clientsMap[clientSocket].player_Info.position.y = fStartY;//fHeightStep * 1.0f + (fHeightStep * clientCount);
		//대입연산자가 후위증가연산자보다 먼저다.
		clientsMap[clientSocket].player_Info.player = clientCount;
		clientsMap[clientSocket].sc_run.index = clientCount;
		//clientsMap[clientSocket].cs_run.player = clientCount;
		clientsMap[clientSocket].dataBuffer.len = sizeof(CS_RUN);
		clientsMap[clientSocket].dataBuffer.buf =(char*)&clientsMap[clientSocket].cs_run.key;
		flags = 0;

		
	
		m_vPlayer.emplace_back(clientsMap[clientSocket].player_Info);
		clientCount++;
		// 중첩 소캣을 지정하고 완료시 실행될 함수를 넘겨준다. - 이벤트는 사용하지 않을거라서 0을 넣어줘도 됨
		clientsMap[clientSocket].overlapped.hEvent = (HANDLE)clientsMap[clientSocket].socket;
		

		//ACCEPT 되자마자 RECV를 해줌, -> 
		if (WSARecv(clientsMap[clientSocket].socket, &clientsMap[clientSocket].dataBuffer, 1, &clientsMap[clientSocket].dataBuffer.len, &flags, &(clientsMap[clientSocket].overlapped), recv_callback))	//플래그에는 NULL쓰면 안됨, -> 플래그가 0인가아닌가 검사를하기때문에
		{
			//WSA_IO_PENDING: An overlapped operation was successfully initiated and completion will be indicated at a later time. 
			if (WSAGetLastError() != WSA_IO_PENDING)	//IOPENDING에러가 나는게 정상
			{
				printf("Error - IO pending Failure\n");
				return 1;
			}
		}
		else {
			//동기식- 여기서 처리하면 프로그래밍이 지저분해짐,위의 인자가 NULL이기때문에 여기로 오지 않음.
			cout << "Non Overlapped Recv return.\n";
			return 1;
		}
	}

	// 6-2. 리슨 소켓종료
	closesocket(listenSocket);

	// Winsock End
	WSACleanup();

	return 0;
}

//콜백함수가 어떤 소켓에서 온것인지 자체적으로 알수있어야함.
//여기 함수에서는 RECV를 하면 안됨. 이미 버퍼에 데이터가 온것이므로
void CALLBACK recv_callback(DWORD Error, DWORD dataBytes, LPWSAOVERLAPPED overlapped, DWORD lnFlags)
{
	SOCKET client_s = reinterpret_cast<int>(overlapped->hEvent);		//32비트이기 때문에 소켓인덱스 

	
	DWORD sendBytes = 0;
	DWORD receiveBytes = 0;
	DWORD flags = 0;

	if (dataBytes == 0)						//Client가 closeSocket호출을하면 dataBytes 가 0 인 채로 recv가 호출된다. 
	{										//그러므로 서버도 해당 클라이언트 소켓을 closeSocket처리해주면됨.
		closesocket(clientsMap[client_s].socket);
		
		clientCount--;
		cout << clientCount << endl;

		m_vPlayer.clear();
		
		return;
	}  // 클라이언트가 closesocket을 했을 경우

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

	// 원래는 wsaSend를 하면 무조건 recv를 해야함. 
	// recv용 버퍼,overlapped구조체 여러개 있어서 recv를 계속해서 처리하게 해야함.
	// 받은 내용을 그대로 send하게 함.
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
	}  // 클라이언트가 closesocket을 했을 경우

	{	//data가 전송된경우
		// WSASend(응답에 대한)의 콜백일 경우
		
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
