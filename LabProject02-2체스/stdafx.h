// stdafx.h : ���� ��������� ���� ��������� �ʴ�
// ǥ�� �ý��� ���� ���� �� ������Ʈ ���� ���� ������
// ��� �ִ� ���� �����Դϴ�.
//

#pragma once

#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "windowscodecs.lib")

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // ���� ������ �ʴ� ������ Windows ������� �����մϴ�.
// Windows ��� ����:
#include <windows.h>

// C�� ��Ÿ�� ��� �����Դϴ�.
#include <stdlib.h>
#include <iostream>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

#include <vector>
#include <string>
#include <wrl.h>
#include <shellapi.h>

#include <d2d1_3.h>
#include <dwrite.h>
#include <dwrite_1.h>
#include <d3d11on12.h>
#include <d2d1_1helper.h>

#include <d2d1effects.h>
#include <wincodec.h>

#include <d3d12.h>
#include <dxgi1_4.h>
#include <DXGIDebug.h>

#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <DirectXColors.h>
#include <DirectXCollision.h>
#include <array>

#include "../����4 ����/Server/Server/Protocol.h"
#pragma comment(linker, "/entry:wWinMainCRTStartup /subsystem:console")


using namespace DirectX;
using namespace DirectX::PackedVector;

using Microsoft::WRL::ComPtr;

#define _CRTDBG_MAP_ALLOC

//#define FRAME_BUFFER_WIDTH				1200f
// FRAME_BUFFER_HEIGHT				800f
//constexpr int FRAME_BUFFER_WIDTH = 1200;

//constexpr int FRAME_BUFFER_HEIGHT		= 800;

//#define _WITH_SWAPCHAIN_FULLSCREEN_STATE

#define _WITH_DIRECT2D
#ifdef _WITH_DIRECT2D
//#define _WITH_DIRECT2D_IMAGE_EFFECT
#endif


