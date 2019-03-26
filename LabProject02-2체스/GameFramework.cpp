//-----------------------------------------------------------------------------
// File: CGameFramework.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "GameFramework.h"
#include "Network.h"

//#define _WITH_PRESENT_PARAMETERS
//#define _WITH_SYNCH_SWAPCHAIN

CGameFramework::CGameFramework()
{
	for (int i = 0; i < m_nSwapChainBuffers; i++)
	{
		m_ppd3dSwapChainBackBuffers[i] = NULL;
		m_ppd3dCommandAllocators[i] = NULL;
		m_pnFenceValues[i] = 0;
#ifdef _WITH_DIRECT2D
		m_ppd3d11WrappedBackBuffers[i] = NULL;
		m_ppd2dRenderTargets[i] = NULL;
#endif
	}
	/*m_offset = { 0.0f + (m_fWidthStep * 4.0f),0.0f + (m_fHeightStep * 4.0f) };*/
	m_rect = { 0.0f,0.0f,(float)m_fWidthStep,(float)m_fHeightStep };
	_tcscpy_s(m_pszFrameRate, _T("LabProject ("));
}

CGameFramework::~CGameFramework()
{
}

bool CGameFramework::OnCreate(HINSTANCE hInstance, HWND hMainWnd)
{
	m_hInstance = hInstance;
	m_hWnd = hMainWnd;

	CreateDirect3DDevice();
	CreateCommandQueueAndList();
#ifdef _WITH_DIRECT2D
	CreateDirect2DDevice();
#endif
	CreateRtvAndDsvDescriptorHeaps();
	CreateSwapChain();
	CreateDepthStencilView();

	m_hFenceEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);

	CreateNetworkSocket();

	BuildObjects();

	return(true);
}
void CGameFramework::CreateNetworkSocket()
{
	m_pNetwork = new CNetwork();

	m_pNetwork->Initialize();

}

void CGameFramework::CreateSwapChain()
{
#ifdef _WITH_CREATE_SWAPCHAIN_FOR_HWND
	DXGI_SWAP_CHAIN_DESC1 dxgiSwapChainDesc;
	::ZeroMemory(&dxgiSwapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC1));
	dxgiSwapChainDesc.Width = m_nWndClientWidth;
	dxgiSwapChainDesc.Height = m_nWndClientHeight;
	dxgiSwapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	dxgiSwapChainDesc.SampleDesc.Count = (m_bMsaa4xEnable) ? 4 : 1;
	dxgiSwapChainDesc.SampleDesc.Quality = (m_bMsaa4xEnable) ? (m_nMsaa4xQualityLevels - 1) : 0;
	dxgiSwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	dxgiSwapChainDesc.BufferCount = m_nSwapChainBuffers;
	dxgiSwapChainDesc.Scaling = DXGI_SCALING_NONE;
	dxgiSwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	dxgiSwapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
#ifdef _WITH_ONLY_RESIZE_BACKBUFFERS
	dxgiSwapChainDesc.Flags = 0;
#else
	dxgiSwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
#endif

	DXGI_SWAP_CHAIN_FULLSCREEN_DESC dxgiSwapChainFullScreenDesc;
	::ZeroMemory(&dxgiSwapChainFullScreenDesc, sizeof(DXGI_SWAP_CHAIN_FULLSCREEN_DESC));
	dxgiSwapChainFullScreenDesc.RefreshRate.Numerator = 60;
	dxgiSwapChainFullScreenDesc.RefreshRate.Denominator = 1;
	dxgiSwapChainFullScreenDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	dxgiSwapChainFullScreenDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	dxgiSwapChainFullScreenDesc.Windowed = TRUE;

	HRESULT hResult = m_pdxgiFactory->CreateSwapChainForHwnd(m_pd3dCommandQueue, m_hWnd, &dxgiSwapChainDesc, &dxgiSwapChainFullScreenDesc, NULL, (IDXGISwapChain1 **)&m_pdxgiSwapChain);
#else
	DXGI_SWAP_CHAIN_DESC dxgiSwapChainDesc;
	::ZeroMemory(&dxgiSwapChainDesc, sizeof(dxgiSwapChainDesc));
	dxgiSwapChainDesc.BufferCount = m_nSwapChainBuffers;
	dxgiSwapChainDesc.BufferDesc.Width = m_nWndClientWidth;
	dxgiSwapChainDesc.BufferDesc.Height = m_nWndClientHeight;
	dxgiSwapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	dxgiSwapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
	dxgiSwapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	dxgiSwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	dxgiSwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	dxgiSwapChainDesc.OutputWindow = m_hWnd;
	dxgiSwapChainDesc.SampleDesc.Count = (m_bMsaa4xEnable) ? 4 : 1;
	dxgiSwapChainDesc.SampleDesc.Quality = (m_bMsaa4xEnable) ? (m_nMsaa4xQualityLevels - 1) : 0;
	dxgiSwapChainDesc.Windowed = TRUE;
