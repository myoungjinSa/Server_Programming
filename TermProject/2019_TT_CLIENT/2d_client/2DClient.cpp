// PROG14_1_16b.CPP - DirectInput keyboard demo

// INCLUDES ///////////////////////////////////////////////


#define WIN32_LEAN_AND_MEAN  
#define INITGUID
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include <WinSock2.h>
#include <windows.h>   // include important windows stuff
#include <windowsx.h>
#include <stdio.h>
#include <string>
#include <tchar.h>
#include <iostream>
#include <d3d9.h>     // directX includes
#include "d3dx9tex.h"     // directX includes
#include "gpdumb1.h"
#include "..\..\2019_TT\2019_TT\protocol.h"

#pragma comment (lib, "ws2_32.lib")



// DEFINES ////////////////////////////////////////////////


#define MAX(a,b)	((a)>(b))?(a):(b)
#define	MIN(a,b)	((a)<(b))?(a):(b)

// defines for windows 
#define WINDOW_CLASS_NAME L"WINXCLASS"  // class name

#define WINDOW_WIDTH    1320   // size of window
#define WINDOW_HEIGHT   800

#define	BUF_SIZE				1024
#define	WM_SOCKET				WM_USER + 1

using namespace std;
// PROTOTYPES /////////////////////////////////////////////

// game console
int Game_Init(void *parms = NULL);
int Game_Shutdown(void *parms = NULL);
int Game_Main(void *parms = NULL);

// GLOBALS ////////////////////////////////////////////////

HWND main_window_handle = NULL; // save the window handle
HINSTANCE main_instance = NULL; // save the instance
char buffer[80];                // used to print text

constexpr int inven_count = 3;
								// demo globals
BOB			player;				// 플레이어 Unit
BOB         item[ITEM_COUNT];			// item;
BOB			inven[inven_count];		//인벤
BOB			object[300];
BOB			npc[NUM_NPC];      // NPC Unit
BOB         skelaton[MAX_USER];     // the other player skelaton

BITMAP_IMAGE reactor;      // the background   

BITMAP_IMAGE black_tile;
BITMAP_IMAGE white_tile;
#define TILE_WIDTH 65
#define TILE_HEIGHT 65

#define UNIT_TEXTURE  0
#define OBJECT_TEXTURE 1
#define NPC_TEXTURE 2
#define HEALTH_PORTION_TEXTURE 3 
#define SKILL_PORTION_TEXTURE 4
#define SPEED_PORTION_TEXTURE 5


SOCKET g_mysocket;
WSABUF	send_wsabuf;
char 	send_buffer[BUF_SIZE];
WSABUF	recv_wsabuf;
char	recv_buffer[BUF_SIZE];
char	packet_buffer[BUF_SIZE];
DWORD		in_packet_size = 0;
int		saved_packet_size = 0;
int		g_myid;
bool    g_gameStart{ false };
int		g_left_x = 0;
int     g_top_y = 0;

int     table_id;
// FUNCTIONS //////////////////////////////////////////////

void SendConnectPacket(const string& id)
{
	//cs_packet_up *my_packet = reinterpret_cast<cs_packet_up *>(send_buffer);
	//	my_packet->size = sizeof(my_packet);
	//	send_wsabuf.len = sizeof(my_packet);
	//	DWORD iobyte;
	//	if (0 != x) {
	//		if (1 == x) my_packet->type = CS_RIGHT;
	//		else my_packet->type = CS_LEFT;
	//		int ret = WSASend(g_mysocket, &send_wsabuf, 1, &iobyte, 0, NULL, NULL);
	//		if (ret) {
	//			int error_code = WSAGetLastError();
	//			printf("Error while sending packet [%d]", error_code);
	//		}
	//	}
	cs_packet_connect* packet = reinterpret_cast<cs_packet_connect*>(send_buffer);
	packet->size = sizeof(packet);
	send_wsabuf.len = sizeof(packet);
	packet->type = CS_REQUEST_CONNECT;
	packet->id = stoi(id);
	table_id = stoi(id);
	
	DWORD iobyte;
	int ret = WSASend(g_mysocket, &send_wsabuf, 1, &iobyte, 0, NULL, NULL);
	if (ret) {
		int error_code = WSAGetLastError();
		printf("Error while sending packet [%d]", error_code);
	}

}

