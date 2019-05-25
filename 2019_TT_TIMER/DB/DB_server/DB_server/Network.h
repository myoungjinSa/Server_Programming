#pragma once

#define _WINSOCK_DEPRECATED_NO_WARNINGS


#define BUF_SIZE 1024

class Network 
{
private:
	SOCKET sock;
	WSABUF send_wsabuf;
	char send_buffer[BUF_SIZE];
	WSABUF recv_wsabuf;
	char recv_buffer[BUF_SIZE];
	char packet_buffer[BUF_SIZE];
	DWORD in_packet_size = 0;
	int saved_packet_size = 0;

private:
	sd_packet_connect *pSDC = NULL;
	ds_packet_connect_result *pDSCR = NULL;
public:
	Network();
	~Network();
	void err_quit(const char *msg);
	void err_display(const char *msg);


	void SendConnectResult();

	void ReadPacket();
	void SendPacket();
	void ProcessPacket(char* packet);

};