#ifdef _WITH_ONLY_RESIZE_BACKBUFFERS
	dxgiSwapChainDesc.Flags = 0;
#else
	dxgiSwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
#endif

	HRESULT hResult = m_pdxgiFactory->CreateSwapChain(m_pd3dCommandQueue, &dxgiSwapChainDesc, (IDXGISwapChain **)&m_pdxgiSwapChain);
#endif

	hResult = m_pdxgiFactory->MakeWindowAssociation(m_hWnd, DXGI_MWA_NO_ALT_ENTER);
	m_nSwapChainBufferIndex = m_pdxgiSwapChain->GetCurrentBackBufferIndex();

#ifndef _WITH_SWAPCHAIN_FULLSCREEN_STATE
	CreateRenderTargetViews();
#endif
}

void CGameFramework::CreateDirect3DDevice()
{
	HRESULT hResult;

	UINT nDXGIFactoryFlags = 0;
#if defined(_DEBUG)
	ID3D12Debug *pd3dDebugController = NULL;
	hResult = D3D12GetDebugInterface(__uuidof(ID3D12Debug), (void **)&pd3dDebugController);
	if (pd3dDebugController)
	{
		pd3dDebugController->EnableDebugLayer();
		pd3dDebugController->Release();
	}
	nDXGIFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
#endif

	hResult = ::CreateDXGIFactory2(nDXGIFactoryFlags, __uuidof(IDXGIFactory4), (void **)&m_pdxgiFactory);

	IDXGIAdapter1 *pd3dAdapter = NULL;
	for (UINT i = 0; DXGI_ERROR_NOT_FOUND != m_pdxgiFactory->EnumAdapters1(i, &pd3dAdapter); i++)
	{
		DXGI_ADAPTER_DESC1 dxgiAdapterDesc;
		pd3dAdapter->GetDesc1(&dxgiAdapterDesc);
		if (dxgiAdapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) continue;
		if (SUCCEEDED(D3D12CreateDevice(pd3dAdapter, D3D_FEATURE_LEVEL_12_0, _uuidof(ID3D12Device), (void **)&m_pd3d12Device))) break;
	}

	if (!pd3dAdapter)
	{
		m_pdxgiFactory->EnumWarpAdapter(_uuidof(IDXGIFactory4), (void **)&pd3dAdapter);
		hResult = D3D12CreateDevice(pd3dAdapter, D3D_FEATURE_LEVEL_12_0, _uuidof(ID3D12Device), (void **)&m_pd3d12Device);
	}

	if (pd3dAdapter) pd3dAdapter->Release();

#if defined(_DEBUG)
	ID3D12InfoQueue *pd3dInfoQueue = NULL;
	hResult = m_pd3d12Device->QueryInterface(IID_PPV_ARGS(&pd3dInfoQueue));

	//D3D12_MESSAGE_CATEGORY pd3dMessageCategories[] = { };
	D3D12_MESSAGE_SEVERITY pd3dMessageSeverities[1];
	pd3dMessageSeverities[0] = D3D12_MESSAGE_SEVERITY_INFO;

	D3D12_MESSAGE_ID pd3dMessageDenyIDs[1];
	pd3dMessageDenyIDs[0] = D3D12_MESSAGE_ID_INVALID_DESCRIPTOR_HANDLE;

	D3D12_INFO_QUEUE_FILTER d3dInfoQueueFilter = { };
	//d3dInfoQueueFilter.DenyList.NumCategories = _countof(pd3dMessageCategories);
	//d3dInfoQueueFilter.DenyList.pCategoryList = pd3dMessageCategories;
	d3dInfoQueueFilter.DenyList.NumSeverities = _countof(pd3dMessageSeverities);
	d3dInfoQueueFilter.DenyList.pSeverityList = pd3dMessageSeverities;
	d3dInfoQueueFilter.DenyList.NumIDs = _countof(pd3dMessageDenyIDs);
	d3dInfoQueueFilter.DenyList.pIDList = pd3dMessageDenyIDs;

	pd3dInfoQueue->PushStorageFilter(&d3dInfoQueueFilter);

	pd3dInfoQueue->Release();
#endif

	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS d3dMsaaQualityLevels;
	d3dMsaaQualityLevels.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	d3dMsaaQualityLevels.SampleCount = 4;
	d3dMsaaQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
	d3dMsaaQualityLevels.NumQualityLevels = 0;
	hResult = m_pd3d12Device->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &d3dMsaaQualityLevels, sizeof(D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS));
	m_nMsaa4xQualityLevels = d3dMsaaQualityLevels.NumQualityLevels;
	m_bMsaa4xEnable = (m_nMsaa4xQualityLevels > 1) ? true : false;

	hResult = m_pd3d12Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, __uuidof(ID3D12Fence), (void **)&m_pd3dFence);

	m_d3dViewport.TopLeftX = 0;
	m_d3dViewport.TopLeftY = 0;
	m_d3dViewport.Width = static_cast<float>(m_nWndClientWidth);
	m_d3dViewport.Height = static_cast<float>(m_nWndClientHeight);
	m_d3dViewport.MinDepth = 0.0f;
	m_d3dViewport.MaxDepth = 1.0f;

	m_d3dScissorRect = { 0, 0, m_nWndClientWidth, m_nWndClientHeight };
}