void SendRequestPosSave(const int& id, const int& posX,const int& posY)
{
	cs_packet_pos_save* packet = reinterpret_cast<cs_packet_pos_save*>(send_buffer);
	ZeroMemory(packet, sizeof(cs_packet_pos_save));
	packet->size = sizeof(cs_packet_pos_save);
	send_wsabuf.len = sizeof(cs_packet_pos_save);

	packet->type = CS_REQUEST_POS_SAVE;
	packet->id = table_id;

	packet->posX = posX;
	packet->posY = posY;

	DWORD iobyte;
	int ret = WSASend(g_mysocket, &send_wsabuf, 1, &iobyte, 0, NULL, NULL);
	if(ret)
	{
		int error_code = WSAGetLastError();
		printf("Error while sending packet [%d]", error_code);
	}

}

void SendUseHealItemPacket(const int id)
{
	cs_packet_use_item_heal *packet = reinterpret_cast<cs_packet_use_item_heal*>(send_buffer);
	ZeroMemory(packet, sizeof(cs_packet_use_item_heal));
	packet->size = sizeof(cs_packet_use_item_heal);
	send_wsabuf.len = sizeof(cs_packet_use_item_heal);

	packet->type = CS_USE_HEAL_ITEM;
	packet->id = id;

	DWORD iobyte;
	int ret = WSASend(g_mysocket, &send_wsabuf, 1, &iobyte, 0, NULL, NULL);
	if(ret)
	{
		int error_code = WSAGetLastError();
		printf("Error while sending packet[%d]", error_code);
	}

}
void SendUseSkillItemPacket(const int id)
{
	cs_packet_use_item_skill *packet = reinterpret_cast<cs_packet_use_item_skill*>(send_buffer);
	ZeroMemory(packet, sizeof(cs_packet_use_item_skill));
	packet->size = sizeof(cs_packet_use_item_skill);
	send_wsabuf.len = sizeof(cs_packet_use_item_skill);

	packet->type = CS_USE_SKILL_ITEM;
	packet->id = id;

	DWORD iobyte;
	int ret = WSASend(g_mysocket, &send_wsabuf, 1, &iobyte, 0, NULL, NULL);
	if(ret)
	{
		int error_code = WSAGetLastError();
		printf("Error while sending packet[%d]", error_code);
	}

}

void SendUseSpeedItemPacket(const int id)
{
	cs_packet_use_item_speed *packet = reinterpret_cast<cs_packet_use_item_speed*>(send_buffer);
	ZeroMemory(packet, sizeof(cs_packet_use_item_speed));
	packet->size = sizeof(cs_packet_use_item_speed);
	send_wsabuf.len = sizeof(cs_packet_use_item_speed);

	packet->type = CS_USE_SPEED_ITEM;
	packet->id = id;

	DWORD iobyte;
	int ret = WSASend(g_mysocket, &send_wsabuf, 1, &iobyte, 0, NULL, NULL);
	if(ret)
	{
		int error_code = WSAGetLastError();
		printf("Error while sending packet[%d]", error_code);
	}

}
void SendAttackPacket(int id)
{
	cs_packet_attack *packet = reinterpret_cast<cs_packet_attack*>(send_buffer);
	ZeroMemory(packet, sizeof(cs_packet_attack));
	packet->size = sizeof(cs_packet_attack);
	send_wsabuf.len = sizeof(cs_packet_attack);

	packet->type = CS_ATTACK;
	packet->id = id;
	packet->power = player.power;

	DWORD iobyte;
	int ret = WSASend(g_mysocket, &send_wsabuf, 1, &iobyte, 0, NULL, NULL);
	if(ret)
	{
		int error_code = WSAGetLastError();
		printf("Error while sending packet[%d]", error_code);
	}
}

