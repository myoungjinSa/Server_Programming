#pragma once
#include "global.h"
#include "GameTimer\GameTimer.h"


class CServerFramework
{
public:
	CServerFramework();
	~CServerFramework();

	void err_quit(const char*);
	void err_display(const char*);
	
	SOCKET& AcceptClient();

	//static DWORD WINAPI	RecvThread(LPVOID);

	void ProcessKeyInput(const byte& keyInput);


	void Update(const CS_RUN& cs_runPacket);
	void SendPacket();
	void RecvPacket();
	int recvn(SOCKET, char*, int, int);


	void Destroy();

	//LRESULT CALLBACK OnProcessingWindowMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);


private:
	const u_short	serverPort{ 9000 };

	const int m_frameWidth = 1200;
	const int m_frameHeight = 800;
	const int m_fWidthStep		= m_frameWidth  / 8;
	const int m_fHeightStep		= m_frameHeight / 8;

	
	//措扁 家南
	SOCKET			m_listenSocket;

	SOCKET	m_clientSocket;
	SOCKADDR_IN m_clientAddr;
	int addrlen;
	int m_AcceptRequest;		//立加 夸没 冉荐

	vector<ClientInfo> m_vClientSocket;

	static HANDLE hThread[2];


	CGameTimer					m_GameTimer;
};