#define _WITH_DIRECT2D_IMAGE_EFFECT
#ifdef _WITH_DIRECT2D
void CGameFramework::CreateDirect2DDevice()
{
	UINT nD3D11DeviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#if defined(_DEBUG)
	nD3D11DeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	ID3D11Device *pd3d11Device = NULL;
	HRESULT hResult = ::D3D11On12CreateDevice(m_pd3d12Device, nD3D11DeviceFlags, NULL, 0, (IUnknown **)&m_pd3dCommandQueue, 1, 0, &pd3d11Device, &m_pd3d11DeviceContext, NULL);
	hResult = pd3d11Device->QueryInterface(__uuidof(ID3D11On12Device), (void **)&m_pd3d11On12Device);
	if (pd3d11Device) pd3d11Device->Release();

	D2D1_FACTORY_OPTIONS nD2DFactoryOptions = { D2D1_DEBUG_LEVEL_NONE };
#if defined(_DEBUG)
	nD2DFactoryOptions.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
#endif
	hResult = ::D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, __uuidof(ID2D1Factory3), &nD2DFactoryOptions, (void **)&m_pd2dFactory);

	IDXGIDevice *pdxgiDevice = NULL;
	hResult = m_pd3d11On12Device->QueryInterface(__uuidof(IDXGIDevice), (void **)&pdxgiDevice);
	hResult = m_pd2dFactory->CreateDevice(pdxgiDevice, &m_pd2dDevice);
	hResult = m_pd2dDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &m_pd2dDeviceContext);
//	hResult = ::DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), (IUnknown **)&m_pdWriteFactory);

	if (pdxgiDevice) pdxgiDevice->Release();

	m_pd2dDeviceContext->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_GRAYSCALE);

	
	m_pd2dDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF(D2D1::ColorF::White, 1.0f)), &m_pd2dChessBoardWhite);
	m_pd2dDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF(D2D1::ColorF::Black, 1.0f)), &m_pd2dChessBoardBlack);

	m_pd2dDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF(D2D1::ColorF::Black, 1.0f)), &m_pd2dLine);

	

#ifdef _WITH_DIRECT2D_IMAGE_EFFECT
	CoInitialize(NULL);
	hResult = ::CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, __uuidof(IWICImagingFactory), (void **)&m_pwicImagingFactory);

	hResult = m_pd2dFactory->CreateDrawingStateBlock(&m_pd2dsbDrawingState);
	hResult = m_pd2dDeviceContext->CreateEffect(CLSID_D2D1BitmapSource, &m_pd2dfxBitmapSource);


	IWICBitmapDecoder *pwicBitmapDecoder;
	hResult = m_pwicImagingFactory->CreateDecoderFromFilename(L"Image/Pawn.png", NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pwicBitmapDecoder);
	IWICBitmapFrameDecode *pwicFrameDecode;
	pwicBitmapDecoder->GetFrame(0, &pwicFrameDecode);
	m_pwicImagingFactory->CreateFormatConverter(&m_pwicFormatConverter);
	m_pwicFormatConverter->Initialize(pwicFrameDecode, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 0.0f, WICBitmapPaletteTypeCustom);
	m_pd2dfxBitmapSource->SetValue(D2D1_BITMAPSOURCE_PROP_WIC_BITMAP_SOURCE, m_pwicFormatConverter);
	
	//이미지 크기 조정
	D2D1_VECTOR_2F vec{ 0.135f,0.145f };
	m_pd2dfxBitmapSource->SetValue(D2D1_BITMAPSOURCE_PROP_SCALE, vec);

	if (pwicBitmapDecoder) pwicBitmapDecoder->Release();
	if (pwicFrameDecode) pwicFrameDecode->Release();
#endif
}
#endif