void ProcessPacket(char *ptr)
{
	static bool first_time = true;
	switch (ptr[1])
	{
	case SC_REQUEST_ID:
	{

	
		sc_packet_request_id *packet = reinterpret_cast<sc_packet_request_id*>(ptr);
			
		string id;
		cout << "접속할 id를 입력하세요" << endl;
		//
		cin >> id;

		cout << id;
		SendConnectPacket(id);

		break;
	}

	case SC_LOGIN_OK:
	{
		sc_packet_login_ok *packet = 
			reinterpret_cast<sc_packet_login_ok *>(ptr);
		g_myid = packet->id;

		break;
	}
	case SC_DENY_LOGIN:
	{
		
		cout << "해당 id가 DB에 없습니다.\n";
		string id;
		cout << "접속할 id를 입력하세요" << endl;
		//
		cin >> id;

		cout << id;
		SendConnectPacket(id);
		break;

	}
	case SC_PUT_PLAYER:
	{
		sc_packet_put_player *my_packet = reinterpret_cast<sc_packet_put_player *>(ptr);
		int id = my_packet->id;

		if (id == g_myid) {
			player.x = my_packet->x;
			player.y = my_packet->y;
			player.hp = my_packet->hp;
			player.mp = my_packet->mp;
			player.power = my_packet->power;
			player.alive = true;
			player.attr |= BOB_ATTR_VISIBLE;
			g_gameStart = true;
		}
		else if (id < MAX_USER) {
			skelaton[id].x = my_packet->x;
			skelaton[id].y = my_packet->y;

			skelaton[id].attr |= BOB_ATTR_VISIBLE;
		}
		else {
			npc[id - NPC_ID_START].x = my_packet->x;
			npc[id - NPC_ID_START].y = my_packet->y;
			npc[id - NPC_ID_START].hp = my_packet->hp;
			npc[id - NPC_ID_START].mp = my_packet->mp;
			npc[id - NPC_ID_START].power = my_packet->power;
			//npc[id - NPC_ID_START].hp = my_packet->hp;
			npc[id - NPC_ID_START].attr |= BOB_ATTR_VISIBLE;
		}
		break;
	}
	case SC_POS:
	{
		sc_packet_pos *my_packet = reinterpret_cast<sc_packet_pos *>(ptr);
		int other_id = my_packet->id;
		if (other_id == g_myid) {
			g_left_x = my_packet->x - 10;
			g_top_y = my_packet->y - 6;
			player.x = my_packet->x;
			player.y = my_packet->y;
		}
		else if (other_id < MAX_USER) {
			skelaton[other_id].x = my_packet->x;
			skelaton[other_id].y = my_packet->y;
		}
		else {
			npc[other_id - NPC_ID_START].x = my_packet->x;
			npc[other_id - NPC_ID_START].y = my_packet->y;
		}
		break;
	}

	case SC_REMOVE_PLAYER:
	{
		sc_packet_remove_player *my_packet = reinterpret_cast<sc_packet_remove_player *>(ptr);
		int other_id = my_packet->id;
		if (other_id == g_myid) {
			player.attr &= ~BOB_ATTR_VISIBLE;
		}
		else if (other_id < MAX_USER) {
			skelaton[other_id].attr &= ~BOB_ATTR_VISIBLE;
		}
		else {
			npc[other_id - NPC_ID_START].attr &= ~BOB_ATTR_VISIBLE;
		}
		break;
	}
	case SC_PUT_ITEM:
	{
		sc_packet_item_put * my_packet = reinterpret_cast<sc_packet_item_put*>(ptr);
		int other_id = my_packet->id;
		if(other_id >= NPC_ID_START + NUM_NPC && other_id < NUM_ITEM)
		{
			item[other_id - (NPC_ID_START + NUM_NPC)].x = my_packet->x;
			item[other_id - (NPC_ID_START + NUM_NPC)].y = my_packet->y;
			item[other_id - (NPC_ID_START + NUM_NPC)].world_item_kind = my_packet->kind;
			item[other_id - (NPC_ID_START + NUM_NPC)].attr |= BOB_ATTR_VISIBLE;
		}

		break;
	}
	case SC_ITEM_EAT:
	{
		sc_packet_item_eat* my_packet = reinterpret_cast<sc_packet_item_eat*>(ptr);
		
		int id = my_packet->id;

		if(id >= NPC_ID_START + NUM_NPC && id < NUM_ITEM )
		{
			if (player.item_count < MAX_ITEMS)
			{
				player.items[player.item_count].blank = false;
				player.items[player.item_count].kind = my_packet->kind+HEALTH_PORTION_TEXTURE;
				player.item_count += 1;
			}
		}

		break;
	}

	case SC_REMOVE_ITEM:
	{
		sc_packet_remove_item* my_packet = reinterpret_cast<sc_packet_remove_item*>(ptr);

		int other_id = my_packet->id;
		if(other_id >= NPC_ID_START + NUM_NPC && other_id < NUM_ITEM)
		{
			item[other_id - (NPC_ID_START + NUM_NPC)].attr &= ~BOB_ATTR_VISIBLE;
		}

		break;
	}
	case SC_POS_SAVE_RESULT:
	{
		cout << "정상적으로 위치가 저장되었습니다" << endl;
		Sleep(1000);

		break;
	}
	case SC_HP:
	{
		sc_packet_hp* my_packet = reinterpret_cast<sc_packet_hp*>(ptr);
		int other_id = my_packet->id;

		if(other_id == g_myid)
		{
			player.hp = my_packet->hp;
		}
		else if(other_id < MAX_USER)
		{
			skelaton[other_id].hp = my_packet->hp;
		}
		else
		{
			npc[other_id - NPC_ID_START].hp = my_packet->hp;
		}

		break;
	}
	case SC_MP:
	{
		sc_packet_mp* my_packet = reinterpret_cast<sc_packet_mp*>(ptr);
		int other_id = my_packet->id;

		if(other_id == g_myid)
		{
			player.mp = my_packet->mp;
		}
		else if(other_id < MAX_USER)
		{
			skelaton[other_id].mp = my_packet->mp;
		}
		else
		{
			npc[other_id - NPC_ID_START].mp = my_packet->mp;
		}
		
		break;
	}
	case SC_SPEED:
	{
		sc_packet_speed* my_packet = reinterpret_cast<sc_packet_speed*>(ptr);
		int other_id = my_packet->id;

		if(other_id == g_myid)
		{
			player.speed = my_packet->speed;

		}
		else if(other_id < MAX_USER)
		{
			skelaton[other_id].speed = my_packet->speed;
		}

			
		break;
	}
	case SC_DEAD:
	{
		sc_packet_dead* my_packet = reinterpret_cast<sc_packet_dead*>(ptr);
		int other_id = my_packet->id;

		if(other_id == g_myid)
		{
			player.alive = false;
		}
	


		break;
	}
	case SC_ATTACK_RESULT:
	{
		sc_packet_attack_result* my_packet = reinterpret_cast<sc_packet_attack_result*>(ptr);

		int other_id = my_packet->id;

		if(other_id == g_myid)
		{
			if(my_packet->isHit == true)
			{
				cout << my_packet->npc_id << "번 째 NPC에게 데미지" << player.power << "를 주었습니다.\n";
			}
			else
			{
				cout << "공격이 빗나갔습니다.\n";
			}
			
		}

		break;
	}
	/*
	case SC_CHAT:
	{
		sc_packet_chat *my_packet = reinterpret_cast<sc_packet_chat *>(ptr);
		int other_id = my_packet->id;
		if (other_id == g_myid) {
			wcsncpy_s(player.message, my_packet->message, 256);
			player.message_time = GetTickCount();
		}
		else if (other_id < NPC_START) {
			wcsncpy_s(skelaton[other_id].message, my_packet->message, 256);
			skelaton[other_id].message_time = GetTickCount();
		}
		else {
			wcsncpy_s(npc[other_id - NPC_START].message, my_packet->message, 256);
			npc[other_id - NPC_START].message_time = GetTickCount();
		}
		break;

	} */
	default:
		printf("Unknown PACKET type [%d]\n", ptr[1]);
	}
}

