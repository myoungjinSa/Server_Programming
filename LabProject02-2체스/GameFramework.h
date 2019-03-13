#pragma once

#include "Timer.h"
#include "Scene.h"


class CNetwork;
#define	_WITH_DIRECT2D_IMAGE_EFFECT
class CGameFramework
{
public:
	CGameFramework();
	~CGameFramework();

	bool OnCreate(HINSTANCE hInstance, HWND hMainWnd);
	void OnDestroy();

	void CreateSwapChain();
	void CreateRtvAndDsvDescriptorHeaps();
	void CreateCommandQueueAndList();
	void CreateDirect3DDevice();
#ifdef _WITH_DIRECT2D
	void CreateDirect2DDevice();
#endif

	void DrawChessBoard(D2D1_SIZE_F &,D2D1_POINT_2F &,D2D1_POINT_2F&,const int&,const int&);
	void CreateRenderTargetViews();
	void CreateDepthStencilView();

	void ChangeSwapChainState();

	void CreateNetworkSocket();
	void BuildChessBoard();
	void BuildObjects();
	void ReleaseObjects();

	void ProcessInput();
	void CommunicateServer();
	void AnimateObjects();
	void FrameAdvance();



	void WaitForGpuComplete();
	void MoveToNextFrame();

	void OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	void OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	LRESULT CALLBACK OnProcessingWindowMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);

private:
	HINSTANCE					m_hInstance = NULL;
	HWND						m_hWnd = NULL;

	int							m_nWndClientWidth = FRAME_BUFFER_WIDTH;
	int							m_nWndClientHeight = FRAME_BUFFER_HEIGHT;

	IDXGIFactory4				*m_pdxgiFactory = NULL;
	IDXGISwapChain3				*m_pdxgiSwapChain = NULL;
	ID3D12Device				*m_pd3d12Device = NULL;

	bool						m_bMsaa4xEnable = false;
	UINT						m_nMsaa4xQualityLevels = 0;

	static const UINT			m_nSwapChainBuffers = 2;
	UINT						m_nSwapChainBufferIndex = 0;

	ID3D12Resource				*m_ppd3dSwapChainBackBuffers[m_nSwapChainBuffers];
	ID3D12DescriptorHeap		*m_pd3dRtvDescriptorHeap = NULL;
	UINT						m_nRtvDescriptorIncrementSize = 0;

	ID3D12Resource				*m_pd3dDepthStencilBuffer = NULL;
	ID3D12DescriptorHeap		*m_pd3dDsvDescriptorHeap = NULL;
	UINT						m_nDsvDescriptorIncrementSize = 0;

	ID3D12CommandAllocator		*m_ppd3dCommandAllocators[m_nSwapChainBuffers];
	ID3D12CommandQueue			*m_pd3dCommandQueue = NULL;
	ID3D12GraphicsCommandList	*m_pd3dCommandList = NULL;

	ID3D12Fence					*m_pd3dFence = NULL;
	UINT64						m_pnFenceValues[m_nSwapChainBuffers];
	HANDLE						m_hFenceEvent = NULL;

	D3D12_VIEWPORT				m_d3dViewport;
	D3D12_RECT					m_d3dScissorRect;

#ifdef _WITH_DIRECT2D
	ID3D11On12Device			*m_pd3d11On12Device = NULL;
	ID3D11DeviceContext			*m_pd3d11DeviceContext = NULL;
	ID2D1Factory3				*m_pd2dFactory = NULL;
	//IDWriteFactory				*m_pdWriteFactory = NULL;
	ID2D1Device2				*m_pd2dDevice = NULL;
	ID2D1DeviceContext2			*m_pd2dDeviceContext = NULL;

	ID3D11Resource				*m_ppd3d11WrappedBackBuffers[m_nSwapChainBuffers];
	ID2D1Bitmap1				*m_ppd2dRenderTargets[m_nSwapChainBuffers];

	//ID2D1SolidColorBrush		*m_pd2dbrBackground = NULL;
	//ID2D1SolidColorBrush		*m_pd2dbrBorder = NULL;
	//IDWriteTextFormat			*m_pdwFont = NULL;
	//IDWriteTextLayout			*m_pdwTextLayout = NULL;
	//ID2D1SolidColorBrush		*m_pd2dbrText = NULL;

	ID2D1SolidColorBrush		*m_pd2dChessBoardWhite = NULL;
	ID2D1SolidColorBrush		*m_pd2dChessBoardBlack = NULL;
	ID2D1SolidColorBrush		*m_pd2dLine = NULL;

#ifdef _WITH_DIRECT2D_IMAGE_EFFECT
	IWICImagingFactory			*m_pwicImagingFactory = NULL;
	ID2D1Effect					*m_pd2dfxBitmapSource = NULL;

	std::vector<D2D1_RECT_F>	m_vecRect;
	D2D1_RECT_F					m_rect;
	D2D1_POINT_2F				m_offset;
	const int m_fWidthStep		= FRAME_BUFFER_WIDTH  / 8;
	const int m_fHeightStep		= FRAME_BUFFER_HEIGHT / 8;

	ID2D1DrawingStateBlock1		*m_pd2dsbDrawingState = NULL;
	IWICFormatConverter			*m_pwicFormatConverter = NULL;
#endif
#endif

	CScene						*m_pScene = NULL;

	CGameTimer					m_GameTimer;
	_TCHAR						m_pszFrameRate[50];



	//////////////////////////////////////////////////////////
	//Network
	CNetwork *m_pNetwork{nullptr};
};
