#pragma once
#include "global.h"

class CServerFramework
{
public:
	CServerFramework();
	~CServerFramework();

	void err_quit(const char*);
	void err_display(const char*);
	
	SOCKET& AcceptClient();

	//static DWORD WINAPI	RecvThread(LPVOID);

	void ProcessKeyInput(byte& keyInput);

	void Update();
	void SendPacket(SOCKET&);
	void RecvPacket();
	int recvn(SOCKET, char*, int, int);


	void Destroy();


private:
	const u_short	serverPort{ 9000 };


	//措扁 家南
	SOCKET			m_listenSocket;

	SOCKET	m_clientSocket;
	SOCKADDR_IN m_clientAddr;
	int addrlen;
	int m_AcceptRequest;		//立加 夸没 冉荐

	vector<ClientInfo> m_vClientSocket;

	static HANDLE hThread[2];



};