void CGameFramework::CreateCommandQueueAndList()
{
	D3D12_COMMAND_QUEUE_DESC d3dCommandQueueDesc;
	::ZeroMemory(&d3dCommandQueueDesc, sizeof(D3D12_COMMAND_QUEUE_DESC));
	d3dCommandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	d3dCommandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	HRESULT hResult = m_pd3d12Device->CreateCommandQueue(&d3dCommandQueueDesc, _uuidof(ID3D12CommandQueue), (void **)&m_pd3dCommandQueue);

	for (UINT i = 0; i < m_nSwapChainBuffers; i++) hResult = m_pd3d12Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, __uuidof(ID3D12CommandAllocator), (void **)&m_ppd3dCommandAllocators[i]);

	hResult = m_pd3d12Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_ppd3dCommandAllocators[0], NULL, __uuidof(ID3D12GraphicsCommandList), (void **)&m_pd3dCommandList);
	hResult = m_pd3dCommandList->Close();
}

void CGameFramework::CreateRtvAndDsvDescriptorHeaps()
{
	D3D12_DESCRIPTOR_HEAP_DESC d3dDescriptorHeapDesc;
	::ZeroMemory(&d3dDescriptorHeapDesc, sizeof(D3D12_DESCRIPTOR_HEAP_DESC));
	d3dDescriptorHeapDesc.NumDescriptors = m_nSwapChainBuffers;
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	d3dDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	d3dDescriptorHeapDesc.NodeMask = 0;
	HRESULT hResult = m_pd3d12Device->CreateDescriptorHeap(&d3dDescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void **)&m_pd3dRtvDescriptorHeap);
	m_nRtvDescriptorIncrementSize = m_pd3d12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	d3dDescriptorHeapDesc.NumDescriptors = 1;
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	hResult = m_pd3d12Device->CreateDescriptorHeap(&d3dDescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void **)&m_pd3dDsvDescriptorHeap);
	m_nDsvDescriptorIncrementSize = m_pd3d12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
}

void CGameFramework::CreateRenderTargetViews()
{
	D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle = m_pd3dRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	for (UINT i = 0; i < m_nSwapChainBuffers; i++)
	{
		m_pdxgiSwapChain->GetBuffer(i, __uuidof(ID3D12Resource), (void **)&m_ppd3dSwapChainBackBuffers[i]);
		m_pd3d12Device->CreateRenderTargetView(m_ppd3dSwapChainBackBuffers[i], NULL, d3dRtvCPUDescriptorHandle);
		d3dRtvCPUDescriptorHandle.ptr += m_nRtvDescriptorIncrementSize;
	}
#ifdef _WITH_DIRECT2D
	float fxDPI, fyDPI;
	m_pd2dFactory->GetDesktopDpi(&fxDPI, &fyDPI); //UINT GetDpiForWindow(HWND hwnd);

	D2D1_BITMAP_PROPERTIES1 d2dBitmapProperties = D2D1::BitmapProperties1(D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW, D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED), fxDPI, fyDPI);

	for (UINT i = 0; i < m_nSwapChainBuffers; i++)
	{
		D3D11_RESOURCE_FLAGS d3d11Flags = { D3D11_BIND_RENDER_TARGET };
		m_pd3d11On12Device->CreateWrappedResource(m_ppd3dSwapChainBackBuffers[i], &d3d11Flags, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT, __uuidof(ID3D11Resource), (void **)&m_ppd3d11WrappedBackBuffers[i]);


		IDXGISurface *pdxgiSurface = NULL;
		m_ppd3d11WrappedBackBuffers[i]->QueryInterface(__uuidof(IDXGISurface), (void **)&pdxgiSurface);
		m_pd2dDeviceContext->CreateBitmapFromDxgiSurface(pdxgiSurface, &d2dBitmapProperties, &m_ppd2dRenderTargets[i]);
		if (pdxgiSurface) pdxgiSurface->Release();
	}


#endif
}

void CGameFramework::CreateDepthStencilView()
{
	D3D12_RESOURCE_DESC d3dResourceDesc;
	d3dResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	d3dResourceDesc.Alignment = 0;
	d3dResourceDesc.Width = m_nWndClientWidth;
	d3dResourceDesc.Height = m_nWndClientHeight;
	d3dResourceDesc.DepthOrArraySize = 1;
	d3dResourceDesc.MipLevels = 1;
	d3dResourceDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	d3dResourceDesc.SampleDesc.Count = (m_bMsaa4xEnable) ? 4 : 1;
	d3dResourceDesc.SampleDesc.Quality = (m_bMsaa4xEnable) ? (m_nMsaa4xQualityLevels - 1) : 0;
	d3dResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	d3dResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_HEAP_PROPERTIES d3dHeapProperties;
	::ZeroMemory(&d3dHeapProperties, sizeof(D3D12_HEAP_PROPERTIES));
	d3dHeapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
	d3dHeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	d3dHeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	d3dHeapProperties.CreationNodeMask = 1;
	d3dHeapProperties.VisibleNodeMask = 1;

	D3D12_CLEAR_VALUE d3dClearValue;
	d3dClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	d3dClearValue.DepthStencil.Depth = 1.0f;
	d3dClearValue.DepthStencil.Stencil = 0;
	m_pd3d12Device->CreateCommittedResource(&d3dHeapProperties, D3D12_HEAP_FLAG_NONE, &d3dResourceDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &d3dClearValue, __uuidof(ID3D12Resource), (void **)&m_pd3dDepthStencilBuffer);

	D3D12_CPU_DESCRIPTOR_HANDLE d3dDsvCPUDescriptorHandle = m_pd3dDsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	m_pd3d12Device->CreateDepthStencilView(m_pd3dDepthStencilBuffer, NULL, d3dDsvCPUDescriptorHandle);

	D3D12_RESOURCE_BARRIER d3dResourceBarrier;
	::ZeroMemory(&d3dResourceBarrier, sizeof(D3D12_RESOURCE_BARRIER));
	d3dResourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	d3dResourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	d3dResourceBarrier.Transition.pResource = m_pd3dDepthStencilBuffer;
	d3dResourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COMMON;
	d3dResourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_DEPTH_WRITE;
	d3dResourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
//	m_pd3dCommandList->ResourceBarrier(1, &d3dResourceBarrier);
}

