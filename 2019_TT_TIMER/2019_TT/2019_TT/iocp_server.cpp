
#include <iostream>
#include <thread>
#include <map>
#include <vector>
#include <unordered_set>
#include <queue>
#include <chrono>
#include <unordered_map>


//#include "stdafx.h

using namespace std;
using namespace chrono;

#include <winsock2.h>

#include "protocol.h"

#pragma comment(lib, "Ws2_32.lib")

#define MAX_BUFFER        1024

#define VIEW_RADIUS  3

#define START_X		4
#define START_Y		4

#include "../../DB/DB_server/DB_server/MainDB.h"

enum EVENT_TYPE {
	EV_RECV,
	EV_SEND,
	EV_MOVE,
	EV_CHASE,
	EV_HEAL,
	EV_DB_CONNECT
};
struct OVER_EX
{
	WSAOVERLAPPED	overlapped;
	WSABUF			dataBuffer;
	char			messageBuffer[MAX_BUFFER];
	EVENT_TYPE		ev;
	//bool			is_recv;
};

struct SOCKETINFO
{
	//mutex	access_lock;
	bool	connected;
	OVER_EX over;
	SOCKET socket;
	char packet_buf[MAX_BUFFER];
	int prev_size;
	high_resolution_clock::time_point last_move_time;

	int x, y;
	unordered_set <int> viewlist;
};

unordered_set<int> clientID;



struct T_EVENT {
	high_resolution_clock::time_point start_time;
	int			do_object;
	EVENT_TYPE	event_type;

	constexpr bool operator < (const T_EVENT& _Left) const
	{	// apply operator< to operands
		return (start_time > _Left.start_time);
	}
};

priority_queue <T_EVENT> timer_queue;

SOCKETINFO clients[NPC_ID_START + NUM_NPC];

HANDLE g_iocp;

void add_timer(EVENT_TYPE ev_type, int object,
	high_resolution_clock::time_point start_time)
{
	timer_queue.push(T_EVENT{ start_time, object, ev_type });
}




void initialize()
{

	for (int i = 0; i < MAX_USER; ++i) {
		clients[i].connected = false;
		clients[i].viewlist.clear();
	}
	for (int i = NPC_ID_START; i < NPC_ID_START + NUM_NPC; ++i) {
		clients[i].x = rand() % WORLD_WIDTH;
		clients[i].y = rand() % WORLD_HEIGHT;
		clients[i].last_move_time = high_resolution_clock::now();
	}
	for (int i = NPC_ID_START; i < NPC_ID_START + NUM_NPC; ++i) {
		add_timer(EV_MOVE, i, high_resolution_clock::now() + 1s);
	}
}

int get_new_id()
{
	while(true)
	for (int i=0;i<MAX_USER;++i)
		if (clients[i].connected == false) {
			clients[i].connected = true;
			return i;
		}
}

bool is_player(int id)
{
	if ((id >= 0) && (id < MAX_USER)) return true;
	return false;
}

bool is_near_object(int a, int b)
{
	if (VIEW_RADIUS < abs(clients[a].x - clients[b].x)) 
		return false;
	if (VIEW_RADIUS < abs(clients[a].y - clients[b].y))
		return false;
	return true;
}

void error_display(const char *msg, int err_no)
{
	WCHAR *lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, err_no,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	cout << msg;
	wcout << L"에러 " << lpMsgBuf << endl;
	while (true);
	LocalFree(lpMsgBuf);
}

void send_packet(int key, char *packet)
{
	SOCKET client_s = clients[key].socket;

	OVER_EX *over = reinterpret_cast<OVER_EX *>(malloc(sizeof(OVER_EX)));

	over->dataBuffer.len = packet[0];
	over->dataBuffer.buf = over->messageBuffer;
	memcpy(over->messageBuffer, packet, packet[0]);
	ZeroMemory(&(over->overlapped), sizeof(WSAOVERLAPPED));
	over->ev = EV_SEND;
	if (WSASend(client_s, &over->dataBuffer, 1, NULL,
		0, &(over->overlapped), NULL) == SOCKET_ERROR)
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			cout << "Error - Fail WSASend(error_code : ";
			cout << WSAGetLastError() << ")\n";
		}
	}
}

void send_remove_player_packet(char to, int id)
{
	sc_packet_remove_player packet;
	packet.id = id;
	packet.size = sizeof(packet);
	packet.type = SC_REMOVE_PLAYER;
	send_packet(to, reinterpret_cast<char *>(&packet));
}

void disconnect(int id)
{
	for (int i = 0; i < MAX_USER; ++i) {
		if (false == clients[i].connected) continue;
		if (0 != clients[i].viewlist.count(id))
			send_remove_player_packet(i, id);
	}
	closesocket(clients[id].socket);
	clients[id].viewlist.clear();
	clients[id].connected = false;
}

