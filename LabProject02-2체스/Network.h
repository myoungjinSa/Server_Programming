#pragma once

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment(lib,"ws2_32")


#include<winSock2.h>



#define KEY_IDLE	0x00
#define KEY_RIGHT	0x01
#define KEY_LEFT	0x02
#define KEY_UP		0x03
#define KEY_DOWN	0x04

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
	SC_RUN(){}
	SC_RUN(float x,float y) :posX(x),posY(y){}
	float posX;
	float posY;

};

#pragma pack()


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

private:
	SOCKET m_socket;
	SOCKADDR_IN m_serverAddr;

	const std::string SERVERIP = "127.0.0.1";
	const u_short SERVERPORT = 9000;

	CS_RUN m_csRunPacket;
	SC_RUN m_scRunPacket;


};