void CGameFramework::ChangeSwapChainState()
{
	WaitForGpuComplete();

	BOOL bFullScreenState = FALSE;
	HRESULT hResult = m_pdxgiSwapChain->GetFullscreenState(&bFullScreenState, NULL);
	hResult = m_pdxgiSwapChain->SetFullscreenState(!bFullScreenState, NULL);

	DXGI_MODE_DESC dxgiTargetParameters;
	dxgiTargetParameters.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	dxgiTargetParameters.Width = m_nWndClientWidth;
	dxgiTargetParameters.Height = m_nWndClientHeight;
	dxgiTargetParameters.RefreshRate.Numerator = 60;
	dxgiTargetParameters.RefreshRate.Denominator = 1;
	dxgiTargetParameters.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	dxgiTargetParameters.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	hResult = m_pdxgiSwapChain->ResizeTarget(&dxgiTargetParameters);

	for (int i = 0; i < m_nSwapChainBuffers; i++)
	{
		if (m_ppd3dSwapChainBackBuffers[i]) m_ppd3dSwapChainBackBuffers[i]->Release();
		if (m_ppd3d11WrappedBackBuffers[i]) m_ppd3d11WrappedBackBuffers[i]->Release();
		if (m_ppd2dRenderTargets[i]) m_ppd2dRenderTargets[i]->Release();
	}

	DXGI_SWAP_CHAIN_DESC dxgiSwapChainDesc;
	m_pdxgiSwapChain->GetDesc(&dxgiSwapChainDesc);
	hResult = m_pdxgiSwapChain->ResizeBuffers(m_nSwapChainBuffers, m_nWndClientWidth, m_nWndClientHeight, dxgiSwapChainDesc.BufferDesc.Format, dxgiSwapChainDesc.Flags);

	m_nSwapChainBufferIndex = m_pdxgiSwapChain->GetCurrentBackBufferIndex();

	CreateRenderTargetViews();
}

void CGameFramework::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
        case WM_LBUTTONDOWN:
        case WM_RBUTTONDOWN:
			break;
        case WM_LBUTTONUP:
        case WM_RBUTTONUP:
			break;
        case WM_MOUSEMOVE:
			break;
		default:
			break;
	}
}

void CGameFramework::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{

	switch (nMessageID)
	{

        case WM_KEYUP:
			switch (wParam) 
            {
				case VK_ESCAPE:
					::PostQuitMessage(0);
                    break;
                case VK_RETURN:
                    break;
				case VK_LEFT:
					if (m_pNetwork)
					{
						CS_RUN cs_runPacket = m_pNetwork->getCSRunPacket();
						cs_runPacket.key = KEY_LEFT;
						m_pNetwork->SetCSRunPacket(cs_runPacket);
					}
					break;
				case VK_RIGHT:
					if (m_pNetwork)
					{
						CS_RUN cs_runPacket = m_pNetwork->getCSRunPacket();
						cs_runPacket.key = KEY_RIGHT;
						m_pNetwork->SetCSRunPacket(cs_runPacket);
					}
					break;
				case VK_UP:
					if (m_pNetwork)
					{
						CS_RUN cs_runPacket = m_pNetwork->getCSRunPacket();
						cs_runPacket.key = KEY_UP;
						m_pNetwork->SetCSRunPacket(cs_runPacket);
					}
					break;
				case VK_DOWN:
					if (m_pNetwork)
					{
						CS_RUN cs_runPacket = m_pNetwork->getCSRunPacket();
						cs_runPacket.key = KEY_DOWN;
						m_pNetwork->SetCSRunPacket(cs_runPacket);
					}
					break;

				case VK_F9:
					ChangeSwapChainState();
					break;
				default:
					
					break;
			} 
			break;
		default:
			break;
	}
}