void ReadPacket(SOCKET sock)
{
	DWORD iobyte, ioflag = 0;

	int ret = WSARecv(sock, &recv_wsabuf, 1, &iobyte, &ioflag, NULL, NULL);
	if (ret) {
		int err_code = WSAGetLastError();
		printf("Recv Error [%d]\n", err_code);
	}

	BYTE *ptr = reinterpret_cast<BYTE *>(recv_buffer);

	while (0 != iobyte) {
		if (0 == in_packet_size) in_packet_size = ptr[0];
		if (iobyte + saved_packet_size >= in_packet_size) {
			memcpy(packet_buffer + saved_packet_size, ptr, in_packet_size - saved_packet_size);
			ProcessPacket(packet_buffer);
			ptr += in_packet_size - saved_packet_size;
			iobyte -= in_packet_size - saved_packet_size;
			in_packet_size = 0;
			saved_packet_size = 0;
		}
		else {
			memcpy(packet_buffer + saved_packet_size, ptr, iobyte);
			saved_packet_size += iobyte;
			iobyte = 0;
		}
	}
}

void clienterror()
{
	exit(-1);
}

LRESULT CALLBACK WindowProc(HWND hwnd,
	UINT msg,
	WPARAM wparam,
	LPARAM lparam)
{
	// this is the main message handler of the system
	PAINTSTRUCT	ps;		   // used in WM_PAINT
	HDC			hdc;	   // handle to a device context

	LRESULT		lResult = 0L;
	TCHAR		szConsoleTitle[256];
						   // what is the message 
	switch (msg)
	{
	case WM_KEYDOWN: 
	{
		{
			int x = 0, y = 0;
			if (wparam == VK_RIGHT)	x += 1;
			if (wparam == VK_LEFT)	x -= 1;
			if (wparam == VK_UP)	y -= 1;
			if (wparam == VK_DOWN)	y += 1;
			cs_packet_up *my_packet = reinterpret_cast<cs_packet_up *>(send_buffer);
			my_packet->size = sizeof(my_packet);
			send_wsabuf.len = sizeof(my_packet);
			DWORD iobyte;
			if (0 != x) {
				if (1 == x) my_packet->type = CS_RIGHT;
				else my_packet->type = CS_LEFT;
				int ret = WSASend(g_mysocket, &send_wsabuf, 1, &iobyte, 0, NULL, NULL);
				if (ret) {
					int error_code = WSAGetLastError();
					printf("Error while sending packet [%d]", error_code);
				}
			}
			if (0 != y) {
				if (1 == y) my_packet->type = CS_DOWN;
				else my_packet->type = CS_UP;
				WSASend(g_mysocket, &send_wsabuf, 1, &iobyte, 0, NULL, NULL);
			}
		}

		{
			if (wparam == VK_ESCAPE)
			{
				SendRequestPosSave(g_myid, player.x, player.y);
			}
			if(wparam == KEY_Z)
			{
				bool bHealPortion = false;
				int k = 0;
				for(int i=0;i<MAX_ITEMS;++i)
				{

					if(player.items[i].kind == HEALTH_PORTION_TEXTURE && player.items[i].blank == false)		//HEAL PORTION
					{
						k = i;
						bHealPortion = true;
					}
				}
				if (bHealPortion)
				{
					SendUseHealItemPacket(g_myid);
					player.items[k].blank = true;
					if (player.item_count > 0)
					{
						player.item_count -= 1;

					}
				}
			}
			if(wparam == KEY_X)
			{
				bool bSkillPortion=false;
				int k = 0;
				for(int i=0;i<MAX_ITEMS;++i)
				{
					if(player.items[i].kind == SKILL_PORTION_TEXTURE && player.items[i].blank == false)
					{
						k = i;
						bSkillPortion = true;
					}
				}
				if(bSkillPortion)
				{
					SendUseSkillItemPacket(g_myid);
					player.items[k].blank = true;
					if (player.item_count > 0)
					{
						player.item_count -= 1;

					}
				}
				
			}
			if(wparam == KEY_C)
			{
				bool bSpeedPortion = false;
				int k = 0;

				for(int i=0;i<MAX_ITEMS;++i)
				{
					if(player.items[i].kind == SPEED_PORTION_TEXTURE && player.items[i].blank == false)
					{
						k = i;
						bSpeedPortion = true;
					}

				}
				if(bSpeedPortion)
				{
					SendUseSpeedItemPacket(g_myid);
					player.items[k].blank = true;
					if (player.item_count > 0)
					{
						player.item_count -= 1;
					}
				}
			}
			if(wparam == KEY_A)
			{
				SendAttackPacket(g_myid);
			}

		}

		

		 break;
	}
				
	case WM_CREATE:
	{
		AllocConsole();
		SetConsoleTitle(TEXT("ID 받기용 콘솔"));

		_wfreopen(_T("CONOUT$"), _T("w"), stdout);
		_wfreopen(_T("CONIN$"), _T("r"), stdin);
		_wfreopen(_T("CONERR$"), _T("w"), stderr);
		// do initialization stuff here
		return(0);
	} break;

	case WM_PAINT:
	{
		// start painting
		hdc = BeginPaint(hwnd, &ps);

		// end painting
		EndPaint(hwnd, &ps);
		return(0);
	} break;
	case WM_CLOSE:
		FreeConsole();
		GetConsoleTitle(szConsoleTitle, sizeof(szConsoleTitle) / sizeof(TCHAR));
		lResult = DefWindowProc(hwnd, msg, wparam, lparam);
		break;
	case WM_DESTROY:
	{
	
		// kill the application			
		PostQuitMessage(0);
		return(0);
	} break;
	case WM_SOCKET:
	{
		if (WSAGETSELECTERROR(lparam)) {
			closesocket((SOCKET)wparam);
			clienterror();
			break;
		}
		switch (WSAGETSELECTEVENT(lparam)) {
		case FD_READ:
			ReadPacket((SOCKET)wparam);
			break;
		case FD_CLOSE:
			closesocket((SOCKET)wparam);
			clienterror();
			break;
		}
	}

	default:break;

	} // end switch

	  // process any messages that we didn't take care of 
	return (DefWindowProc(hwnd, msg, wparam, lparam));

} // end WinProc

  // WINMAIN ////////////////////////////////////////////////