void send_request_id(int to)
{
	sc_packet_request_id packet;

	packet.size = sizeof(packet);
	packet.type = SC_REQUEST_ID;
	send_packet(to, reinterpret_cast<char*>(&packet));

}
void send_login_ok_packet(int to)
{
	sc_packet_login_ok packet;
	packet.id = to;
	packet.size = sizeof(packet);
	packet.type = SC_LOGIN_OK;
	send_packet(to, reinterpret_cast<char *>(&packet));
}

void send_put_player_packet(int to, int obj)
{
	sc_packet_put_player packet;
	packet.id = obj;
	packet.size = sizeof(packet);
	packet.type = SC_PUT_PLAYER;
	packet.x = clients[obj].x;
	packet.y = clients[obj].y;
	send_packet(to, reinterpret_cast<char *>(&packet));
}

void send_pos_packet(int to, int obj)
{
	sc_packet_pos packet;
	packet.id = obj;
	packet.size = sizeof(packet);
	packet.type = SC_POS;
	packet.x = clients[obj].x;
	packet.y = clients[obj].y;
	send_packet(to, reinterpret_cast<char *>(&packet));
}

void process_packet(int id, char * buf)
{
	cs_packet_up *packet = reinterpret_cast<cs_packet_up *>(buf);

	short x = clients[id].x;
	short y = clients[id].y;
	switch (packet->type) {
	case CS_UP:
		--y;
		if (y < 0) y = 0;
		break;
	case CS_DOWN:
		++y;
		if (y >= WORLD_HEIGHT) y = WORLD_HEIGHT - 1;
		break;
	case CS_LEFT: if (0 < x) x--; break;
	case CS_RIGHT: if ((WORLD_WIDTH - 1) > x) x++; break;
	default :
		cout << "Unknown Packet Type Error\n";
		while (true);
	}
	clients[id].x = x;
	clients[id].y = y;

	unordered_set <int> old_vl = clients[id].viewlist;
	unordered_set <int> new_vl;
	for (int i = 0; i < MAX_USER; ++i) {
		if ((true == clients[i].connected) &&
			(true == is_near_object(id, i)) &&
			(i != id))
			new_vl.insert(i);
	}
	for (int i = 0; i < NUM_NPC; ++i) {
		int npc_id = i + NPC_ID_START;
		if (true == is_near_object(id, npc_id))
			new_vl.insert(npc_id);
	}

	for (auto cl : new_vl)
	{
		if (0 != old_vl.count(cl)) {  // old, new 동시 존재	 
			if (false == is_player(cl)) continue;
			if (0 != clients[cl].viewlist.count(id))
				send_pos_packet(cl, id);
			else {
				clients[cl].viewlist.insert(id);
				send_put_player_packet(cl, id);
			}
		} else { // 새로 시야에 들어옴
			clients[id].viewlist.insert(cl);
			send_put_player_packet(id, cl);
			if (false == is_player(cl)) continue;
			if (0 != clients[cl].viewlist.count(id))
				send_pos_packet(cl, id);
			else {
				clients[cl].viewlist.insert(id);
				send_put_player_packet(cl, id);
			}
		}
	}
	for (auto cl : old_vl) { // 시야에서 사라짐
		if (0 != new_vl.count(cl)) continue;
		clients[id].viewlist.erase(cl);
		send_remove_player_packet(id, cl);
		if (false == is_player(cl)) continue;
		if (0 != clients[cl].viewlist.count(id))
		{
			clients[cl].viewlist.erase(id);
			send_remove_player_packet(cl, id);
		}
	}
	send_pos_packet(id, id);
}

void do_recv(int id)
{
	DWORD flags = 0;

	SOCKET client_s = clients[id].socket;
	OVER_EX *over = &clients[id].over;

	over->dataBuffer.len = MAX_BUFFER;
	over->dataBuffer.buf = over->messageBuffer;
	ZeroMemory(&(over->overlapped), sizeof(WSAOVERLAPPED));
	if (WSARecv(client_s, &over->dataBuffer, 1, NULL,
		&flags, &(over->overlapped), NULL) == SOCKET_ERROR)
	{
		int err_no = WSAGetLastError();
		if (err_no != WSA_IO_PENDING)
		{
			error_display("RECV ERROR", err_no);
		}
	}
}