LRESULT CALLBACK CGameFramework::OnProcessingWindowMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
		case WM_ACTIVATE:
		{
			if (LOWORD(wParam) == WA_INACTIVE) {
				m_GameTimer.Stop();
			}
			else if(LOWORD(wParam) ==WA_CLICKACTIVE)
			{
				m_GameTimer.Start();
				
			}
			else{
				m_GameTimer.Start();
			}
			break;
		}
		case WM_SIZE:
			break;
        case WM_LBUTTONDOWN:
        case WM_RBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_RBUTTONUP:
        case WM_MOUSEMOVE:
			OnProcessingMouseMessage(hWnd, nMessageID, wParam, lParam);
            break;
        case WM_KEYDOWN:
        case WM_KEYUP:
			OnProcessingKeyboardMessage(hWnd, nMessageID, wParam, lParam);
			break;
	}
	return(0);
}

void CGameFramework::OnDestroy()
{
    ReleaseObjects();

	::CloseHandle(m_hFenceEvent);

	

#ifdef _WITH_DIRECT2D
//	if (m_pd2dbrBackground) m_pd2dbrBackground->Release();
//	if (m_pd2dbrBorder) m_pd2dbrBorder->Release();
	//if (m_pdwFont) m_pdwFont->Release();
	//if (m_pdwTextLayout) m_pdwTextLayout->Release();
	//if (m_pd2dbrText) m_pd2dbrText->Release();
	if (m_pd2dChessBoardBlack) m_pd2dChessBoardBlack->Release();
	if (m_pd2dChessBoardWhite) m_pd2dChessBoardWhite->Release();
	if (m_pd2dLine) m_pd2dLine->Release();
#ifdef _WITH_DIRECT2D_IMAGE_EFFECT
	if (m_pd2dfxBitmapSource) m_pd2dfxBitmapSource->Release();

	if (m_pd2dsbDrawingState) m_pd2dsbDrawingState->Release();
	if (m_pwicFormatConverter) m_pwicFormatConverter->Release();
	if (m_pwicImagingFactory) m_pwicImagingFactory->Release();
#endif

	if (m_pd2dDeviceContext) m_pd2dDeviceContext->Release();
	if (m_pd2dDevice) m_pd2dDevice->Release();
	//if (m_pdWriteFactory) m_pdWriteFactory->Release();
	if (m_pd3d11On12Device) m_pd3d11On12Device->Release();
	if (m_pd3d11DeviceContext) m_pd3d11DeviceContext->Release();
	if (m_pd2dFactory) m_pd2dFactory->Release();

	for (int i = 0; i < m_nSwapChainBuffers; i++)
	{
		if (m_ppd3d11WrappedBackBuffers[i]) m_ppd3d11WrappedBackBuffers[i]->Release();
		if (m_ppd2dRenderTargets[i]) m_ppd2dRenderTargets[i]->Release();
	}
#endif

	for (int i = 0; i < m_nSwapChainBuffers; i++)
	{
		if (m_ppd3dSwapChainBackBuffers[i]) m_ppd3dSwapChainBackBuffers[i]->Release();
		if (m_ppd3dCommandAllocators[i]) m_ppd3dCommandAllocators[i]->Release();
	}
	if (m_pd3dDepthStencilBuffer) m_pd3dDepthStencilBuffer->Release();
	if (m_pd3dDsvDescriptorHeap) m_pd3dDsvDescriptorHeap->Release();
	if (m_pd3dRtvDescriptorHeap) m_pd3dRtvDescriptorHeap->Release();

	if (m_pd3dCommandQueue) m_pd3dCommandQueue->Release();
	if (m_pd3dCommandList) m_pd3dCommandList->Release();

	if (m_pd3dFence) m_pd3dFence->Release();

	if (m_pdxgiSwapChain) m_pdxgiSwapChain->SetFullscreenState(FALSE, NULL);
	if (m_pdxgiSwapChain) m_pdxgiSwapChain->Release();
    if (m_pd3d12Device) m_pd3d12Device->Release();
	if (m_pdxgiFactory) m_pdxgiFactory->Release();


	for(int i=0;i<m_vecRect.size();i++)
	{
		m_vecRect.pop_back();
	}
	m_vecRect.clear();
#if defined(_DEBUG)
	IDXGIDebug1	*pdxgiDebug = NULL;
	DXGIGetDebugInterface1(0, __uuidof(IDXGIDebug1), (void **)&pdxgiDebug);
	HRESULT hResult = pdxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_DETAIL);
	pdxgiDebug->Release();
#endif
	if (m_pNetwork) m_pNetwork->Destroy();
	
}

