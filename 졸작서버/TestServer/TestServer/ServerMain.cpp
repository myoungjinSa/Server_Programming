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

#include <vector>
#include <thread>
#include "TestServer.h"


using namespace std;


int main()
{

	vector<thread> worker_threads;

	wcout.imbue(locale("korean"));			//한글 출력
	_wsetlocale(LC_ALL, L"korean");

	CServerFramework server;

	server.Initialize();
	//터레인 raw파일 정보를 서버도 갖고 있는다.
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