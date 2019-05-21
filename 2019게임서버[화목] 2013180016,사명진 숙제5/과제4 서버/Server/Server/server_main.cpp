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
//#include <iostream>
//#include <functional>
#include <thread>
#include "TestServer.h"



using namespace std;

int main()
{

	vector<thread> worker_threads;

	wcout.imbue(locale("korean"));			//한글 출력
	_wsetlocale(LC_ALL, L"korean");


	CServerFramework server = CServerFramework();


	server.Initialize();
	

	server.g_iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);

	
	//function<void(CServerFramework&)> wt = &CServerFramework::Worker_Thread;
	//function<void(CServerFramework&)> doAcc = &CServerFramework::Do_Accept;

	for(int i=0; i< 4 ;++i)
	{
		worker_threads.emplace_back(thread{ &CServerFramework::Worker_Thread ,&server});
	}
	thread accept_thread{ &CServerFramework::Do_Accept,&server };
	thread timer_thread{ &CServerFramework::Timer_Thread,&server };



	timer_thread.join();
	accept_thread.join();

	for(auto& th : worker_threads)
	{
		th.join();
	}

	CloseHandle(server.g_iocp);


}