void CGameFramework::BuildChessBoard()
{
	for (int j = 0; j < 8; j++) 
	{
		for (int i = 0; i < 8; i++)
		{
			if (j % 2 == 0)
			{
				if (i % 2 == 0) {
					m_rect.left = m_fWidthStep * i;
					m_rect.right = m_fWidthStep *( i + 1);
					m_vecRect.emplace_back(m_rect);
				}
			}
			else if (j % 2 == 1)
			{
				if( i % 2 == 0)
				{
					m_rect.left = m_fWidthStep * (i+1);
					m_rect.right= m_fWidthStep * (i + 2);
					m_vecRect.emplace_back(m_rect);
				}
				
			}
			
		}
		m_rect.top += m_fHeightStep;
		m_rect.bottom += m_fHeightStep;


	}
}
void CGameFramework::BuildObjects()
{
	m_pScene = new CScene();
	if (m_pScene) m_pScene->BuildObjects(m_pd3d12Device);

	BuildChessBoard();

	m_GameTimer.Reset();
}

void CGameFramework::ReleaseObjects()
{
	if (m_pScene) m_pScene->ReleaseObjects();
	if (m_pScene) delete m_pScene;
}

void CGameFramework::ProcessInput()
{
	//if (m_pNetwork) {
	//	CS_RUN cs_runPacket = m_pNetwork->getCSRunPacket();
	//	
	//	cs_runPacket.key = KEY_IDLE;
	//
	//	if(GetAsyncKeyState(VK_UP) & 0x0001)
	//	{
	//		cs_runPacket.key = KEY_UP;
	//		m_pNetwork->SetCSRunPacket(cs_runPacket);
	//		//cs_runPacket.player
	//	}
	//	if(GetAsyncKeyState(VK_DOWN) & 0x0001)
	//	{
	//		cs_runPacket.key = KEY_DOWN;
	//		m_pNetwork->SetCSRunPacket(cs_runPacket);
	//	}
	//	if(GetAsyncKeyState(VK_RIGHT) & 0x0001)
	//	{
	//		cs_runPacket.key = KEY_RIGHT;
	//		m_pNetwork->SetCSRunPacket(cs_runPacket);
	//	}
	//	if(GetAsyncKeyState(VK_LEFT) & 0x0001)
	//	{
	//		cs_runPacket.key = KEY_LEFT;
	//		m_pNetwork->SetCSRunPacket(cs_runPacket);
	//	}
	//}
}
void CGameFramework::CommunicateServer()
{
	if(m_pNetwork)
	{
		m_pNetwork->SendPacket();
		m_pNetwork->RecvPacket();
	}
}

void CGameFramework::AnimateObjects()
{

}

void CGameFramework::WaitForGpuComplete()
{
	HRESULT hResult = m_pd3dCommandQueue->Signal(m_pd3dFence, m_pnFenceValues[m_nSwapChainBufferIndex]);

	hResult = m_pd3dFence->SetEventOnCompletion(m_pnFenceValues[m_nSwapChainBufferIndex], m_hFenceEvent);
	::WaitForSingleObject(m_hFenceEvent, INFINITE);

	m_pnFenceValues[m_nSwapChainBufferIndex]++;
}

void CGameFramework::MoveToNextFrame()
{
	UINT64 nFenceValue = m_pnFenceValues[m_nSwapChainBufferIndex];
	HRESULT hResult = m_pd3dCommandQueue->Signal(m_pd3dFence, nFenceValue);

	m_nSwapChainBufferIndex = m_pdxgiSwapChain->GetCurrentBackBufferIndex();

	if (m_pd3dFence->GetCompletedValue() < m_pnFenceValues[m_nSwapChainBufferIndex])
	{
		hResult = m_pd3dFence->SetEventOnCompletion(m_pnFenceValues[m_nSwapChainBufferIndex], m_hFenceEvent);
		::WaitForSingleObject(m_hFenceEvent, INFINITE);
	}
	m_pnFenceValues[m_nSwapChainBufferIndex] = nFenceValue + 1;
}

void CGameFramework::DrawChessBoard(D2D1_SIZE_F &szRenderTarget,D2D1_POINT_2F& Point1,D2D1_POINT_2F& Point2,const int& fHeightStep,const int& fWidthStep)
{

	for(auto iter = m_vecRect.begin();iter != m_vecRect.end();iter++)
	{
		D2D1_RECT_F nRect = *iter;
		m_pd2dDeviceContext->FillRectangle(&nRect, m_pd2dChessBoardWhite);
		
	}
	
}