int WINAPI WinMain(HINSTANCE hinstance,
	HINSTANCE hprevinstance,
	LPSTR lpcmdline,
	int ncmdshow)
{
	// this is the winmain function


	WNDCLASS winclass;	// this will hold the class we create
	HWND	 hwnd;		// generic window handle
	MSG		 msg;		// generic message


						// first fill in the window class stucture
	winclass.style = CS_DBLCLKS | CS_OWNDC |
		CS_HREDRAW | CS_VREDRAW;
	winclass.lpfnWndProc = WindowProc;
	winclass.cbClsExtra = 0;
	winclass.cbWndExtra = 0;
	winclass.hInstance = hinstance;
	winclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	winclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	winclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	winclass.lpszMenuName = NULL;
	winclass.lpszClassName = WINDOW_CLASS_NAME;

	// register the window class
	if (!RegisterClass(&winclass))
		return(0);

	// create the window, note the use of WS_POPUP
	if (!(hwnd = CreateWindow(WINDOW_CLASS_NAME, // class
		L"Chess Client",	 // title
		WS_OVERLAPPEDWINDOW | WS_VISIBLE,
		0, 0,	   // x,y
		WINDOW_WIDTH,  // width
		WINDOW_HEIGHT, // height
		NULL,	   // handle to parent 
		NULL,	   // handle to menu
		hinstance,// instance
		NULL)))	// creation parms
		return(0);

	// save the window handle and instance in a global
	main_window_handle = hwnd;
	main_instance = hinstance;

	// perform all game console specific initialization
	Game_Init();

	// enter main event loop
	while (1)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			// test if this is a quit
			if (msg.message == WM_QUIT)
				break;

			// translate any accelerator keys
			TranslateMessage(&msg);

			// send the message to the window proc
			DispatchMessage(&msg);
		} // end if

		  // main game processing goes here
		Game_Main();

	} // end while

	  // shutdown game and release all resources
	Game_Shutdown();

	// return to Windows like this
	return(msg.wParam);

} // end WinMain

  ///////////////////////////////////////////////////////////

  // WINX GAME PROGRAMMING CONSOLE FUNCTIONS ////////////////