void worker_thread()
{
	while (true) {
		DWORD io_byte;
		ULONGLONG key;
		OVER_EX *lpover_ex;
		BOOL is_error = GetQueuedCompletionStatus(g_iocp, &io_byte, &key,
			reinterpret_cast<LPWSAOVERLAPPED *>(&lpover_ex), INFINITE);

		if (FALSE == is_error)
		{
			int err_no = WSAGetLastError();
			if (64 != err_no) 
				error_display("GQCS ", err_no);
			else {
				disconnect(key);
				continue;
			}
		}
		if (0 == io_byte) {
			disconnect(key);
			continue;
		}

		if (lpover_ex->ev==EV_RECV) {
			int rest_size = io_byte;
			char *ptr = lpover_ex->messageBuffer;
			char packet_size = 0;
			if (0 < clients[key].prev_size) packet_size = clients[key].packet_buf[0];
			while (rest_size > 0) {
				if (0 == packet_size) packet_size = ptr[0];
				int required = packet_size - clients[key].prev_size;
				if (rest_size >= required) {
					memcpy(clients[key].packet_buf + clients[key].
						prev_size, ptr, required);
					process_packet(key, clients[key].packet_buf);
					rest_size -= required;
					ptr += required;
					packet_size = 0;
				}
				else {
					memcpy(clients[key].packet_buf + clients[key].prev_size,
						ptr, rest_size);
					rest_size = 0;
				}
			}
			do_recv(key);
		}
		else {
			delete lpover_ex;
		}
	}
}