void CGameFramework::FrameAdvance()
{ 
	m_GameTimer.Tick(60.0f);

	//ProcessInput();

	CommunicateServer();
    //AnimateObjects();

	HRESULT hResult = m_ppd3dCommandAllocators[m_nSwapChainBufferIndex]->Reset();
	hResult = m_pd3dCommandList->Reset(m_ppd3dCommandAllocators[m_nSwapChainBufferIndex], NULL);

	D3D12_RESOURCE_BARRIER d3dResourceBarrier;
	::ZeroMemory(&d3dResourceBarrier, sizeof(D3D12_RESOURCE_BARRIER));
	d3dResourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	d3dResourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	d3dResourceBarrier.Transition.pResource = m_ppd3dSwapChainBackBuffers[m_nSwapChainBufferIndex];
	d3dResourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	d3dResourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	d3dResourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	m_pd3dCommandList->ResourceBarrier(1, &d3dResourceBarrier);

	m_pd3dCommandList->RSSetViewports(1, &m_d3dViewport);
	m_pd3dCommandList->RSSetScissorRects(1, &m_d3dScissorRect);

	D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle = m_pd3dRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	d3dRtvCPUDescriptorHandle.ptr += (m_nSwapChainBufferIndex * m_nRtvDescriptorIncrementSize);

	D3D12_CPU_DESCRIPTOR_HANDLE d3dDsvCPUDescriptorHandle = m_pd3dDsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

	float pfClearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f };
	m_pd3dCommandList->ClearRenderTargetView(d3dRtvCPUDescriptorHandle, pfClearColor/*Colors::Azure*/, 0, NULL);

	m_pd3dCommandList->ClearDepthStencilView(d3dDsvCPUDescriptorHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, NULL);

	m_pd3dCommandList->OMSetRenderTargets(1, &d3dRtvCPUDescriptorHandle, TRUE, &d3dDsvCPUDescriptorHandle);

	//if (m_pScene) m_pScene->Render(m_pd3dCommandList);

#ifndef _WITH_DIRECT2D
	d3dResourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	d3dResourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	d3dResourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	m_pd3dCommandList->ResourceBarrier(1, &d3dResourceBarrier);
#endif

	hResult = m_pd3dCommandList->Close();

	ID3D12CommandList *ppd3dCommandLists[] = { m_pd3dCommandList };
	m_pd3dCommandQueue->ExecuteCommandLists(1, ppd3dCommandLists);

	WaitForGpuComplete();

#ifdef _WITH_DIRECT2D
//	//Direct2D Drawing
	m_pd3d11On12Device->AcquireWrappedResources(&m_ppd3d11WrappedBackBuffers[m_nSwapChainBufferIndex], 1);
	m_pd2dDeviceContext->SetTarget(m_ppd2dRenderTargets[m_nSwapChainBufferIndex]);

	m_pd2dDeviceContext->BeginDraw();
//
	m_pd2dDeviceContext->SetTransform(D2D1::Matrix3x2F::Identity());
//

	D2D1_SIZE_F szRenderTarget = m_ppd2dRenderTargets[m_nSwapChainBufferIndex]->GetSize();
//
	D2D1_POINT_2F Point1 = { 0.0f,0.0f };
	D2D1_POINT_2F Point2 = { szRenderTarget.width - 10.0f, 10.0f };
//
	DrawChessBoard(szRenderTarget,Point1,Point2,m_fHeightStep,m_fWidthStep);
	if (m_pNetwork) {
		const SC_RUN& sc_runPacket = m_pNetwork->getSCRunPacket();
		
		for(int i=0;i<m_pNetwork->GetClientNum();++i)
		{
			m_offset.x = sc_runPacket.player[i].position.x;
			m_offset.y = sc_runPacket.player[i].position.y;

			m_pd2dDeviceContext->DrawImage(m_pd2dfxBitmapSource, &m_offset);
		}
		

		
		

	}

	//std::cout << m_pNetwork->GetClientNum()<<"\n";
	m_pd2dDeviceContext->EndDraw();

	m_pd3d11On12Device->ReleaseWrappedResources(&m_ppd3d11WrappedBackBuffers[m_nSwapChainBufferIndex], 1);

	m_pd3d11DeviceContext->Flush();
#endif

#ifdef _WITH_PRESENT_PARAMETERS
	DXGI_PRESENT_PARAMETERS dxgiPresentParameters;
	dxgiPresentParameters.DirtyRectsCount = 0;
	dxgiPresentParameters.pDirtyRects = NULL;
	dxgiPresentParameters.pScrollRect = NULL;
	dxgiPresentParameters.pScrollOffset = NULL;
	m_pdxgiSwapChain->Present1(1, 0, &dxgiPresentParameters);
#else
#ifdef _WITH_SYNCH_SWAPCHAIN
	m_pdxgiSwapChain->Present(1, 0);
#else
	m_pdxgiSwapChain->Present(0, 0);
#endif
#endif

	MoveToNextFrame();

	m_GameTimer.GetFrameRate(m_pszFrameRate + 12, 37);
	::SetWindowText(m_hWnd, m_pszFrameRate);
}


