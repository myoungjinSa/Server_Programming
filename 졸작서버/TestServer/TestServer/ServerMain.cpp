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

#include <vector>
#include <thread>
#include "TestServer.h"


using namespace std;


int main()
{

	vector<thread> worker_threads;

	wcout.imbue(locale("korean"));			//�ѱ� ���
	_wsetlocale(LC_ALL, L"korean");

	CServerFramework server;

	server.Initialize();
	//�ͷ��� raw���� ������ ������ ���� �ִ´�.
	server.SetHeightmapInfo("../../../FreezeBomb/Resource/Textures/Terrain/Terrain.raw", 256, 256, XMFLOAT3(2.0f, 1.0f, 2.0f));
	

	server.g_iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);

	
	//function<void(CServerFramework&)> wt = &CServerFramework::Worker_Thread;
	//function<void(CServerFramework&)> doAcc = &CServerFramework::Do_Accept;

	for(int i=0; i< 4 ;++i)
	{
		worker_threads.emplace_back(thread{ &CServerFramework::Worker_Thread ,&server});
	}
	
	thread accept_thread{ &CServerFramework::Do_Accept,&server };


	accept_thread.join();

	for(auto& th : worker_threads)
	{
		th.join();
	}
	server.DeleteHeightmapInfo();

	CloseHandle(server.g_iocp);


}