int Game_Init(void *parms)
{
	// this function is where you do all the initialization 
	// for your game

	// set up screen dimensions
	screen_width = WINDOW_WIDTH;
	screen_height = WINDOW_HEIGHT;
	screen_bpp = 32;

	// initialize directdraw
	DD_Init(screen_width, screen_height, screen_bpp);


	// create and load the reactor bitmap image
	Create_Bitmap32(&reactor, 0, 0, 531, 532);
	Create_Bitmap32(&black_tile, 0, 0, 531, 532);
	Create_Bitmap32(&white_tile, 0, 0, 531, 532);
	Load_Image_Bitmap32(&reactor, L"CHESSMAP.BMP", 0, 0, BITMAP_EXTRACT_MODE_ABS);
	Load_Image_Bitmap32(&black_tile, L"CHESSMAP.BMP", 0, 0, BITMAP_EXTRACT_MODE_ABS);
	black_tile.x = 69;
	black_tile.y = 5;
	black_tile.height = TILE_HEIGHT;
	black_tile.width = TILE_WIDTH;
	Load_Image_Bitmap32(&white_tile, L"CHESSMAP.BMP", 0, 0, BITMAP_EXTRACT_MODE_ABS);
	white_tile.x = 5;
	white_tile.y = 5;
	white_tile.height = TILE_HEIGHT;
	white_tile.width = TILE_WIDTH;

	// now let's load in all the frames for the skelaton!!!

	Load_Texture(L"Player.PNG", UNIT_TEXTURE, 64, 64);

	if (!Create_BOB32(&player, 0, 0, 64, 64, 1, BOB_ATTR_SINGLE_FRAME)) return(0);
	Load_Frame_BOB32(&player, UNIT_TEXTURE, 0, 0, 0, BITMAP_EXTRACT_MODE_CELL);

	// set up stating state of skelaton
	Set_Animation_BOB32(&player, 0);
	Set_Anim_Speed_BOB32(&player, 4);
	Set_Vel_BOB32(&player, 0, 0);
	Set_Pos_BOB32(&player, 0, 0);
	Load_Texture(L"bear2.PNG", OBJECT_TEXTURE, 64, 49);


	for (int j = 0 ,k=0; j < 100; j+=8) 
	{
		for (int i = 0; i < 100; i+=8)
		{
			if (!Create_BOB32(&object[k], 0, 0, 64, 49, 1, BOB_ATTR_SINGLE_FRAME)) return (0);
			Load_Frame_BOB32(&object[k],OBJECT_TEXTURE, 0, 0, 0, BITMAP_EXTRACT_MODE_CELL);

			//	Set_Animation_BOB32(&object[k], 0);
			//	Set_Anim_Speed_BOB32(&object[k], 4);
				//Set_Vel_BOB32(&object[k], 0, 0);
			object[k].attr |= BOB_ATTR_VISIBLE;
	
			Set_Pos_BOB32(&object[k++],i ,  j);
		}
	}

	// create skelaton bob
	for (int i = 0; i < MAX_USER; ++i) {
		if (!Create_BOB32(&skelaton[i], 0, 0, 64, 64, 1, BOB_ATTR_SINGLE_FRAME))
			return(0);
		Load_Frame_BOB32(&skelaton[i], UNIT_TEXTURE, 0, 0, 0, BITMAP_EXTRACT_MODE_CELL);

		// set up stating state of skelaton
		Set_Animation_BOB32(&skelaton[i], 0);
		Set_Anim_Speed_BOB32(&skelaton[i], 4);
		Set_Vel_BOB32(&skelaton[i], 0, 0);
		Set_Pos_BOB32(&skelaton[i], 0, 0);
	}

	Load_Texture(L"monster1.PNG", NPC_TEXTURE, 64, 64);
	 // create skelaton bob
	for (int i = 0; i < NUM_NPC; ++i) {
		if (!Create_BOB32(&npc[i], 0, 0, 64, 64, 1, BOB_ATTR_SINGLE_FRAME))
			return(0);
		Load_Frame_BOB32(&npc[i], NPC_TEXTURE, 0, 0, 0, BITMAP_EXTRACT_MODE_CELL);

		// set up stating state of skelaton
		Set_Animation_BOB32(&npc[i], 0);
		Set_Anim_Speed_BOB32(&npc[i], 4);
		Set_Vel_BOB32(&npc[i], 0, 0);
		Set_Pos_BOB32(&npc[i], 0, 0);
		// Set_ID(&npc[i], i);
	}

	Load_Texture(L"Health portion.png", HEALTH_PORTION_TEXTURE, 64, 64);
	Load_Texture(L"skill_portion.png", SKILL_PORTION_TEXTURE, 64, 64);
	Load_Texture(L"speed_portion.png", SPEED_PORTION_TEXTURE, 64, 64);

	for(int i=0;i<NUM_ITEM -(NPC_ID_START+NUM_NPC);++i)
	{
		if (!Create_BOB32(&item[i], 0,0,64,64,1,BOB_ATTR_SINGLE_FRAME))
			return(0);

		Load_Frame_BOB32(&item[i], (i % 3) + HEALTH_PORTION_TEXTURE, 0, 0, 0, BITMAP_EXTRACT_MODE_CELL);

		Set_Animation_BOB32(&item[i], 0);
		Set_Anim_Speed_BOB32(&item[i], 4);
		Set_Vel_BOB32(&item[i], 0, 0);
		Set_Pos_BOB32(&item[i], 0, 0);
	
	}

	for(int i=0;i<inven_count;++i)
	{
		if (!Create_BOB32(&inven[i], 0, 0, 64, 64, 1, BOB_ATTR_SINGLE_FRAME))
			return 0;

		Load_Frame_BOB32(&inven[i], i + HEALTH_PORTION_TEXTURE, 0, 0, 0, BITMAP_EXTRACT_MODE_CELL);

		Set_Animation_BOB32(&inven[i], 0);
		Set_Anim_Speed_BOB32(&inven[i], 4);
		Set_Vel_BOB32(&inven[i], 0, 0);
		Set_Pos_BOB32(&inven[i], 0, 0);

		inven[i].attr |= BOB_ATTR_VISIBLE;
	}
	
	

	// set clipping rectangle to screen extents so mouse cursor
	// doens't mess up at edges
	//RECT screen_rect = {0,0,screen_width,screen_height};
	//lpddclipper = DD_Attach_Clipper(lpddsback,1,&screen_rect);

	// hide the mouse
	//ShowCursor(FALSE);


	WSADATA	wsadata;
	WSAStartup(MAKEWORD(2, 2), &wsadata);

	g_mysocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, 0);

	SOCKADDR_IN ServerAddr;
	ZeroMemory(&ServerAddr, sizeof(SOCKADDR_IN));
	ServerAddr.sin_family = AF_INET;
	ServerAddr.sin_port = htons(SERVER_PORT);
	ServerAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

	int Result = WSAConnect(g_mysocket, (sockaddr *)&ServerAddr, sizeof(ServerAddr), NULL, NULL, NULL, NULL);

	WSAAsyncSelect(g_mysocket, main_window_handle, WM_SOCKET, FD_CLOSE | FD_READ);

	send_wsabuf.buf = send_buffer;
	send_wsabuf.len = BUF_SIZE;
	recv_wsabuf.buf = recv_buffer;
	recv_wsabuf.len = BUF_SIZE;



	// return success
	return(1);

} // end Game_Init

  ///////////////////////////////////////////////////////////