void check_login()
{
	while(true)
	{
		
	}
}
void admit_client(int new_id)
{


		clients[new_id].connected = true;



		send_login_ok_packet(new_id);
		send_put_player_packet(new_id, new_id);
		for (int i = 0; i < MAX_USER; ++i) {
			if (false == clients[i].connected) continue;
			if (i == new_id) continue;
			if (true == is_near_object(i, new_id)) {
				clients[i].viewlist.insert(new_id);
				send_put_player_packet(i, new_id);
			}
		}

		for (int i = 0; i < MAX_USER; ++i) {
			if (false == clients[i].connected) continue;
			if (i == new_id) continue;
			if (true == is_near_object(i, new_id)) {
				clients[new_id].viewlist.insert(i);
				send_put_player_packet(new_id, i);
			}
		}

		for (int i = 0; i < NUM_NPC; ++i) {
			int npc_id = i + NPC_ID_START;
			if (true == is_near_object(npc_id, new_id))
			{
				clients[new_id].viewlist.insert(npc_id);
				send_put_player_packet(new_id, npc_id);
			}
		}
	
}
void do_accept()
{
	// Winsock Start - windock.dll 로드
	WSADATA WSAData;
	if (WSAStartup(MAKEWORD(2, 2), &WSAData) != 0)
	{
		cout << "Error - Can not load 'winsock.dll' file\n";
		return;
	}

	// 1. 소켓생성  
	SOCKET listenSocket = WSASocketW(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (listenSocket == INVALID_SOCKET)
	{
		cout << "Error - Invalid socket\n";
		return;
	}

	// 서버정보 객체설정
	SOCKADDR_IN serverAddr;
	memset(&serverAddr, 0, sizeof(SOCKADDR_IN));
	serverAddr.sin_family = PF_INET;
	serverAddr.sin_port = htons(SERVER_PORT);
	serverAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

	// 2. 소켓설정
	if (::bind(listenSocket, (struct sockaddr*)&serverAddr, sizeof(SOCKADDR_IN)) == SOCKET_ERROR)
	{
		cout << "Error - Fail bind\n";
		// 6. 소켓종료
		closesocket(listenSocket);
		// Winsock End
		WSACleanup();
		return;
	}

	// 3. 수신대기열생성
	if (listen(listenSocket, 5) == SOCKET_ERROR)
	{
		cout << "Error - Fail listen\n";
		// 6. 소켓종료
		closesocket(listenSocket);
		// Winsock End
		WSACleanup();
		return;
	}

	SOCKADDR_IN clientAddr;
	int addrLen = sizeof(SOCKADDR_IN);
	memset(&clientAddr, 0, addrLen);
	SOCKET clientSocket;
	DWORD flags;

	while (true)
	{
		clientSocket = accept(listenSocket, (struct sockaddr *)&clientAddr, &addrLen);
		if (clientSocket == INVALID_SOCKET)
		{
			cout << "Error - Accept Failure\n";
			return;
		}

		int new_id = get_new_id();
	

		clients[new_id].socket = clientSocket;
		clients[new_id].over.dataBuffer.len = MAX_BUFFER;
		clients[new_id].over.dataBuffer.buf =
			clients[clientSocket].over.messageBuffer;
		clients[new_id].over.ev = EV_RECV;
		clients[new_id].x = START_X;
		clients[new_id].y = START_Y;
		clients[new_id].viewlist.clear();
		clients[new_id].prev_size = 0;
		ZeroMemory(&clients[new_id].over.overlapped, 
			sizeof(WSAOVERLAPPED));
		flags = 0;

		CreateIoCompletionPort(
			reinterpret_cast<HANDLE>(clientSocket)
			, g_iocp, new_id, 0);
		
		//DB에게 ID 존재 여부를 보내고 결과를 받아야함
		
		//ID 요청을 한 후 clientID에 저장해놓는다.
	//	send_request_id(new_id);
		
		//clientID.emplace(new_id);
		
		admit_client(new_id);
		
		do_recv(new_id);
	}

	// 6-2. 리슨 소켓종료
	closesocket(listenSocket);

	// Winsock End
	WSACleanup();

	return;
}


void random_move_npc(int id)
{
	int x = clients[id].x;
	int y = clients[id].y;

	unordered_set <int> old_vl;
	for (int i = 0; i < MAX_USER; i++) {
		if (false == clients[i].connected) continue;
		if (false == is_near_object(id, i)) continue;
		old_vl.insert(i);
	}

	char dir = rand() % 4;
	switch (dir) {
	case 0: if (y > 0) y--; break;
	case 1:if (y < (WORLD_HEIGHT - 1)) y++; break;
	case 2: if (x > 0) x--; break;
	case 3: if (x < (WORLD_WIDTH - 1)) x++; break;
	}
	clients[id].x = x;
	clients[id].y = y;

	unordered_set<int> new_vl;
	for (int i = 0; i < MAX_USER; i++) {
		if (false == clients[i].connected) continue;
		if (false == is_near_object(id, i)) continue;
		new_vl.insert(i);
	}

	volatile int sum = 0;
	for (int i = 0; i < 1000000; ++i)
		sum += i;

	for (auto user : old_vl) {
		if (0 != new_vl.count(user)) {
			if (clients[user].viewlist.count(id))
				send_pos_packet(user, id);
			else {
				clients[user].viewlist.insert(id);
				send_put_player_packet(user, id);
			}
		}
		else {
			if (0 < clients[user].viewlist.count(id)) {
				clients[user].viewlist.erase(id);
				send_remove_player_packet(user, id);
			}
		}
	}
	for (auto user : new_vl) {
		if (0 == old_vl.count(id)) {
			if (0 == clients[user].viewlist.count(id)) {
				clients[user].viewlist.insert(id);
				send_put_player_packet(user, id);
			}
			else
				send_pos_packet(user, id);
		}
	}
}

void heart_beat(int npc_id)
{
	random_move_npc(npc_id);
}

void do_ai()
{
	while (true) {
		auto start_t = high_resolution_clock::now();
		for (auto i = NPC_ID_START; i < NPC_ID_START + NUM_NPC; ++i)
		{
			//if (clients[i].last_move_time < high_resolution_clock::now() - 1s) {
			//	clients[i].last_move_time = high_resolution_clock::now();
			//	random_move_npc(i);
			//}
			heart_beat(i);
		}
		auto end_t = high_resolution_clock::now();
		auto ai_time = end_t - start_t;
		auto delay_time = 1s - ai_time;
		if (ai_time < 1s) 
			this_thread::sleep_for(delay_time);
		//cout << "AI process time : " << duration_cast<milliseconds>(ai_time).count() << "ms\n";
	}
}

void process_event(T_EVENT &ev)
{
	switch (ev.event_type) {
	case EV_MOVE: random_move_npc(ev.do_object );
		add_timer(EV_MOVE, ev.do_object, high_resolution_clock::now() + 1s); 
		break;
	default :
		cout << "Unknown Event!!!\n";
		while (true);
	}
}

void do_timer()
{
	int count = 0;
	auto timer_start = high_resolution_clock::now();

	while (true) {
		this_thread::sleep_for(10ms);
		while (true) {
			if (true == timer_queue.empty()) break;
			T_EVENT ev = timer_queue.top();
			if (ev.start_time > high_resolution_clock::now()) 
				break;
			timer_queue.pop();
			process_event(ev);
			count++;

			if ((count % 100) == 0)
			{
				int sc = duration_cast<seconds>(
					high_resolution_clock::now() - timer_start).count(); 
				if(sc !=0)
					cout << count / sc <<  " MonsterMove/Sec\n";
			}
		}
	}
}

int main()
{
	

	vector <thread> worker_threads;

	wcout.imbue(locale("korean"));
	initialize();
	g_iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);
	for (int i = 0; i < 4; ++i)
		worker_threads.emplace_back(thread{ worker_thread });
//	thread login_thread{ check_login };
	thread accept_thread{ do_accept };

	//thread ai_thread{ do_ai };
	//ai_thread.join();
	/*auto db = std::make_unique<MainDB>();

	if (db->Initialize())
		db->Connect_DB();
	else
		std::cout << "디비 생성 실패" << std::endl;*/

	
	thread timer_thread{ do_timer };
	timer_thread.join();

	//login_thread.join();
	accept_thread.join();
	for (auto &th : worker_threads) th.join();
	CloseHandle(g_iocp);
}