int Game_Shutdown(void *parms)
{
	// this function is where you shutdown your game and
	// release all resources that you allocated

	// kill the reactor
	Destroy_Bitmap32(&black_tile);
	Destroy_Bitmap32(&white_tile);
	Destroy_Bitmap32(&reactor);

	// kill skelaton
	for (int i = 0; i < MAX_USER; ++i) Destroy_BOB32(&skelaton[i]);
	//for (int i = 0; i < MAX_NPC; ++i)
	//	Destroy_BOB32(&npc[i]);

	// shutdonw directdraw
	DD_Shutdown();

	WSACleanup();

	// return success
	return(1);
} // end Game_Shutdown

  ///////////////////////////////////////////////////////////

int Game_Main(void *parms)
{
	// this is the workhorse of your game it will be called
	// continuously in real-time this is like main() in C
	// all the calls for you game go here!
	// check of user is trying to exit
	if (KEY_DOWN(VK_ESCAPE) || KEY_DOWN(VK_SPACE))
		PostMessage(main_window_handle, WM_DESTROY, 0, 0);

	// start the timing clock
	Start_Clock();

	// clear the drawing surface
	DD_Fill_Surface(D3DCOLOR_ARGB(255, 0, 0, 0));

	// get player input

	g_pd3dDevice->BeginScene();
	g_pSprite->Begin(D3DXSPRITE_ALPHABLEND | D3DXSPRITE_SORT_TEXTURE);

	// draw the background reactor image
	for (int i = 0; i<20; ++i)
		for (int j = 0; j<20; ++j)
		{
			int tile_x = i + g_left_x;
			int tile_y = j + g_top_y;
			if ((tile_x <0) || (tile_y<0)) continue;
			if (((tile_x >> 2) % 2) == ((tile_y >> 2) % 2))
				Draw_Bitmap32(&white_tile, TILE_WIDTH * i + 7, TILE_HEIGHT * j + 7);
			else
				Draw_Bitmap32(&black_tile, TILE_WIDTH * i + 7, TILE_HEIGHT * j + 7);
		}
	//	Draw_Bitmap32(&reactor);

	g_pSprite->End();
	g_pSprite->Begin(D3DXSPRITE_ALPHABLEND | D3DXSPRITE_SORT_TEXTURE);


	// draw the skelaton
	Draw_BOB32(&player);
	for (int i = 0; i<MAX_USER; ++i) Draw_BOB32(&skelaton[i]);
	for (int i = 0; i<NUM_NPC; ++i) Draw_BOB32(&npc[i]);
	for (int i = 0; i < 300;++i) Draw_BOB32(&object[i]);
	for (int i = 0; i < NUM_ITEM - (NPC_ID_START + NUM_NPC); ++i)
	{
		Draw_BOB32(&item[i]);
	}
	for(int i=0;i<player.item_count;++i)
	{
		if (player.items[i].blank == false)
		{
			Set_Pos_BOB32(&inven[player.items[i].kind-HEALTH_PORTION_TEXTURE], player.x - (8 + i), player.y - 6);
			Draw_BOB32(&inven[player.items[i].kind-HEALTH_PORTION_TEXTURE]);
		}
	}
	// draw some text
	wchar_t text[300];
	wsprintf(text, L"MY POSITION (%3d, %3d)", player.x, player.y);
	Draw_Text_D3D(text, 10, screen_height - 64, D3DCOLOR_ARGB(255, 255, 255, 255));
	wsprintf(text, L"MY HP (%3d)", player.hp);
	Draw_Text_D3D(text, 300, screen_height - 64, D3DCOLOR_ARGB(255, 255, 255, 255));
	wsprintf(text, L"MY MP (%3d)", player.mp);
	Draw_Text_D3D(text, 450, screen_height - 64, D3DCOLOR_ARGB(255, 255, 255, 255));
	


	if(player.alive == false && g_gameStart == true)
	{
		wsprintf(text, L"YOU DEAD !!");
		Draw_Text_D3D(text, 300, 240, D3DCOLOR_ARGB(255, 0, 0, 255));
	}


	g_pSprite->End();
	g_pd3dDevice->EndScene();

	// flip the surfaces
	DD_Flip();

	// sync to 3o fps
	//Wait_Clock(30);


	// return success
	return(1);

} // end Game_Main

  //////////////////////////////////////////////////////////