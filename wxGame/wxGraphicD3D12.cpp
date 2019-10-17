#pragma once
#include "stdafx.h"
#include "wxGraphicD3D12.h"
#include "SceneManager.h"
#include <DirectXPackedVector.h>
#define ASSET_DIRECTORY	"../wxAsset/"

using namespace DirectX::PackedVector;
static std::vector<std::string> ResourceType = { MATCH_TEXTURE, MATCH_NORMALMAP };

wxGraphicD3D12::wxGraphicD3D12(UINT width, UINT height, std::wstring name) :
	GraphicD3D12(width, height, name),
	m_pCBDataBegin(nullptr),			//constant buffer data pointer
	constBuff(),
	m_pwxMatDataBegin(nullptr),
	m_sunLightBuff(),
	m_pSunLightDataBegin(nullptr),
	m_objConst(),
	m_pObjConstDataBegin(nullptr),
	m_numIndices(0),
	angleAxisY(0.f),
	mSunAngleAxisY(0.f),
	mSunAngleAxisYPerSec(0.5f),
	m_frameIndex(0),
	m_fenceEvent(HANDLE()),
	m_viewport(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height)),
	m_scissorRect(0, 0, static_cast<LONG>(width), static_cast<LONG>(height)),
	m_shadowmap_Viewport(0.0f, 0.0f, 4096, 4096),
	m_shadowmap_ScissorRect(0, 0, 4096, 4096),
	m_ssao_viewport(0.f,0.f, static_cast<float>((width) / 2), static_cast<float>((height) / 2)),
	m_ssao_scissorRect(0,0, static_cast<float>((width) / 2), static_cast<float>((height) / 2)),
	m_rtvDescriptorSize(0),
	m_cameraMoveBaseSpeed(0.5f),
	m_cameraRotationSpeed(0.015f),
	m_defaultCameraPosition({ 0.f,300.f,300.f,1.f }),
	m_defaultLookAt({ 0.0f,0.f,0.f,1.0f}),
	m_defaultUp({ 0.f,1.f,0.f,0.f }),
	cameraDistance({ 0.f,0.f,0.f,1.f })
{}

void wxGraphicD3D12::OnInit()
{
	LoadPipeline();
	LoadAssets();
}

// Load the rendering pipeline dependencies.
void wxGraphicD3D12::LoadPipeline()
{
	UINT dxgiFactoryFlags = 0;

#if defined(_DEBUG)
	// Enable the debug layer (requires the Graphics Tools "optional feature").
	// NOTE: Enabling the debug layer after device creation will invalidate the active device.
	{
		ComPtr<ID3D12Debug> debugController;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
		{
			debugController->EnableDebugLayer();

			// Enable additional debug layers.
			dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
		}
	}
#endif

	ComPtr<IDXGIFactory4> factory;
	ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory)));

	if (m_useWarpDevice)
	{
		ComPtr<IDXGIAdapter> warpAdapter;
		ThrowIfFailed(factory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter)));

		ThrowIfFailed(D3D12CreateDevice(
			warpAdapter.Get(),
			D3D_FEATURE_LEVEL_11_0,
			IID_PPV_ARGS(&m_device)
			));
	}
	else
	{
		ComPtr<IDXGIAdapter1> hardwareAdapter;
		GetHardwareAdapter(factory.Get(), &hardwareAdapter);

		ThrowIfFailed(D3D12CreateDevice(
			hardwareAdapter.Get(),
			D3D_FEATURE_LEVEL_11_0,
			IID_PPV_ARGS(&m_device)
			));
	}

	// Describe and create the command queue.
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	ThrowIfFailed(m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue)));

	// Describe and create the swap chain.
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.BufferCount = FrameCount;
	swapChainDesc.Width = m_width;
	swapChainDesc.Height = m_height;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.SampleDesc.Count = 1;

	ComPtr<IDXGISwapChain1> swapChain;
	ThrowIfFailed(factory->CreateSwapChainForHwnd(
		m_commandQueue.Get(),		// Swap chain needs the queue so that it can force a flush on it.
		Win32Application::GetHwnd(),
		&swapChainDesc,
		nullptr,
		nullptr,
		&swapChain
		));

	// This sample does not support fullscreen transitions.
	ThrowIfFailed(factory->MakeWindowAssociation(Win32Application::GetHwnd(), DXGI_MWA_NO_ALT_ENTER));

	ThrowIfFailed(swapChain.As(&m_swapChain));
	m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

	RetrievalAssetDirectory();
	LoadDataFromOGEX(m_vec_AssetFileTitle);

	// Create descriptor heaps.
	{
		// Describe and create a depth stencil view (DSV) descriptor heap.
		D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
		dsvHeapDesc.NumDescriptors = 1 + SHADOW_DSV_MAP_COUNT + CAMERA_DSV_MAP_COUNT;
		dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		m_device->CreateDescriptorHeap(&dsvHeapDesc, __uuidof(m_dsvHeap), (void**)&m_dsvHeap);

		// Describe and create a render target view (RTV) descriptor heap.
		D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
		rtvHeapDesc.NumDescriptors = FrameCount + NormalMapCount + AmbientMapCount;
		rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		ThrowIfFailed(m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap)));

		// Describe and create a shader resource view (SRV) heap for the texture.
		D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
		srvHeapDesc.NumDescriptors = GetSceneGeometryNodeCount() * TYPE_END + SUNLIGHT_COUNT + CONSTANTMATRIX_COUNT + SHADOW_DSV_MAP_COUNT + NormalMapCount + AmbientMapCount + RANDOM_VECTOR_MAP_COUNT + SSAO_CONSTANT_COUNT + CAMERA_DSV_MAP_COUNT;	//order of the srv in srvHeap is:texture(more the one), material(geometry count)//SHADOW_MAP_DSV_COUNT used for srv
		srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;												//TransformMatrix(geometry count), light matrix, world related matrix.
		srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;																	
		ThrowIfFailed(m_device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&m_srvHeap)));

		m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	}

	// Create frame resources.
	{
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());

		// Create a RTV for each frame.
		for (UINT n = 0; n < FrameCount; n++)
		{
			ThrowIfFailed(m_swapChain->GetBuffer(n, IID_PPV_ARGS(&m_renderTargets[n])));
			m_device->CreateRenderTargetView(m_renderTargets[n].Get(), nullptr, rtvHandle);
			rtvHandle.Offset(1, m_rtvDescriptorSize);
		}
	}

	ThrowIfFailed(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocator)));
}

void wxGraphicD3D12::RetrievalAssetDirectory()
{
	FileLoader fileLoader;
	//get .ogex file title then get associated texture name from these .ogex title
	fileLoader.GetTitleBySuffix(m_vec_AssetFileTitle, ASSET_DIRECTORY, MATCH_OGEX, SUFFIX_OGEX);
	for (auto it : m_vec_AssetFileTitle)
	{
		fileLoader.GetNameByNameAndSuffix(m_vec_TextureTitle, ASSET_DIRECTORY, ResourceType, SUFFIX_BMP, it);
	}
}
// Load the sample assets.
void wxGraphicD3D12::LoadAssets()
{
	HRESULT hr;

	CreatePipelineStateObject();
	// Create the command list.
	ThrowIfFailed(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator.Get(), m_defaultPipelineState.Get(), IID_PPV_ARGS(&m_commandList)));

	m_TypedDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	m_DepthStencilDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

	CreateTexture(m_vec_TextureTitle);
	ParserDataFromScene(m_vec_AssetFileTitle);
	//resource Description for depth stencil buffer
	D3D12_HEAP_PROPERTIES dsHeapProperties;
	dsHeapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
	dsHeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	dsHeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	dsHeapProperties.CreationNodeMask = 1;
	dsHeapProperties.VisibleNodeMask = 1;

	D3D12_RESOURCE_DESC depthStencilDesc;
	depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depthStencilDesc.Alignment = 0;
	depthStencilDesc.Width = m_width;
	depthStencilDesc.Height = m_height;
	depthStencilDesc.DepthOrArraySize = 1;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_CLEAR_VALUE depthOptimizedClearValue = {};
	depthOptimizedClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthOptimizedClearValue.DepthStencil.Depth = 1.0f;
	depthOptimizedClearValue.DepthStencil.Stencil = 0;

	m_device->CreateCommittedResource(&dsHeapProperties, D3D12_HEAP_FLAG_NONE, &depthStencilDesc, \
		D3D12_RESOURCE_STATE_COMMON, &depthOptimizedClearValue, __uuidof(m_depthStencil), (void**)&m_depthStencil);

	// Create the depth stencil view.
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvBufferDesc = {};
	dsvBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsvBufferDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvBufferDesc.Flags = D3D12_DSV_FLAG_NONE;
	dsvBufferDesc.Texture2D.MipSlice = 0;
	// Create DepthStencilView
	m_device->CreateDepthStencilView(m_depthStencil.Get(), &dsvBufferDesc, m_dsvHeap->GetCPUDescriptorHandleForHeapStart());

	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_depthStencil.Get(),
		D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_DEPTH_WRITE));

	CreateSunLightBuffer();
	CreateConstantMatrix();
	GenerateShadowMap(); 
	GenerateCameraDepthMap();
	GenerateNormalMap();
	GenerateAmbientMap();
	GenerateRandomVectorTexture();
	BuildOffsetVectors();
	CreateSSAOBuffer();
	// Close the command list and execute it to begin the initial GPU setup.
	ThrowIfFailed(m_commandList->Close());
	ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	// Create synchronization objects and wait until assets have been uploaded to the GPU.
	{
		ThrowIfFailed(m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));
		m_fenceValue = 1;

		// Create an event handle to use for frame synchronization.
		m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		if (m_fenceEvent == nullptr)
		{
			ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
		}

		// Wait for the command list to execute; we are reusing the same command 
		// list in our main loop but for now, we just want to wait for setup to 
		// complete before continuing.
		WaitForPreviousFrame();
	}
}

void wxGraphicD3D12::CreatePipelineStateObject()
{
	HRESULT hr;
	//Create default pipeline.
		// Create the root signature.
	{
		D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};

		// This is the highest version the sample supports. If CheckFeatureSupport succeeds, the HighestVersion returned will not be greater than this.
		featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

		if (FAILED(m_device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
		{
			featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
		}

		CD3DX12_DESCRIPTOR_RANGE1 ranges[7];
		ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);	//texture resource view, specified shaderRegister number
		ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, SUNLIGHT_COUNT + CONSTANTMATRIX_COUNT, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);
		ranges[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 3, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);	//normal texture, specified shaderRegister number
		ranges[3].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 4, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);	//shadow map(all kinds of depth map), specified shaderRegister number
		ranges[4].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 5, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);	//random vector map, specified shaderRegister number
		ranges[5].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, SSAO_CONSTANT_COUNT, 2, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC); //ssao constant
		ranges[6].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, AmbientMapCount, 6, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC); //ssao map

		CD3DX12_ROOT_PARAMETER1 rootParameters[9];
		rootParameters[0].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_PIXEL);
		rootParameters[1].InitAsShaderResourceView(1, 0);//material resource view
		rootParameters[2].InitAsShaderResourceView(2, 0);//transform matrix resource view
		rootParameters[3].InitAsDescriptorTable(1, &ranges[1], D3D12_SHADER_VISIBILITY_ALL);
		rootParameters[4].InitAsDescriptorTable(1, &ranges[2], D3D12_SHADER_VISIBILITY_PIXEL);
		rootParameters[5].InitAsDescriptorTable(1, &ranges[3], D3D12_SHADER_VISIBILITY_ALL);
		rootParameters[6].InitAsDescriptorTable(1, &ranges[4], D3D12_SHADER_VISIBILITY_ALL);
		rootParameters[7].InitAsDescriptorTable(1, &ranges[5], D3D12_SHADER_VISIBILITY_ALL);
		rootParameters[8].InitAsDescriptorTable(1, &ranges[6], D3D12_SHADER_VISIBILITY_ALL);

		D3D12_STATIC_SAMPLER_DESC sampleDesc[7];
		D3D12_STATIC_SAMPLER_DESC sampler = {};
		sampler.Filter = D3D12_FILTER_ANISOTROPIC;
		sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		sampler.MipLODBias = 0;
		sampler.MaxAnisotropy = 0;
		sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_LESS;
		sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
		sampler.MinLOD = 0.0f;
		sampler.MaxLOD = D3D12_FLOAT32_MAX;
		sampler.ShaderRegister = 0;
		sampler.RegisterSpace = 0;
		sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
		sampleDesc[0] = sampler;
		
		D3D12_STATIC_SAMPLER_DESC anisotropicSampler = {};
		anisotropicSampler.Filter = D3D12_FILTER_ANISOTROPIC;
		anisotropicSampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		anisotropicSampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		anisotropicSampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		anisotropicSampler.MipLODBias = 0.f;
		anisotropicSampler.MaxAnisotropy = 8;
		anisotropicSampler.ComparisonFunc = D3D12_COMPARISON_FUNC_LESS;
		anisotropicSampler.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
		anisotropicSampler.MinLOD = 0.0f;
		anisotropicSampler.MaxLOD = D3D12_FLOAT32_MAX;
		anisotropicSampler.ShaderRegister = 1;
		anisotropicSampler.RegisterSpace = 0;
		anisotropicSampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
		sampleDesc[1] = anisotropicSampler;
		
		
		D3D12_STATIC_SAMPLER_DESC shadowSampler = {};
		shadowSampler.Filter = D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
		shadowSampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		shadowSampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		shadowSampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		shadowSampler.MipLODBias = 0.f;
		shadowSampler.MaxAnisotropy = 16;
		shadowSampler.ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
		shadowSampler.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
		shadowSampler.MinLOD = 0.0f;
		shadowSampler.MaxLOD = D3D12_FLOAT32_MAX;
		shadowSampler.ShaderRegister = 2;
		shadowSampler.RegisterSpace = 0;
		shadowSampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		sampleDesc[2] = shadowSampler;

		D3D12_STATIC_SAMPLER_DESC depthMapSampler = {};
		depthMapSampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
		depthMapSampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		depthMapSampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		depthMapSampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		depthMapSampler.MipLODBias = 0.f;
		depthMapSampler.MaxAnisotropy = 16;
		depthMapSampler.ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
		depthMapSampler.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
		depthMapSampler.MinLOD = 0.0f;
		depthMapSampler.MaxLOD = D3D12_FLOAT32_MAX;
		depthMapSampler.ShaderRegister = 3;
		depthMapSampler.RegisterSpace = 0;
		depthMapSampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		sampleDesc[3] = depthMapSampler;

		D3D12_STATIC_SAMPLER_DESC pointClampSampler = {};
		pointClampSampler.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
		pointClampSampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		pointClampSampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		pointClampSampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		pointClampSampler.MipLODBias = 0.f;
		pointClampSampler.MaxAnisotropy = 16;
		pointClampSampler.ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
		pointClampSampler.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
		pointClampSampler.MinLOD = 0.0f;
		pointClampSampler.MaxLOD = D3D12_FLOAT32_MAX;
		pointClampSampler.ShaderRegister = 4;
		pointClampSampler.RegisterSpace = 0;
		pointClampSampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		sampleDesc[4] = pointClampSampler;

		D3D12_STATIC_SAMPLER_DESC linearClampSampler = {};
		linearClampSampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
		linearClampSampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		linearClampSampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		linearClampSampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		linearClampSampler.MipLODBias = 0.f;
		linearClampSampler.MaxAnisotropy = 16;
		linearClampSampler.ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
		linearClampSampler.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
		linearClampSampler.MinLOD = 0.0f;
		linearClampSampler.MaxLOD = D3D12_FLOAT32_MAX;
		linearClampSampler.ShaderRegister = 5;
		linearClampSampler.RegisterSpace = 0;
		linearClampSampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		sampleDesc[5] = linearClampSampler;

		D3D12_STATIC_SAMPLER_DESC linearWrapSampler = {};
		linearWrapSampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
		linearWrapSampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		linearWrapSampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		linearWrapSampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		linearWrapSampler.MipLODBias = 0.f;
		linearWrapSampler.MaxAnisotropy = 16;
		linearWrapSampler.ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
		linearWrapSampler.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
		linearWrapSampler.MinLOD = 0.0f;
		linearWrapSampler.MaxLOD = D3D12_FLOAT32_MAX;
		linearWrapSampler.ShaderRegister = 6;
		linearWrapSampler.RegisterSpace = 0;
		linearWrapSampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		sampleDesc[6] = linearWrapSampler;

		CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
		rootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, 7, sampleDesc, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

		ComPtr<ID3DBlob> signature;
		ComPtr<ID3DBlob> error;
		ThrowIfFailed(D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, featureData.HighestVersion, &signature, &error));
		ThrowIfFailed(m_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature)));
	}

	// Create the pipeline state, which includes compiling and loading shaders.

		ComPtr<ID3DBlob> vertexShader;
		ComPtr<ID3DBlob> pixelShader;
		ComPtr<ID3DBlob> vertexShader1;
		ComPtr<ID3DBlob> pixelShader1;
		ComPtr<ID3DBlob> vertexShaderNormal;
		ComPtr<ID3DBlob> pixelShaderNormal;
		ComPtr<ID3DBlob> vertexSSAOShader;
		ComPtr<ID3DBlob> pixelSSAOShader;
		ComPtr<ID3DBlob> vertexBlurSSAOShader;
		ComPtr<ID3DBlob> pixelBlurSSAOShader;

#if defined(_DEBUG)
		// Enable better shader debugging with the graphics debugging tools.
		UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
		UINT compileFlags = 0;
#endif
		ThrowIfFailed(D3DCompileFromFile(GetAssetFullPath(L"..\\..\\..\\vs.hlsl").c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "VSMain", "vs_5_0", compileFlags, 0, &vertexShader, nullptr));
		ThrowIfFailed(D3DCompileFromFile(GetAssetFullPath(L"..\\..\\..\\ps.hlsl").c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "PSMain", "ps_5_0", compileFlags, 0, &pixelShader, nullptr));

		// Define the vertex input layout.
		D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 16, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 36, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		};
		D3D12_RASTERIZER_DESC RasterizerDefault;
		RasterizerDefault.FillMode = D3D12_FILL_MODE_SOLID;
		RasterizerDefault.CullMode = D3D12_CULL_MODE_BACK;	//this specified the not drawing face of the object
		RasterizerDefault.FrontCounterClockwise = FALSE;
		RasterizerDefault.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
		RasterizerDefault.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
		RasterizerDefault.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
		RasterizerDefault.DepthClipEnable = TRUE;
		RasterizerDefault.MultisampleEnable = FALSE;
		RasterizerDefault.AntialiasedLineEnable = FALSE;
		RasterizerDefault.ForcedSampleCount = 0;
		RasterizerDefault.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

		const D3D12_RENDER_TARGET_BLEND_DESC defaultRenderTargetBlendDesc =
		{
			FALSE,FALSE,
			D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
			D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
			D3D12_LOGIC_OP_NOOP,
			D3D12_COLOR_WRITE_ENABLE_ALL,
		};

		// depth stencil state
		D3D12_DEPTH_STENCIL_DESC defaultDepthStencil =
		{
			true, D3D12_DEPTH_WRITE_MASK_ALL, D3D12_COMPARISON_FUNC_LESS,
			false, D3D12_DEFAULT_STENCIL_READ_MASK, D3D12_DEFAULT_STENCIL_WRITE_MASK,
		{ D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP, D3D12_COMPARISON_FUNC_ALWAYS },
		{ D3D12_STENCIL_OP_KEEP ,D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP, D3D12_COMPARISON_FUNC_ALWAYS }

		};

		// Describe and create the graphics pipeline state object (PSO).
		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
		psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
		psoDesc.pRootSignature = m_rootSignature.Get();
		psoDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShader.Get());
		psoDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShader.Get());
		psoDesc.RasterizerState = RasterizerDefault;
		psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
		//psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
		psoDesc.SampleMask = UINT_MAX;
		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		psoDesc.NumRenderTargets = 1;
		psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		psoDesc.SampleDesc.Count = 1;
		psoDesc.SampleDesc.Quality = 0;
		psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
		ThrowIfFailed(m_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_defaultPipelineState)));

		ThrowIfFailed(D3DCompileFromFile(GetAssetFullPath(L"..\\..\\..\\shadow_map_v.hlsl").c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "VSMain", "vs_5_0", compileFlags, 0, &vertexShader1, nullptr));
		ThrowIfFailed(D3DCompileFromFile(GetAssetFullPath(L"..\\..\\..\\shadow_map_p.hlsl").c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "PSMain", "ps_5_0", compileFlags, 0, &pixelShader1, nullptr));

		D3D12_RASTERIZER_DESC smapRasterizer;
		smapRasterizer.FillMode = D3D12_FILL_MODE_SOLID;
		smapRasterizer.CullMode = D3D12_CULL_MODE_BACK;	//this specified the not drawing face of the object
		smapRasterizer.FrontCounterClockwise = FALSE;
		smapRasterizer.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
		smapRasterizer.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
		smapRasterizer.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
		smapRasterizer.DepthClipEnable = TRUE;
		smapRasterizer.MultisampleEnable = FALSE;
		smapRasterizer.AntialiasedLineEnable = FALSE;
		smapRasterizer.ForcedSampleCount = 0;
		smapRasterizer.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

		//shadow map pipeline state object
		D3D12_GRAPHICS_PIPELINE_STATE_DESC smapPsoDesc = psoDesc;
		smapPsoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
		smapPsoDesc.pRootSignature = m_rootSignature.Get();
		smapPsoDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShader1.Get());
		smapPsoDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShader1.Get());
		smapPsoDesc.SampleMask = UINT_MAX;
		smapPsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		smapPsoDesc.NumRenderTargets = 0;
		smapPsoDesc.RTVFormats[0] = DXGI_FORMAT_UNKNOWN;
		smapPsoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
		smapPsoDesc.RasterizerState.DepthBias = 100000;
		smapPsoDesc.RasterizerState.DepthBiasClamp = 0.0f;
		smapPsoDesc.RasterizerState.SlopeScaledDepthBias = 1.0f;
		ThrowIfFailed(m_device->CreateGraphicsPipelineState(&smapPsoDesc, IID_PPV_ARGS(&m_shadowMapPipelineState)));

		ThrowIfFailed(D3DCompileFromFile(GetAssetFullPath(L"..\\..\\..\\normals_vs.hlsl").c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "VSMain", "vs_5_0", compileFlags, 0, &vertexShaderNormal, nullptr));
		ThrowIfFailed(D3DCompileFromFile(GetAssetFullPath(L"..\\..\\..\\normals_ps.hlsl").c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "PSMain", "ps_5_0", compileFlags, 0, &pixelShaderNormal, nullptr));

		//drawing nromal pipeline state object
		D3D12_GRAPHICS_PIPELINE_STATE_DESC drawNormalPsoDesc = psoDesc;
		drawNormalPsoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
		drawNormalPsoDesc.pRootSignature = m_rootSignature.Get();
		drawNormalPsoDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShaderNormal.Get());
		drawNormalPsoDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShaderNormal.Get());
		drawNormalPsoDesc.SampleMask = UINT_MAX;
		drawNormalPsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		drawNormalPsoDesc.NumRenderTargets = 1;
		drawNormalPsoDesc.RTVFormats[0] = DXGI_FORMAT_R16G16B16A16_FLOAT;
		drawNormalPsoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
		drawNormalPsoDesc.SampleDesc.Count = 1;
		drawNormalPsoDesc.SampleDesc.Quality = 0;
		drawNormalPsoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;	//this specified the not drawing face of the object
		ThrowIfFailed(m_device->CreateGraphicsPipelineState(&drawNormalPsoDesc, IID_PPV_ARGS(&m_DrawNormalPipelineState)));

		ThrowIfFailed(D3DCompileFromFile(GetAssetFullPath(L"..\\..\\..\\ssao_vs.hlsl").c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "VSMain", "vs_5_0", compileFlags, 0, &vertexSSAOShader, nullptr));
		ThrowIfFailed(D3DCompileFromFile(GetAssetFullPath(L"..\\..\\..\\ssao_ps.hlsl").c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "PSMain", "ps_5_0", compileFlags, 0, &pixelSSAOShader, nullptr));
		//ssao pipeline state object
		D3D12_GRAPHICS_PIPELINE_STATE_DESC ssaoPsoDesc = psoDesc;
		ssaoPsoDesc.InputLayout = { nullptr, 0 };
		ssaoPsoDesc.pRootSignature = m_rootSignature.Get();
		ssaoPsoDesc.VS = CD3DX12_SHADER_BYTECODE(vertexSSAOShader.Get());
		ssaoPsoDesc.PS = CD3DX12_SHADER_BYTECODE(pixelSSAOShader.Get());
		ssaoPsoDesc.SampleMask = UINT_MAX;
		ssaoPsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		ssaoPsoDesc.NumRenderTargets = 1;
		ssaoPsoDesc.RTVFormats[0] = DXGI_FORMAT_R16_UNORM;
		ssaoPsoDesc.DepthStencilState.DepthEnable = false;
		ssaoPsoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
		ssaoPsoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
		ssaoPsoDesc.RasterizerState = RasterizerDefault;
		ssaoPsoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;//ssao doesn't need CullMode
		//ssaoPsoDesc.RasterizerState.FrontCounterClockwise = false;
		ssaoPsoDesc.SampleDesc.Count = 1;
		ssaoPsoDesc.SampleDesc.Quality = 0;
		ssaoPsoDesc.DSVFormat = DXGI_FORMAT_UNKNOWN;
		ThrowIfFailed(m_device->CreateGraphicsPipelineState(&ssaoPsoDesc, IID_PPV_ARGS(&m_SsaoPipelineState)));

		ThrowIfFailed(D3DCompileFromFile(GetAssetFullPath(L"..\\..\\..\\blurssao_vs.hlsl").c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "VSMain", "vs_5_0", compileFlags, 0, &vertexBlurSSAOShader, nullptr));
		ThrowIfFailed(D3DCompileFromFile(GetAssetFullPath(L"..\\..\\..\\blurssao_ps.hlsl").c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "PSMain", "ps_5_0", compileFlags, 0, &pixelBlurSSAOShader, nullptr));
		ssaoPsoDesc.VS = CD3DX12_SHADER_BYTECODE(vertexBlurSSAOShader.Get());
		ssaoPsoDesc.PS = CD3DX12_SHADER_BYTECODE(pixelBlurSSAOShader.Get());
		ThrowIfFailed(m_device->CreateGraphicsPipelineState(&ssaoPsoDesc, IID_PPV_ARGS(&m_BlurSsaoPipelineState)));
}
void wxGraphicD3D12::PopulateShadowMapCommandList()
{
	m_commandList->RSSetViewports(1, &m_shadowmap_Viewport);
	m_commandList->RSSetScissorRects(1, &m_shadowmap_ScissorRect);
	// Change to DEPTH_WRITE.
	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_shadowDepthMap.Get(),
		D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_DEPTH_WRITE));
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = m_dsvHeap->GetCPUDescriptorHandleForHeapStart();
	dsvHandle.ptr += m_DepthStencilDescriptorSize * DEFAULT_DSV_MAP_COUNT;
	m_commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	m_commandList->OMSetStencilRef(0);
	m_commandList->OMSetRenderTargets(0, nullptr, false, &dsvHandle);
	m_commandList->SetPipelineState(m_shadowMapPipelineState.Get());
	m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	for (int i = 0; i < GetSceneGeometryNodeCount(); i++)
	{
		D3D12_GPU_DESCRIPTOR_HANDLE srvOffset = m_srvHeap->GetGPUDescriptorHandleForHeapStart();
		srvOffset.ptr += i * m_TypedDescriptorSize;
		m_commandList->SetGraphicsRootDescriptorTable(0, srvOffset);	//texture
		m_commandList->IASetVertexBuffers(0, 1, &(m_vec_VertexBufferView[i]));
		m_commandList->IASetIndexBuffer(&m_vec_IndexBufferView[i]);
		m_commandList->SetGraphicsRootShaderResourceView(1, m_vec_matRes[i]->GetGPUVirtualAddress());	//material
		m_commandList->SetGraphicsRootShaderResourceView(2, m_vec_objConstRes[i]->GetGPUVirtualAddress());	//transform matrix
		srvOffset = m_srvHeap->GetGPUDescriptorHandleForHeapStart();
		srvOffset.ptr += (GetSceneGeometryNodeCount() * TYPE_NORMAL_MAP + i) * m_TypedDescriptorSize;
		m_commandList->SetGraphicsRootDescriptorTable(4, srvOffset);	//normalmap
		srvOffset = m_srvHeap->GetGPUDescriptorHandleForHeapStart();
		srvOffset.ptr += (GetSceneGeometryNodeCount() * TYPE_END + SUNLIGHT_COUNT + CONSTANTMATRIX_COUNT) * m_TypedDescriptorSize;
		m_commandList->SetGraphicsRootDescriptorTable(5, srvOffset);	//shadowmap
		m_commandList->DrawIndexedInstanced(m_vec_numIndices[i], 1, 0, 0, 0);
	}
	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_shadowDepthMap.Get(),
		D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_GENERIC_READ));
}

void wxGame::wxGraphicD3D12::PopulateNormalCommandList()
{
	m_commandList->RSSetViewports(1, &m_viewport);
	m_commandList->RSSetScissorRects(1, &m_scissorRect);
	// Change to RENDER_TARGET.
	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_NormalMap.Get(),
		D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET));

	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_rtvHeap->GetCPUDescriptorHandleForHeapStart();
	rtvHandle.ptr += m_rtvDescriptorSize * FrameCount;


	const float clearColor[] = { 0.f, 0.f, 1.f, 0.f };
	m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = m_dsvHeap->GetCPUDescriptorHandleForHeapStart();
	dsvHandle.ptr += m_DepthStencilDescriptorSize * (DEFAULT_DSV_MAP_COUNT + SHADOW_DSV_MAP_COUNT);
	m_commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
	m_commandList->OMSetRenderTargets(1, &rtvHandle, true, &dsvHandle);

	m_commandList->OMSetStencilRef(0);
	m_commandList->SetPipelineState(m_DrawNormalPipelineState.Get());
	m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	D3D12_GPU_DESCRIPTOR_HANDLE srvOffset = m_srvHeap->GetGPUDescriptorHandleForHeapStart();
	//srvOffset.ptr += (GetSceneGeometryNodeCount() * TYPE_END + SUNLIGHT_COUNT + CONSTANTMATRIX_COUNT + SHADOW_DSV_MAP_COUNT + NormalMapCount + AmbientMapCount + RANDOM_VECTOR_MAP_COUNT + SSAO_COUNT) * m_TypedDescriptorSize;
	//m_commandList->SetGraphicsRootDescriptorTable(5, srvOffset);	//depth map
	for (int i = 0; i < GetSceneGeometryNodeCount(); i++)
	{
		D3D12_GPU_DESCRIPTOR_HANDLE srvOffset = m_srvHeap->GetGPUDescriptorHandleForHeapStart();
		srvOffset.ptr += i * m_TypedDescriptorSize;
		m_commandList->SetGraphicsRootDescriptorTable(0, srvOffset);	//texture
		m_commandList->IASetVertexBuffers(0, 1, &(m_vec_VertexBufferView[i]));
		m_commandList->IASetIndexBuffer(&m_vec_IndexBufferView[i]);
		m_commandList->SetGraphicsRootShaderResourceView(1, m_vec_matRes[i]->GetGPUVirtualAddress());	//material
		m_commandList->SetGraphicsRootShaderResourceView(2, m_vec_objConstRes[i]->GetGPUVirtualAddress());	//transform matrix
		srvOffset = m_srvHeap->GetGPUDescriptorHandleForHeapStart();
		srvOffset.ptr += (GetSceneGeometryNodeCount() * TYPE_NORMAL_MAP + i) * m_TypedDescriptorSize;
		m_commandList->SetGraphicsRootDescriptorTable(4, srvOffset);	//normalmap
		srvOffset = m_srvHeap->GetGPUDescriptorHandleForHeapStart();
		m_commandList->DrawIndexedInstanced(m_vec_numIndices[i], 1, 0, 0, 0);
	}
	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_NormalMap.Get(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ));
}

void wxGraphicD3D12::PopulateSSAOCommandList()
{
	m_commandList->RSSetViewports(1, &m_ssao_viewport);
	m_commandList->RSSetScissorRects(1, &m_ssao_scissorRect);

	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_AmbientMap1.Get(),
		D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET));

	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_rtvHeap->GetCPUDescriptorHandleForHeapStart();
	rtvHandle.ptr += m_rtvDescriptorSize * (FrameCount + NormalMapCount);

	float clearColor[] = { 1.0f,1.0f,1.0f,1.0f };
	m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
	m_commandList->OMSetRenderTargets(1, &rtvHandle, true, nullptr);

	m_commandList->OMSetStencilRef(0);
	m_commandList->SetPipelineState(m_SsaoPipelineState.Get());
	m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	D3D12_GPU_DESCRIPTOR_HANDLE srvConstantBuff = m_srvHeap->GetGPUDescriptorHandleForHeapStart();		//begin from material cbv append Matrix4X4FT constantBuff and m_sunLightBuff
	srvConstantBuff.ptr += (GetSceneGeometryNodeCount() * TYPE_END + SUNLIGHT_COUNT + CONSTANTMATRIX_COUNT + SHADOW_DSV_MAP_COUNT + NormalMapCount + AmbientMapCount + RANDOM_VECTOR_MAP_COUNT) * m_TypedDescriptorSize;
	m_commandList->SetGraphicsRootDescriptorTable(7, srvConstantBuff);

	D3D12_GPU_DESCRIPTOR_HANDLE srvOffset = m_srvHeap->GetGPUDescriptorHandleForHeapStart();
	m_commandList->IASetVertexBuffers(0, 0, nullptr);
	m_commandList->IASetIndexBuffer(nullptr);
	srvOffset = m_srvHeap->GetGPUDescriptorHandleForHeapStart();
	srvOffset.ptr += (GetSceneGeometryNodeCount() * TYPE_END + +SUNLIGHT_COUNT + CONSTANTMATRIX_COUNT + SHADOW_DSV_MAP_COUNT) * m_TypedDescriptorSize;
	m_commandList->SetGraphicsRootDescriptorTable(4, srvOffset);	//ssao normalmap
	srvOffset = m_srvHeap->GetGPUDescriptorHandleForHeapStart();
	srvOffset.ptr += (GetSceneGeometryNodeCount() * TYPE_END + SUNLIGHT_COUNT + CONSTANTMATRIX_COUNT + SHADOW_DSV_MAP_COUNT + NormalMapCount + AmbientMapCount + RANDOM_VECTOR_MAP_COUNT + SSAO_CONSTANT_COUNT) * m_TypedDescriptorSize;
	m_commandList->SetGraphicsRootDescriptorTable(5, srvOffset);	//depth map
	srvOffset = m_srvHeap->GetGPUDescriptorHandleForHeapStart();
	srvOffset.ptr += (GetSceneGeometryNodeCount() * TYPE_END + SUNLIGHT_COUNT + CONSTANTMATRIX_COUNT + SHADOW_DSV_MAP_COUNT + NormalMapCount + AmbientMapCount) * m_TypedDescriptorSize;
	m_commandList->SetGraphicsRootDescriptorTable(6, srvOffset);	//random vector map
	m_commandList->DrawInstanced(6, 1, 0, 0);
	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_AmbientMap1.Get(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ));
}

void wxGame::wxGraphicD3D12::PopulateBlurSSAOCommandList()
{
	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_AmbientMap2.Get(),
		D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET));

	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_rtvHeap->GetCPUDescriptorHandleForHeapStart();
	rtvHandle.ptr += m_rtvDescriptorSize * (FrameCount + NormalMapCount + 1);	//blurAmbiemtMap

	float clearColor[] = { 1.0f,1.0f,1.0f,1.0f };
	m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
	m_commandList->OMSetRenderTargets(1, &rtvHandle, true, nullptr);

	m_commandList->OMSetStencilRef(0);
	m_commandList->SetPipelineState(m_BlurSsaoPipelineState.Get());
	m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	D3D12_GPU_DESCRIPTOR_HANDLE srvOffset = m_srvHeap->GetGPUDescriptorHandleForHeapStart();
	srvOffset.ptr += m_TypedDescriptorSize * (TYPE_END * GetSceneGeometryNodeCount() + SUNLIGHT_COUNT + CONSTANTMATRIX_COUNT + SHADOW_DSV_MAP_COUNT + NormalMapCount);
	m_commandList->SetGraphicsRootDescriptorTable(8, srvOffset);	//AmbientMap

	m_commandList->IASetVertexBuffers(0, 0, nullptr);
	m_commandList->IASetIndexBuffer(nullptr);

	m_commandList->DrawInstanced(6, 1, 0, 0);
	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_AmbientMap2.Get(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ));
}

void wxGraphicD3D12::PopulateCommandList()
{
	ThrowIfFailed(m_commandAllocator->Reset());

	ThrowIfFailed(m_commandList->Reset(m_commandAllocator.Get(), m_defaultPipelineState.Get()));

	m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());

	ID3D12DescriptorHeap* ppHeaps[] = { m_srvHeap.Get() };
	m_commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

	D3D12_GPU_DESCRIPTOR_HANDLE srvConstantBuff = m_srvHeap->GetGPUDescriptorHandleForHeapStart();		//begin from material cbv append Matrix4X4FT constantBuff and m_sunLightBuff
	srvConstantBuff.ptr += m_TypedDescriptorSize * (GetSceneGeometryNodeCount() * TYPE_END);
	m_commandList->SetGraphicsRootDescriptorTable(3, srvConstantBuff);

	PopulateNormalCommandList();
	PopulateSSAOCommandList(); 
	m_constSSAOBuff.horzBlur = 1.f; 
	UpdateSSAO(GetTimer());
	PopulateBlurSSAOCommandList();
	m_constSSAOBuff.horzBlur = 0.f;
	UpdateSSAO(GetTimer());
	PopulateBlurSSAOCommandList();
	PopulateShadowMapCommandList();
	m_commandList->RSSetViewports(1, &m_viewport);
	m_commandList->RSSetScissorRects(1, &m_scissorRect);

	// Indicate that the back buffer will be used as a render target.
	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_frameIndex, m_rtvDescriptorSize);
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = m_dsvHeap->GetCPUDescriptorHandleForHeapStart();
	m_commandList->ClearDepthStencilView(m_dsvHeap->GetCPUDescriptorHandleForHeapStart(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
	m_commandList->OMSetRenderTargets(1, &rtvHandle, true, &dsvHandle);

	const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
	m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

	m_commandList->OMSetStencilRef(0);
	m_commandList->SetPipelineState(m_defaultPipelineState.Get());
	// Record commands.
	m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//draw objects
	for (int i = 0; i < GetSceneGeometryNodeCount(); i++)
	{
		D3D12_GPU_DESCRIPTOR_HANDLE srvOffset = m_srvHeap->GetGPUDescriptorHandleForHeapStart();
		srvOffset.ptr += i * m_TypedDescriptorSize;
		m_commandList->SetGraphicsRootDescriptorTable(0, srvOffset);	//texture
		m_commandList->IASetVertexBuffers(0, 1, &(m_vec_VertexBufferView[i]));
		m_commandList->IASetIndexBuffer(&m_vec_IndexBufferView[i]);
		m_commandList->SetGraphicsRootShaderResourceView(1, m_vec_matRes[i]->GetGPUVirtualAddress());	//material
		m_commandList->SetGraphicsRootShaderResourceView(2, m_vec_objConstRes[i]->GetGPUVirtualAddress());	//transform matrix
		srvOffset = m_srvHeap->GetGPUDescriptorHandleForHeapStart();
		srvOffset.ptr += (GetSceneGeometryNodeCount() * TYPE_NORMAL_MAP + i) * m_TypedDescriptorSize;
		m_commandList->SetGraphicsRootDescriptorTable(4, srvOffset);	//normalmap
		srvOffset = m_srvHeap->GetGPUDescriptorHandleForHeapStart();
		srvOffset.ptr += m_TypedDescriptorSize * (TYPE_END * GetSceneGeometryNodeCount() + SUNLIGHT_COUNT + CONSTANTMATRIX_COUNT + SHADOW_DSV_MAP_COUNT + NormalMapCount + 1);
		m_commandList->SetGraphicsRootDescriptorTable(8, srvOffset);	//ambient map
		
		m_commandList->DrawIndexedInstanced(m_vec_numIndices[i], 1, 0, 0, 0);
	}

	// Record commands.
	m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Indicate that the back buffer will now be used to present.
	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

	ThrowIfFailed(m_commandList->Close());
}

// Update frame-based values.
void wxGraphicD3D12::OnUpdate(wxTimer *timer)
{
	UpdateSunLight(timer);
	UpdateConstantBuffer();
	UpdateSSAO(timer);
	CheckControllerInput();
}

// Render the scene.
void wxGraphicD3D12::OnRender()
{
	// Record all the commands we need to render the scene into the command list.
	PopulateCommandList();

	// Execute the command list.
	ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	// Present the frame.
	ThrowIfFailed(m_swapChain->Present(1, 0));
	WaitForPreviousFrame();
}

void wxGraphicD3D12::OnDestroy()
 {
	// Ensure that the GPU is no longer referencing resources that are about to be
	// cleaned up by the destructor.
	WaitForPreviousFrame();

	CloseHandle(m_fenceEvent);
}

void wxGraphicD3D12::WaitForPreviousFrame()
{
	// Signal and increment the fence value.
	const UINT64 fence = m_fenceValue;
	ThrowIfFailed(m_commandQueue->Signal(m_fence.Get(), fence));
	m_fenceValue++;

	// Wait until the previous frame is finished.
	if (m_fence->GetCompletedValue() < fence)
	{
		ThrowIfFailed(m_fence->SetEventOnCompletion(fence, m_fenceEvent));
		WaitForSingleObject(m_fenceEvent, INFINITE);
	}

	m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
}

void wxGraphicD3D12::UpdateConstantBuffer(void)
{
	constBuff.rotatMatrix = MatrixRotationY(angleAxisY);
	constBuff.cameraPos = m_defaultCameraPosition;
	constBuff.viewPos = m_defaultLookAt;
	UpdateShadowMatrix();
	memcpy(m_pCBDataBegin, &constBuff, sizeof(wxConstMatrix));
}

void wxGame::wxGraphicD3D12::UpdateShadowMatrix(void)
{	
	//fix the light direction temporary
	Matrix4X4FT LightViewMatrix = BuildViewMatrix(Vector4FT({ m_sunLightBuff.Position.element[0], m_sunLightBuff.Position.element[1], m_sunLightBuff.Position.element[2], 1.f }),\
								  Vector4FT({ 0.f, 0.f, 0.f, 1.f}), \
								  Vector4FT({ 0.f, 1.f, 0.f, 0.f }));//light up direction.

	Matrix4X4FT LightOthgraphicMatrix = BuildOthographicMatrixForLH(-100,100,100,-100,-1000,1000);
	Matrix4X4FT LightTransformNDC = {
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.0f, 1.0f };

	constBuff.shadowTransform = MatrixMultiMatrix(MatrixMultiMatrix(LightViewMatrix, LightOthgraphicMatrix), LightTransformNDC);
	constBuff.lightOthgraphicMatrix = LightOthgraphicMatrix;
	constBuff.lightViewMatrix = LightViewMatrix;
	constBuff.lightTransformNDC = LightTransformNDC;
	constBuff.invProjMatrix = MatrixInverse(constBuff.perspectiveMatrix);
	Matrix4X4FT T = MatrixMultiMatrix(constBuff.perspectiveMatrix, constBuff.invProjMatrix);
}

void wxGame::wxGraphicD3D12::UpdateSunLight(wxTimer * timer)
{
	mSunAngleAxisY += mSunAngleAxisYPerSec * timer->DeltaTime();
	int k = mSunAngleAxisY / 360;
	mSunAngleAxisY = mSunAngleAxisY - k * 360;
	m_sunLightBuff.Direction = Vector3FT({ 0.f, -1.f, -1.f });//don't accumulate the direction value,but set default value every time;
	Vector4FT Direction = { m_sunLightBuff.Direction.element[0], m_sunLightBuff.Direction.element[1], m_sunLightBuff.Direction.element[2], 0.f };
	RotateYAxis(Direction, mSunAngleAxisY);
	m_sunLightBuff.Direction.element[0] = Direction.element[0];
	m_sunLightBuff.Direction.element[1] = Direction.element[1];
	m_sunLightBuff.Direction.element[2] = Direction.element[2];

	m_sunLightBuff.Position = Vector3FT({ 0.f, 300.f, 300.f });
	Vector4FT Position = { m_sunLightBuff.Position.element[0], m_sunLightBuff.Position.element[1], m_sunLightBuff.Position.element[2], 1.f };
	RotateYAxis(Position, mSunAngleAxisY);
	m_sunLightBuff.Position.element[0] = Position.element[0];
	m_sunLightBuff.Position.element[1] = Position.element[1];
	m_sunLightBuff.Position.element[2] = Position.element[2];

	memcpy(m_pSunLightDataBegin, &m_sunLightBuff, sizeof(wxLight));
}

void wxGame::wxGraphicD3D12::UpdateSSAO(wxTimer * timer)
{
	m_constSSAOBuff.offsetVectors[0] = m_offsets[0];
	m_constSSAOBuff.offsetVectors[1] = m_offsets[1];
	m_constSSAOBuff.offsetVectors[2] = m_offsets[2];
	m_constSSAOBuff.offsetVectors[3] = m_offsets[3];
	m_constSSAOBuff.offsetVectors[4] = m_offsets[4];
	m_constSSAOBuff.offsetVectors[5] = m_offsets[5];
	m_constSSAOBuff.offsetVectors[6] = m_offsets[6];
	m_constSSAOBuff.offsetVectors[7] = m_offsets[7];
	m_constSSAOBuff.offsetVectors[8] = m_offsets[8];
	m_constSSAOBuff.offsetVectors[9] = m_offsets[9];
	m_constSSAOBuff.offsetVectors[10] = m_offsets[10];
	m_constSSAOBuff.offsetVectors[11] = m_offsets[11];
	m_constSSAOBuff.offsetVectors[12] = m_offsets[12];
	m_constSSAOBuff.offsetVectors[13] = m_offsets[13];

	m_constSSAOBuff.occlusionRadius = 2.f;
	m_constSSAOBuff.occlusionFadeStart = 0.8f;
	m_constSSAOBuff.occlusionFadeEnd = 4.f;
	m_constSSAOBuff.surfaceEpsilon = 0.2f;
	m_constSSAOBuff.ScreenSize.element[0] = GetWidth();
	m_constSSAOBuff.ScreenSize.element[1] = GetHeight();

	UpdateBlurWidget();

	memcpy(m_pCBSSAOBegin, &m_constSSAOBuff, sizeof(wxSSAOConstant));
}

void wxGame::wxGraphicD3D12::UpdateBlurWidget()
{
	std::vector<Vector4FT> BlurWidget = CalcGaussWeights(2.5f);
	for (int i = 0; i != 3; i++)
	{
		m_constSSAOBuff.blurWeights[0] = BlurWidget[0];
		m_constSSAOBuff.blurWeights[1] = BlurWidget[1];
		m_constSSAOBuff.blurWeights[2] = BlurWidget[2];
	}
}

std::vector<Vector4FT> wxGame::wxGraphicD3D12::CalcGaussWeights(float sigma)
{
	float twoSigmaSquare = 2.0f * pow(sigma, 2);
	int blurRadius = (int)ceil(2.0f * sigma);

	std::vector<float> weights;
	std::vector<Vector4FT> returnWeights;
	Vector4FT vec4ftWeights;
	weights.resize(2 * blurRadius + 1);
	float weightSum = 0.0f;
	for (INT i = -blurRadius; i <= blurRadius; i++)
	{
		float x = (float)i;
		float base = E;
		weights[i + blurRadius] = pow(base, -x * x / twoSigmaSquare);
		weightSum += weights[i + blurRadius];
	}
	for (int i = 0; i < weights.size();)
	{
		weights[i] /= weightSum;
		vec4ftWeights.element[i % 4] = weights[i];
		if (i % 4 == 3 || i == weights.size() - 1)
		{
			returnWeights.push_back(vec4ftWeights);
			Vector4FT emptyVector4FT(0);
			vec4ftWeights = emptyVector4FT;
		}
		i++;
	}
	return returnWeights;
}

void wxGraphicD3D12::LoadDataFromOGEX(std::vector<std::string>& title)
{
	for (int i = 0; i != title.size(); i++)
	{
		std::string path = (ASSET_DIRECTORY + title[i] + SUFFIX_OGEX).c_str();
		SceneManager* sceneManager = new SceneManager;
		sceneManager->LoadScene(path.c_str());
		g_vecpSceneManager.push_back(sceneManager);
	}
}

int wxGraphicD3D12::GetSceneGeometryNodeCount()
{
	int count = 0;
	for (auto& _it : g_vecpSceneManager)
	{
		Scene scene = _it->GetSceneForRendering();
		count += scene.GeometryNodes.size();
	}
	return count;
}

void wxGraphicD3D12::ParserDataFromScene(std::vector<std::string>& title)
{
	for (auto& _it : g_vecpSceneManager)
	{
		Scene scene = _it->GetSceneForRendering();
		if (!scene.Geometries.empty())
			for (auto& _it : scene.GeometryNodes)
			{
				SceneGeometryNode* geoNode = _it.second;
				std::string title = _it.first;
				if (geoNode)
				{
					std::string GeometryObjName = geoNode->GetGeometryObjectRef();
					if (GeometryObjName != "")
					{
						const auto& _itr = scene.Geometries.find(GeometryObjName);
						if (_itr != scene.Geometries.end())	//If we found a GeometryObject in GeometryNode, The parser's proccessing start.
						{
							const auto& pGeometryObject = _itr->second;
							SceneObjectMesh* pMesh = pGeometryObject->GetMesh();
							size_t elementCount = pMesh->GetVertexCount();
							Vertex* vertexMix = new Vertex[elementCount];
							for (uint32_t i = 0; i < pMesh->GetVertexPropertiesCount(); i++)
							{
								const SceneObjectVertexArray& v_property_array = pMesh->GetVertexPropertyArray(i);
								size_t length = v_property_array.GetElementCount();
								if (v_property_array.GetAttribute() == "position")
								{
									float* pPosition = (float*)v_property_array.GetData();
									for (uint32_t i = 0; i < length; i = i + 3)
									{
										size_t label = i / 3;
										vertexMix[label].position.element[0] = pPosition[i];
										vertexMix[label].position.element[1] = pPosition[i + 1];
										vertexMix[label].position.element[2] = pPosition[i + 2];
										vertexMix[label].position.element[3] = 1.f;
									}
								}
								if (v_property_array.GetAttribute() == "texcoord")
								{
									float* pTexcoord = (float*)v_property_array.GetData();
									for (uint32_t i = 0; i < length; i = i + 2)
									{
										size_t label = i / 2;
										vertexMix[label].uv.element[0] = pTexcoord[i];
										vertexMix[label].uv.element[1] = pTexcoord[i + 1];
									}
								}
								if (v_property_array.GetAttribute() == "normal")
								{
									float* pNormal = (float*)v_property_array.GetData();
									for (uint32_t i = 0; i < length; i = i + 3)
									{
										size_t label = i / 3;
										vertexMix[label].Normal.element[0] = pNormal[i];
										vertexMix[label].Normal.element[1] = pNormal[i + 1];
										vertexMix[label].Normal.element[2] = pNormal[i + 2];
									}
								}
								if (v_property_array.GetAttribute() == "tangent")
								{
									float* pTangent = (float*)v_property_array.GetData();
									for (uint32_t i = 0; i < length; i = i + 3)
									{
										size_t label = i / 3;
										vertexMix[label].tangent.element[0] = pTangent[i];
										vertexMix[label].tangent.element[1] = pTangent[i + 1];
										vertexMix[label].tangent.element[2] = pTangent[i + 2];
									}
								}
							}

							SceneObjectTransform* transform = geoNode->GetTransform(0);
							wxObjConst objConst;
							if (transform)
							{
								objConst.linearTransMatrix = *(transform->GetTransformMatrix());
								m_vec_objConstRes.push_back(ComPtr<ID3D12Resource>());
								m_vec_objConstStut.push_back(objConst);
							}

							CreateVertexBuffer(*vertexMix, elementCount);
							delete[] vertexMix;
							const auto indexCount = pMesh->GetIndexCount();
							int* Indice = new int[indexCount];
							for (uint32_t i = 0; i < pMesh->GetIndexGroupCount(); i++)
							{
								const SceneObjectIndexArray& v_index_array = pMesh->GetIndexArray(i);
								int* pPosition = (int*)v_index_array.GetData();
								for (size_t i = 0; i < indexCount; i++)
								{
									Indice[i] = pPosition[i];
								}
							}
							CreateIndexBuffer(*Indice, indexCount);
							m_vec_numIndices.push_back(indexCount);
							delete[] Indice;

							wxMaterial mat;
							const auto& _itr1 = scene.Materials.find(geoNode->GetGeometryMaterialName(0));
							if (_itr1 != scene.Materials.end())
							{
								SceneObjectMaterial* matNode = _itr1->second;
								mat.Roughness = matNode->GetRoughness();
								mat.Roughness = 0.7;
								Color tempColor = matNode->GetDiffuseColor();
								mat.diffuse = Vector4FT({ tempColor.Value.element[0], tempColor.Value.element[1], tempColor.Value.element[2] , 1.0f});
								//mat.m_AmbientOcclusion = matNode->GetAmbientOcclusion();
								mat.FresnelR0 = Vector3FT({ matNode->GetSpecular().Value.element[0], matNode->GetSpecular().Value.element[1], matNode->GetSpecular().Value.element[2] });
								m_vec_matRes.push_back(ComPtr<ID3D12Resource>());
								m_vec_matStut.push_back(mat);
								m_MaterialCount++;
							}
						}
					}
				}
			}
	}

	CreateObjConst(m_vec_objConstStut);
	CreateConstantMaterialBuffer(m_vec_matStut);
}

void wxGraphicD3D12::LoadDefaultVertexIndexData()
{
	HRESULT hr;
	// Create the vertex buffer.
	{
		// Define the geometry for a triangle.
		Vertex triangleVertices[] =
		{
			{ { 0.25f, 0.25f, -0.25f, 1.0f }, { 1.0f, 0.0f } },//0
			{ { 0.25f, -0.25f, -0.25f, 1.0f }, { 1.0f, 1.0f } },//1
			{ { -0.25f, 0.25f, -0.25f, 1.0f },{  0.0f, 0.0f } },//2
			{ { -0.25f, -0.25f, -0.25f, 1.0f }, { 0.0f, 1.0f } },//3
			{ { 0.25f, 0.25f, 0.25f, 1.0f },{ 0.0f, 0.0f } },//4
			{ { 0.25f, -0.25f, 0.25f, 1.0f },{ 0.0f, 1.0f } },//5
			{ { -0.25f, 0.25f, 0.25f, 1.0f },{ 1.0f, 0.0f } },//6
			{ { -0.25f, -0.25f, 0.25f, 1.0f },{ 1.0f, 1.0f } },//7
		};

		const UINT vertexBufferSize = sizeof(triangleVertices);

		// Note: using upload heaps to transfer static data like vert buffers is not 
		// recommended. Every time the GPU needs it, the upload heap will be marshalled 
		// over. Please read up on Default Heap usage. An upload heap is used here for 
		// code simplicity and because there are very few verts to actually transfer.
		ThrowIfFailed(m_device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&m_vertexBuffer)));

		// Copy the triangle data to the vertex buffer.
		UINT8* pVertexDataBegin;
		CD3DX12_RANGE readRange(0, 0);		// We do not intend to read from this resource on the CPU.
		ThrowIfFailed(m_vertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin)));
		memcpy(pVertexDataBegin, triangleVertices, sizeof(triangleVertices));
		m_vertexBuffer->Unmap(0, nullptr);

		// Initialize the vertex buffer view.
		m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
		m_vertexBufferView.StrideInBytes = sizeof(Vertex);
		m_vertexBufferView.SizeInBytes = vertexBufferSize;
	}

	{
		int triangleIndices[] = {
			0,1,3,
			0,3,2,
			//4,5,1,
			//4,1,0,
			//6,7,5,
			//6,5,4,
			//2,3,7,
			//2,7,6,
		};
		const unsigned int indicesBufferSize = sizeof(triangleIndices);
		m_numIndices = indicesBufferSize / sizeof(int);

		D3D12_HEAP_PROPERTIES indexHeapProperties;
		indexHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
		indexHeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		indexHeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		indexHeapProperties.CreationNodeMask = 1;
		indexHeapProperties.VisibleNodeMask = 1;

		//resource Description for index buffer
		D3D12_RESOURCE_DESC indexBufferDesc;
		indexBufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		indexBufferDesc.Alignment = 0;
		indexBufferDesc.Width = indicesBufferSize;
		indexBufferDesc.Height = 1;
		indexBufferDesc.DepthOrArraySize = 1;
		indexBufferDesc.MipLevels = 1;
		indexBufferDesc.Format = DXGI_FORMAT_UNKNOWN;
		indexBufferDesc.SampleDesc.Count = 1;
		indexBufferDesc.SampleDesc.Quality = 0;
		indexBufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		indexBufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

		indexHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
		hr = m_device->CreateCommittedResource(&indexHeapProperties, D3D12_HEAP_FLAG_NONE, \
			&indexBufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, __uuidof(m_indexBuffer), (void**)&m_indexBuffer);

		void* pIndexDataBegin;
		D3D12_RANGE readRange1 = { 0,0 };
		m_indexBuffer->Map(0, &readRange1, &pIndexDataBegin);
		memcpy(pIndexDataBegin, triangleIndices, sizeof(triangleIndices));
		m_indexBuffer->Unmap(0, nullptr);

		m_indexBufferView.BufferLocation = m_indexBuffer->GetGPUVirtualAddress();
		m_indexBufferView.Format = DXGI_FORMAT_R32_UINT;
		m_indexBufferView.SizeInBytes = indicesBufferSize;
	}
}

void wxGraphicD3D12::CreateVertexBuffer(Vertex& vertex, size_t size)
{
	const UINT vertexBufferSize = sizeof(vertex) * size;
	ID3D12Resource* pVertexBuffer;
	// Note: using upload heaps to transfer static data like vert buffers is not 
	// recommended. Every time the GPU needs it, the upload heap will be marshalled 
	// over. Please read up on Default Heap usage. An upload heap is used here for 
	// code simplicity and because there are very few verts to actually transfer.
	ThrowIfFailed(m_device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&pVertexBuffer)));
	
	// Copy the triangle data to the vertex buffer.
	UINT8* pVertexDataBegin;
	CD3DX12_RANGE readRange(0, 0);		// We do not intend to read from this resource on the CPU.
	ThrowIfFailed(pVertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin)));
	memcpy(pVertexDataBegin, &vertex, vertexBufferSize);
	pVertexBuffer->Unmap(0, nullptr);

	// Initialize the vertex buffer view.
	m_vertexBufferView.BufferLocation = pVertexBuffer->GetGPUVirtualAddress();
	m_vertexBufferView.StrideInBytes = sizeof(Vertex);
	m_vertexBufferView.SizeInBytes = vertexBufferSize;
	m_vec_VertexBufferView.push_back(m_vertexBufferView);
}

void wxGraphicD3D12::CreateIndexBuffer(int& indice, size_t size)
{
	HRESULT hr;
	D3D12_HEAP_PROPERTIES indexHeapProperties;
	indexHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
	indexHeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	indexHeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	indexHeapProperties.CreationNodeMask = 1;
	indexHeapProperties.VisibleNodeMask = 1;

	//resource Description for index buffer
	D3D12_RESOURCE_DESC indexBufferDesc;
	indexBufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	indexBufferDesc.Alignment = 0;
	indexBufferDesc.Width = sizeof(indice) * size;
	indexBufferDesc.Height = 1;
	indexBufferDesc.DepthOrArraySize = 1;
	indexBufferDesc.MipLevels = 1;
	indexBufferDesc.Format = DXGI_FORMAT_UNKNOWN;
	indexBufferDesc.SampleDesc.Count = 1;
	indexBufferDesc.SampleDesc.Quality = 0;
	indexBufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	indexBufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	ID3D12Resource* indexBuffer;
	indexHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
	hr = m_device->CreateCommittedResource(&indexHeapProperties, D3D12_HEAP_FLAG_NONE, \
		&indexBufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, __uuidof(indexBuffer), (void**)&indexBuffer);

	void* pIndexDataBegin;
	D3D12_RANGE readRange1 = { 0,0 };
	indexBuffer->Map(0, &readRange1, &pIndexDataBegin);
	memcpy(pIndexDataBegin, &indice, sizeof(indice) * size);
	indexBuffer->Unmap(0, nullptr);

	m_indexBufferView.BufferLocation = indexBuffer->GetGPUVirtualAddress();
	m_indexBufferView.Format = DXGI_FORMAT_R32_UINT;
	m_indexBufferView.SizeInBytes = sizeof(indice) * size;
	m_vec_IndexBufferView.push_back(m_indexBufferView);
}

void wxGraphicD3D12::GenerateShadowMap()
{
	D3D12_RESOURCE_DESC texDesc;
	ZeroMemory(&texDesc, sizeof(D3D12_RESOURCE_DESC));
	texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	texDesc.Alignment = 0;
	texDesc.Width = 4096;
	texDesc.Height = 4096;
	texDesc.DepthOrArraySize = 1;
	texDesc.MipLevels = 1;
	texDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_CLEAR_VALUE optClear;
	optClear.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	optClear.DepthStencil.Depth = 1.0f;
	optClear.DepthStencil.Stencil = 0;

	ThrowIfFailed(m_device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&texDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		&optClear,
		IID_PPV_ARGS(&m_shadowDepthMap)));

	D3D12_CPU_DESCRIPTOR_HANDLE cbvHandle = m_srvHeap->GetCPUDescriptorHandleForHeapStart();
	cbvHandle.ptr += m_TypedDescriptorSize * (TYPE_END * GetSceneGeometryNodeCount() + SUNLIGHT_COUNT + CONSTANTMATRIX_COUNT);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
	srvDesc.Texture2D.PlaneSlice = 0;
	m_device->CreateShaderResourceView(m_shadowDepthMap.Get(), &srvDesc, cbvHandle);

	// Create DSV to resource so we can render to the shadow map.
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = m_dsvHeap->GetCPUDescriptorHandleForHeapStart();
	dsvHandle.ptr += m_DepthStencilDescriptorSize * DEFAULT_DSV_MAP_COUNT;
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsvDesc.Texture2D.MipSlice = 0;
	m_device->CreateDepthStencilView(m_shadowDepthMap.Get(), &dsvDesc, dsvHandle);

}

void wxGraphicD3D12::GenerateCameraDepthMap()
{
	D3D12_RESOURCE_DESC texDesc;
	ZeroMemory(&texDesc, sizeof(D3D12_RESOURCE_DESC));
	texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	texDesc.Alignment = 0;
	texDesc.Width = GetWidth();
	texDesc.Height = GetHeight();
	texDesc.DepthOrArraySize = 1;
	texDesc.MipLevels = 1;
	texDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_CLEAR_VALUE optClear;
	optClear.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	optClear.DepthStencil.Depth = 1.0f;
	optClear.DepthStencil.Stencil = 0;

	ThrowIfFailed(m_device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&texDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		&optClear,
		IID_PPV_ARGS(&m_cameraDepthMap)));

	D3D12_CPU_DESCRIPTOR_HANDLE cbvHandle = m_srvHeap->GetCPUDescriptorHandleForHeapStart();
	cbvHandle.ptr += m_TypedDescriptorSize * (TYPE_END * GetSceneGeometryNodeCount() + SUNLIGHT_COUNT + CONSTANTMATRIX_COUNT + SHADOW_DSV_MAP_COUNT + NormalMapCount + AmbientMapCount + RANDOM_VECTOR_MAP_COUNT + SSAO_CONSTANT_COUNT);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
	srvDesc.Texture2D.PlaneSlice = 0;
	m_device->CreateShaderResourceView(m_cameraDepthMap.Get(), &srvDesc, cbvHandle);

	// Create DSV to resource so we can render to the shadow map.
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = m_dsvHeap->GetCPUDescriptorHandleForHeapStart();
	dsvHandle.ptr += m_DepthStencilDescriptorSize * (DEFAULT_DSV_MAP_COUNT + SHADOW_DSV_MAP_COUNT);
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsvDesc.Texture2D.MipSlice = 0;
	m_device->CreateDepthStencilView(m_cameraDepthMap.Get(), &dsvDesc, dsvHandle);

}

void wxGraphicD3D12::GenerateNormalMap()
{
	D3D12_RESOURCE_DESC texDesc;
	ZeroMemory(&texDesc, sizeof(D3D12_RESOURCE_DESC));
	texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	texDesc.Alignment = 0;
	texDesc.Width = GetWidth();
	texDesc.Height = GetHeight();
	texDesc.DepthOrArraySize = 1;
	texDesc.MipLevels = 1;
	texDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

	D3D12_CLEAR_VALUE optClear;
	optClear.Color[0] = 0.0f; optClear.Color[1] = 0.0f; optClear.Color[2] = 1.0f; optClear.Color[3] = 0.0f;
	optClear.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	optClear.DepthStencil.Depth = 1.0f;
	optClear.DepthStencil.Stencil = 0;


	ThrowIfFailed(m_device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&texDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		&optClear,
		IID_PPV_ARGS(&m_NormalMap)));

	D3D12_CPU_DESCRIPTOR_HANDLE cbvHandle = m_srvHeap->GetCPUDescriptorHandleForHeapStart();
	cbvHandle.ptr += m_TypedDescriptorSize * (TYPE_END * GetSceneGeometryNodeCount() + SUNLIGHT_COUNT + CONSTANTMATRIX_COUNT + SHADOW_DSV_MAP_COUNT);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;
	m_device->CreateShaderResourceView(m_NormalMap.Get(), &srvDesc, cbvHandle);

	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_rtvHeap->GetCPUDescriptorHandleForHeapStart();
	rtvHandle.ptr += m_rtvDescriptorSize * FrameCount;

	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	rtvDesc.Texture2D.MipSlice = 0;
	rtvDesc.Texture2D.PlaneSlice = 0;
	m_device->CreateRenderTargetView(m_NormalMap.Get(), &rtvDesc, rtvHandle);
}

void wxGraphicD3D12::GenerateAmbientMap()
{
	D3D12_RESOURCE_DESC texDesc;
	ZeroMemory(&texDesc, sizeof(D3D12_RESOURCE_DESC));
	texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	texDesc.Alignment = 0;
	texDesc.Width = GetWidth()/2;
	texDesc.Height = GetHeight()/2;
	texDesc.DepthOrArraySize = 1;
	texDesc.MipLevels = 1;
	texDesc.Format = DXGI_FORMAT_R16_UNORM;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

	float normalClearColor[] = { 0.0f, 0.0f, 1.0f, 0.0f };
	D3D12_CLEAR_VALUE optClear;
	optClear.Color[0] = 0.0f; optClear.Color[1] = 0.0f; optClear.Color[2] = 1.0f; optClear.Color[3] = 0.0f;
	optClear.Format = DXGI_FORMAT_R16_UNORM;

	ThrowIfFailed(m_device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&texDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		&optClear,
		IID_PPV_ARGS(&m_AmbientMap1)));

	D3D12_CPU_DESCRIPTOR_HANDLE cbvHandle = m_srvHeap->GetCPUDescriptorHandleForHeapStart();
	cbvHandle.ptr += m_TypedDescriptorSize * (TYPE_END * GetSceneGeometryNodeCount() + SUNLIGHT_COUNT + CONSTANTMATRIX_COUNT + SHADOW_DSV_MAP_COUNT + NormalMapCount);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Format = DXGI_FORMAT_R16_UNORM;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;
	m_device->CreateShaderResourceView(m_AmbientMap1.Get(), &srvDesc, cbvHandle);

	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_rtvHeap->GetCPUDescriptorHandleForHeapStart();
	rtvHandle.ptr += m_rtvDescriptorSize * (FrameCount + NormalMapCount);

	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Format = DXGI_FORMAT_R16_UNORM;
	rtvDesc.Texture2D.MipSlice = 0;
	rtvDesc.Texture2D.PlaneSlice = 0;
	m_device->CreateRenderTargetView(m_AmbientMap1.Get(), &rtvDesc, rtvHandle);

	ThrowIfFailed(m_device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&texDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		&optClear,
		IID_PPV_ARGS(&m_AmbientMap2)));

	cbvHandle = m_srvHeap->GetCPUDescriptorHandleForHeapStart();
	cbvHandle.ptr += m_TypedDescriptorSize * (TYPE_END * GetSceneGeometryNodeCount() + SUNLIGHT_COUNT + CONSTANTMATRIX_COUNT + SHADOW_DSV_MAP_COUNT + NormalMapCount + 1);

	srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Format = DXGI_FORMAT_R16_UNORM;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;
	m_device->CreateShaderResourceView(m_AmbientMap2.Get(), &srvDesc, cbvHandle);

	rtvHandle = m_rtvHeap->GetCPUDescriptorHandleForHeapStart();
	rtvHandle.ptr += m_rtvDescriptorSize * (FrameCount + NormalMapCount + 1);

	rtvDesc = {};
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Format = DXGI_FORMAT_R16_UNORM;
	rtvDesc.Texture2D.MipSlice = 0;
	rtvDesc.Texture2D.PlaneSlice = 0;
	m_device->CreateRenderTargetView(m_AmbientMap2.Get(), &rtvDesc, rtvHandle);

}

void wxGraphicD3D12::GenerateRandomVectorTexture()
{
	D3D12_RESOURCE_DESC texDesc;
	ZeroMemory(&texDesc, sizeof(D3D12_RESOURCE_DESC));
	texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	texDesc.Alignment = 0;
	texDesc.Width = 256;
	texDesc.Height = 256;
	texDesc.DepthOrArraySize = 1;
	texDesc.MipLevels = 1;
	texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	texDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	ThrowIfFailed(m_device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&texDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_RandomVectorMap)));

	const UINT num2DSubresources = texDesc.DepthOrArraySize * texDesc.MipLevels;
	const UINT64 uploadBufferSize = GetRequiredIntermediateSize(m_RandomVectorMap.Get(), 0, num2DSubresources);

	ThrowIfFailed(m_device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(m_RandomVectorMapUploadBuffer.GetAddressOf())));

	XMCOLOR initData[256 * 256];
	for (int i = 0; i < 256; ++i)
	{
		for (int j = 0; j < 256; ++j)
		{
			// Random vector in [0,1].  We will decompress in shader to [-1,1].
			XMFLOAT3 v((float)(rand()) / (float)RAND_MAX, (float)(rand()) / (float)RAND_MAX, (float)(rand()) / (float)RAND_MAX);

			initData[i * 256 + j] = XMCOLOR(v.x, v.y, v.z, 0.0f);
		}
	}

	//Vector4FT initData[256 * 256];
	//for (int i = 0; i < 256; ++i)
	//{
		//for (int j = 0; j < 256; ++j)
		//{
	//		// Random vector in [0,1].  We will decompress in shader to [-1,1].
	//		XMFLOAT3 v((float)(rand()) / (float)RAND_MAX, (float)(rand()) / (float)RAND_MAX, (float)(rand()) / (float)RAND_MAX);

	//		initData[i * 256 + j] = Vector4FT{ v.x, v.y, v.z, 0.0f };
		//}
	//}

	D3D12_SUBRESOURCE_DATA subResourceData = {};
	subResourceData.pData = initData;
	subResourceData.RowPitch = 256 * sizeof(XMCOLOR);
	subResourceData.SlicePitch = subResourceData.RowPitch * 256;

	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_RandomVectorMap.Get(),
		D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_COPY_DEST));
	UpdateSubresources(m_commandList.Get(), m_RandomVectorMap.Get(), m_RandomVectorMapUploadBuffer.Get(),
		0, 0, num2DSubresources, &subResourceData);
	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_RandomVectorMap.Get(),
		D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ));

	D3D12_CPU_DESCRIPTOR_HANDLE cbvHandle = m_srvHeap->GetCPUDescriptorHandleForHeapStart();
	cbvHandle.ptr += m_TypedDescriptorSize * (TYPE_END * GetSceneGeometryNodeCount() + SUNLIGHT_COUNT + CONSTANTMATRIX_COUNT + SHADOW_DSV_MAP_COUNT + NormalMapCount + AmbientMapCount);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;
	m_device->CreateShaderResourceView(m_RandomVectorMap.Get(), &srvDesc, cbvHandle);
}

void wxGame::wxGraphicD3D12::BuildOffsetVectors()
{
	m_offsets[0] = Vector4FT{ +1.0f, +1.0f, +1.0f, 0.0f };
	m_offsets[1] = Vector4FT{ -1.0f, -1.0f, -1.0f, 0.0f };
	m_offsets[2] = Vector4FT{ -1.0f, +1.0f, +1.0f, 0.0f };
	m_offsets[3] = Vector4FT{ +1.0f, -1.0f, -1.0f, 0.0f };
	m_offsets[4] = Vector4FT{ +1.0f, +1.0f, -1.0f, 0.0f };
	m_offsets[5] = Vector4FT{ -1.0f, -1.0f, +1.0f, 0.0f };
	m_offsets[6] = Vector4FT{ -1.0f, +1.0f, -1.0f, 0.0f };
	m_offsets[7] = Vector4FT{ +1.0f, -1.0f, +1.0f, 0.0f };
	m_offsets[8] = Vector4FT{ -1.0f, 0.0f, 0.0f, 0.0f };
	m_offsets[9] = Vector4FT{ +1.0f, 0.0f, 0.0f, 0.0f };
	m_offsets[10] = Vector4FT{ 0.0f, -1.0f, 0.0f, 0.0f };
	m_offsets[11] = Vector4FT{ 0.0f, +1.0f, 0.0f, 0.0f };
	m_offsets[12] = Vector4FT{ 0.0f, 0.0f, -1.0f, 0.0f };
	m_offsets[13] = Vector4FT{ 0.0f, 0.0f, +1.0f, 0.0f };

	for (int i = 0; i < 14; i++)
	{
		float s = GetRandFloat(0.25f, 1.f);
		Vector4FT v = vectorNormalize(m_offsets[i]);
		m_offsets[i] = v * s;
	}
}

void wxGraphicD3D12::CreateTexture(std::vector<std::string>& title)
{
	BMPDecoder bmpDecoder;
	DataBuffer dataBuffer;
	ImageCommon imgCommon;
	HRESULT hr;
	m_vec_texture.resize(title.size());
	m_vec_textureUploadHeap.resize(title.size());
	for (int i = 0; i != title.size(); i++)
	{
#ifdef _X86
		bmpDecoder.BMPLoader((ASSET_DIRECTORY + title[i]).c_str());
		bmpDecoder.BMPParser(imgCommon);

#else
		bmpDecoder.BMPLoader((ASSET_DIRECTORY + title[i]).c_str());
		bmpDecoder.BMPParser(imgCommon);
#endif

		// Create the texture.
		{
			// Describe and create a Texture2D.
			D3D12_RESOURCE_DESC textureDesc = {};
			textureDesc.MipLevels = 1;
			textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			textureDesc.Width = imgCommon.imWidth;
			textureDesc.Height = imgCommon.imHeight;
			textureDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
			textureDesc.DepthOrArraySize = 1;
			textureDesc.SampleDesc.Count = 1;
			textureDesc.SampleDesc.Quality = 0;
			textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

			hr = m_device->CreateCommittedResource(
				&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
				D3D12_HEAP_FLAG_NONE,
				&textureDesc,
				D3D12_RESOURCE_STATE_COPY_DEST,
				nullptr,
				IID_PPV_ARGS(&(m_vec_texture[i])));

			UINT64 uploadBufferSize = 0;
			if (imgCommon.imWidth != 0 && imgCommon.imHeight != 0)
			{
				uploadBufferSize = GetRequiredIntermediateSize(m_vec_texture[i], 0, 1);
			}

			// Create the GPU upload buffer.
			hr = m_device->CreateCommittedResource(
				&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
				D3D12_HEAP_FLAG_NONE,
				&CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize),
				D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr,
				IID_PPV_ARGS(&(m_vec_textureUploadHeap[i])));

			// Copy data to the intermediate upload heap and then schedule a copy 
			// from the upload heap to the Texture2D.

			D3D12_SUBRESOURCE_DATA srvSubrecData = {};
			if (imgCommon.imBitCount == 24)
			{
				// DXGI does not support 24bit formats so we have to extend it to 32bit
				unsigned char new_pitch = imgCommon.imPitch / 3 * 4;
				size_t data_size = new_pitch * imgCommon.imHeight;
				DataBuffer* data = new DataBuffer(data_size);
				unsigned char* buf = reinterpret_cast<unsigned char*>(data->GetData());
				unsigned char* src = reinterpret_cast<unsigned char*>(imgCommon.imData);
				for (uint32_t row = 0; row < imgCommon.imHeight; row++)
				{
					buf = reinterpret_cast<unsigned char*>(data->GetData()) + row * new_pitch;
					src = reinterpret_cast<unsigned char*>(imgCommon.imData) + row * imgCommon.imPitch;
					for (unsigned char col = 0; col < imgCommon.imWidth; col++)
					{
						*(unsigned char*)buf = *(unsigned char*)src;
						buf[3] = 0;  // set alpha to 0
						buf += 4;
						src += 3;
					}
				}
				imgCommon.imData = data->GetData();
				imgCommon.imDataSize = data_size;
				imgCommon.imPitch = new_pitch;
			}
			int sizeData = sizeof(imgCommon.imData);
			srvSubrecData.pData = imgCommon.imData;
			srvSubrecData.RowPitch = imgCommon.imPitch;
			srvSubrecData.SlicePitch = imgCommon.imPitch * imgCommon.imHeight;

			if (imgCommon.imWidth != 0 && imgCommon.imHeight != 0)
			{
				UpdateSubresources(m_commandList.Get(), m_vec_texture[i], m_vec_textureUploadHeap[i].Get(), 0, 0, 1, &srvSubrecData);
			}
			m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_vec_texture[i], D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

			// Describe and create a SRV for the texture.
			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			srvDesc.Format = textureDesc.Format;
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			srvDesc.Texture2D.MipLevels = 1;
			CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor(m_srvHeap->GetCPUDescriptorHandleForHeapStart());
			hDescriptor.ptr += m_TypedDescriptorSize * m_textureSRVCount;
			m_device->CreateShaderResourceView(m_vec_texture[i], &srvDesc, hDescriptor);
			m_textureSRVCount++;
		}
	}
}

void wxGraphicD3D12::CreateConstantMaterialBuffer(std::vector<wxMaterial>& mat)
{
	for (int i = 0; i != mat.size(); i++)
	{
		HRESULT hr;
		D3D12_HEAP_PROPERTIES cbvHeapProperties;
		cbvHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
		cbvHeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		cbvHeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		cbvHeapProperties.CreationNodeMask = 1;
		cbvHeapProperties.VisibleNodeMask = 1;

		D3D12_RESOURCE_DESC constantMaterialDesc;
		constantMaterialDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		constantMaterialDesc.Alignment = 0;
		constantMaterialDesc.Width = sizeof(wxMaterial);
		constantMaterialDesc.Height = 1;
		constantMaterialDesc.DepthOrArraySize = 1;
		constantMaterialDesc.MipLevels = 1;
		constantMaterialDesc.Format = DXGI_FORMAT_UNKNOWN;// DXGI_FORMAT_D24_UNORM_S8_UINT;
		constantMaterialDesc.SampleDesc.Count = 1;
		constantMaterialDesc.SampleDesc.Quality = 0;
		constantMaterialDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		constantMaterialDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

		hr = m_device->CreateCommittedResource(&cbvHeapProperties, D3D12_HEAP_FLAG_NONE, &constantMaterialDesc, \
			D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, __uuidof(m_vec_matRes[i]), (void**)&m_vec_matRes[i]);

		const UINT64 uploadBufferSize = GetRequiredIntermediateSize(m_vec_matRes[i].Get(), 0, 1);
		// Map the constant buffers. Note that unlike D3D11, the resource 
		// does not need to be unmapped for use by the GPU. In this sample, 
		// the resource stays 'permenantly' mapped to avoid overhead with 
		// mapping/unmapping each frame.
		D3D12_RANGE readRange = { 0,0 };		// We do not intend to read from this resource on the CPU.
		m_vec_matRes[i]->Map(0, &readRange, &m_pwxMatDataBegin);
		memcpy(m_pwxMatDataBegin, &mat[i], sizeof(wxMaterial));
		D3D12_CPU_DESCRIPTOR_HANDLE cbvHandle = m_srvHeap->GetCPUDescriptorHandleForHeapStart();
		cbvHandle.ptr += m_TypedDescriptorSize * (TYPE_MATERIAL * GetSceneGeometryNodeCount() + i);

		// Describe and create a SRV for the material.
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Format = DXGI_FORMAT_UNKNOWN;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		srvDesc.Buffer.FirstElement = 0;
		srvDesc.Buffer.NumElements = 1;
		srvDesc.Buffer.StructureByteStride = sizeof(wxMaterial);
		srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

		m_device->CreateShaderResourceView(m_vec_matRes[i].Get(), &srvDesc, cbvHandle);
	}
}

void wxGraphicD3D12::CreateObjConst(std::vector<wxObjConst>& objConst)
{
	for (int i = 0; i != objConst.size(); i++)
	{
		const UINT64 uploadBufferSize = sizeof(wxObjConst);

		D3D12_HEAP_PROPERTIES cbvHeapProperties;
		cbvHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
		cbvHeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		cbvHeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		cbvHeapProperties.CreationNodeMask = 1;
		cbvHeapProperties.VisibleNodeMask = 1;

		D3D12_RESOURCE_DESC ConstResDesc;
		ConstResDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		ConstResDesc.Alignment = 0;
		ConstResDesc.Width = sizeof(wxObjConst);
		ConstResDesc.Height = 1;
		ConstResDesc.DepthOrArraySize = 1;
		ConstResDesc.MipLevels = 1;
		ConstResDesc.Format = DXGI_FORMAT_UNKNOWN;// DXGI_FORMAT_D24_UNORM_S8_UINT;
		ConstResDesc.SampleDesc.Count = 1;
		ConstResDesc.SampleDesc.Quality = 0;
		ConstResDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		ConstResDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
		m_device->CreateCommittedResource(&cbvHeapProperties, D3D12_HEAP_FLAG_NONE, &ConstResDesc, \
			D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, __uuidof(m_vec_objConstRes[i]), (void**)&m_vec_objConstRes[i]);

		D3D12_RANGE readRange = { 0,0 };		// We do not intend to read from this resource on the CPU.
		m_vec_objConstRes[i]->Map(0, &readRange, &m_pObjConstDataBegin);
		memcpy(m_pObjConstDataBegin, &objConst[i], sizeof(wxObjConst));

		// Describe and create a SRV for the texture.
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Format = DXGI_FORMAT_UNKNOWN;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		srvDesc.Buffer.FirstElement = 0;
		srvDesc.Buffer.NumElements = 1;
		srvDesc.Buffer.StructureByteStride = sizeof(wxObjConst);
		srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

		CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor(m_srvHeap->GetCPUDescriptorHandleForHeapStart());
		hDescriptor.ptr += m_TypedDescriptorSize * (GetSceneGeometryNodeCount() * TYPE_TRANSFORMMATRIX + i);
		m_device->CreateShaderResourceView(m_vec_objConstRes[i].Get(), &srvDesc, hDescriptor);
	}
}

void wxGraphicD3D12::CreateConstantMatrix()
{
	HRESULT hr;
	// Create CBV resource and bind it to the pipeline by ConstantBufferView.
		//resource Description for constant buffer
	D3D12_HEAP_PROPERTIES cbvHeapProperties;
	cbvHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
	cbvHeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	cbvHeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	cbvHeapProperties.CreationNodeMask = 1;
	cbvHeapProperties.VisibleNodeMask = 1;

	D3D12_RESOURCE_DESC constantBufferDesc;
	constantBufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	constantBufferDesc.Alignment = 0;
	constantBufferDesc.Width = sizeof(wxConstMatrix);
	constantBufferDesc.Height = 1;
	constantBufferDesc.DepthOrArraySize = 1;
	constantBufferDesc.MipLevels = 1;
	constantBufferDesc.Format = DXGI_FORMAT_UNKNOWN;// DXGI_FORMAT_D24_UNORM_S8_UINT;
	constantBufferDesc.SampleDesc.Count = 1;
	constantBufferDesc.SampleDesc.Quality = 0;
	constantBufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	constantBufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	hr = m_device->CreateCommittedResource(&cbvHeapProperties, D3D12_HEAP_FLAG_NONE, &constantBufferDesc, \
		D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, __uuidof(m_constantBuffer), (void**)&m_constantBuffer);

	constBuff.viewMatrix = BuildViewMatrix(m_defaultCameraPosition, m_defaultLookAt, m_defaultUp);
	constBuff.perspectiveMatrix = BuildPerspectiveMatrixForLH(0.25f * PI, m_aspectRatio, 1.0f, 1000.0f);

	// Map the constant buffers. Note that unlike D3D11, the resource 
	// does not need to be unmapped for use by the GPU. In this sample, 
	// the resource stays 'permenantly' mapped to avoid overhead with 
	// mapping/unmapping each frame.
	D3D12_RANGE readRange = { 0,0 };		// We do not intend to read from this resource on the CPU.
	m_constantBuffer->Map(0, &readRange, &m_pCBDataBegin);
	memcpy(m_pCBDataBegin, &constBuff, sizeof(wxConstMatrix));
	D3D12_CPU_DESCRIPTOR_HANDLE cbvHandle = m_srvHeap->GetCPUDescriptorHandleForHeapStart();
	cbvHandle.ptr += m_TypedDescriptorSize * (GetSceneGeometryNodeCount() * TYPE_END + SUNLIGHT_COUNT);

	D3D12_CONSTANT_BUFFER_VIEW_DESC constantBufferView = {};
	constantBufferView.BufferLocation = m_constantBuffer->GetGPUVirtualAddress();
	constantBufferView.SizeInBytes = ALIGN_256(sizeof(wxConstMatrix));
	m_device->CreateConstantBufferView(&constantBufferView, cbvHandle);				//constant matrix for all of the objects;
}

void wxGame::wxGraphicD3D12::CreateSSAOBuffer()
{
	HRESULT hr;
	// Create CBV resource and bind it to the pipeline by ConstantBufferView.
		//resource Description for constant buffer
	D3D12_HEAP_PROPERTIES cbvHeapProperties;
	cbvHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
	cbvHeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	cbvHeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	cbvHeapProperties.CreationNodeMask = 1;
	cbvHeapProperties.VisibleNodeMask = 1;

	D3D12_RESOURCE_DESC constantBufferDesc;
	constantBufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	constantBufferDesc.Alignment = 0;
	constantBufferDesc.Width = sizeof(wxSSAOConstant);
	constantBufferDesc.Height = 1;
	constantBufferDesc.DepthOrArraySize = 1;
	constantBufferDesc.MipLevels = 1;
	constantBufferDesc.Format = DXGI_FORMAT_UNKNOWN;// DXGI_FORMAT_D24_UNORM_S8_UINT;
	constantBufferDesc.SampleDesc.Count = 1;
	constantBufferDesc.SampleDesc.Quality = 0;
	constantBufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	constantBufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	hr = m_device->CreateCommittedResource(&cbvHeapProperties, D3D12_HEAP_FLAG_NONE, &constantBufferDesc, \
		D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, __uuidof(m_ssaoBuffer), (void**)&m_ssaoBuffer);

	m_constSSAOBuff.occlusionRadius = 0.5f;
	m_constSSAOBuff.occlusionFadeStart = 0.2f;
	m_constSSAOBuff.occlusionFadeEnd = 1.0f;
	m_constSSAOBuff.surfaceEpsilon = 0.05f;
	m_constSSAOBuff.ScreenSize.element[0] = GetWidth();
	m_constSSAOBuff.ScreenSize.element[1] = GetHeight();
	// Map the constant buffers. Note that unlike D3D11, the resource 
	// does not need to be unmapped for use by the GPU. In this sample, 
	// the resource stays 'permenantly' mapped to avoid overhead with 
	// mapping/unmapping each frame.
	D3D12_RANGE readRange = { 0,0 };		// We do not intend to read from this resource on the CPU.
	m_ssaoBuffer->Map(0, &readRange, &m_pCBSSAOBegin);
	memcpy(m_pCBSSAOBegin, &m_constSSAOBuff, sizeof(wxSSAOConstant));
	D3D12_CPU_DESCRIPTOR_HANDLE cbvHandle = m_srvHeap->GetCPUDescriptorHandleForHeapStart();
	cbvHandle.ptr += m_TypedDescriptorSize * (GetSceneGeometryNodeCount() * TYPE_END + SUNLIGHT_COUNT + CONSTANTMATRIX_COUNT + SHADOW_DSV_MAP_COUNT + NormalMapCount + AmbientMapCount + RANDOM_VECTOR_MAP_COUNT);
	
	D3D12_CONSTANT_BUFFER_VIEW_DESC constantBufferView = {};
	constantBufferView.BufferLocation = m_ssaoBuffer->GetGPUVirtualAddress();
	constantBufferView.SizeInBytes = ALIGN_256(sizeof(wxSSAOConstant));
	m_device->CreateConstantBufferView(&constantBufferView, cbvHandle);				//constant matrix for all of the objects;
}

void wxGraphicD3D12::CreateSunLightBuffer()
{
	HRESULT hr;
	D3D12_HEAP_PROPERTIES cbvHeapProperties;
	cbvHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
	cbvHeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	cbvHeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	cbvHeapProperties.CreationNodeMask = 1;
	cbvHeapProperties.VisibleNodeMask = 1;

	D3D12_RESOURCE_DESC constantSunLightDesc;
	constantSunLightDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	constantSunLightDesc.Alignment = 0;
	constantSunLightDesc.Width = sizeof(wxLight);
	constantSunLightDesc.Height = 1;
	constantSunLightDesc.DepthOrArraySize = 1;
	constantSunLightDesc.MipLevels = 1;
	constantSunLightDesc.Format = DXGI_FORMAT_UNKNOWN;// DXGI_FORMAT_D24_UNORM_S8_UINT;
	constantSunLightDesc.SampleDesc.Count = 1;
	constantSunLightDesc.SampleDesc.Quality = 0;
	constantSunLightDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	constantSunLightDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	hr = m_device->CreateCommittedResource(&cbvHeapProperties, D3D12_HEAP_FLAG_NONE, &constantSunLightDesc, \
		D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, __uuidof(m_sunLight), (void**)&m_sunLight);

	m_sunLightBuff.Direction = Vector3FT({ 0.f, 0.f, -1.f });
	m_sunLightBuff.Position = Vector3FT({ 300.f, 300.f, -300.f });
	m_sunLightBuff.Strength = Vector3FT({ 1.f,1.f,1.f });
	m_sunLightBuff.FalloffEnd = 0.f;
	m_sunLightBuff.FalloffStart = 0.f;
	m_sunLightBuff.SpotPower = 0.f;

	D3D12_RANGE readRange = { 0,0 };		// We do not intend to read from this resource on the CPU.
	m_sunLight->Map(0, &readRange, &m_pSunLightDataBegin);
	memcpy(m_pSunLightDataBegin, &m_sunLightBuff, sizeof(wxLight));
	D3D12_CPU_DESCRIPTOR_HANDLE cbvHandle = m_srvHeap->GetCPUDescriptorHandleForHeapStart();
	cbvHandle.ptr += m_TypedDescriptorSize * (GetSceneGeometryNodeCount() * TYPE_END);
	
	m_cbvSunLight.BufferLocation = m_sunLight->GetGPUVirtualAddress();
	m_cbvSunLight.SizeInBytes = ALIGN_256(sizeof(wxLight));
	m_device->CreateConstantBufferView(&m_cbvSunLight, cbvHandle);	//const light variable
}

void wxGraphicD3D12::CheckControllerInput()
{
	DWORD dwResult = 0;
	for (DWORD i = 0; i < XUSER_MAX_COUNT; i++)
	{
		XINPUT_STATE state;
		ZeroMemory(&state, sizeof(XINPUT_STATE));

		// Simply get the state of the controller from XInput.
		dwResult = XInputGetState(i, &state);

		if (state.dwPacketNumber == VK_PAD_LTRIGGER)
		{
			int abc = 1;
			int c = 2;
			int d = abc + c;
		}
		if (dwResult == ERROR_SUCCESS)
		{
			// Controller is connected 
			if (state.dwPacketNumber != m_controllerState)
			{
				float LX = state.Gamepad.sThumbLX;
				float LY = state.Gamepad.sThumbLY;

				float RX = state.Gamepad.sThumbRX;
				float RY = -state.Gamepad.sThumbRY;
				if (RX != 0 || RY != 0)
				{
			

					float magnitudeR = sqrt(RX*RX + RY * RY);
					if (RX * RX < 9000000)
					{
						RX = 0;
					}
					if (RY * RY < 9000000)
					{
						RY = 0;
					}

					float normalizedRX = RX / magnitudeR;
					float normalizedRY = RY / magnitudeR;

					Vector4FT viewDir = m_defaultLookAt - m_defaultCameraPosition;
					float length = vectorLength(viewDir);
					viewDir = vectorNormalize(viewDir);
					RotateYAxis(viewDir, normalizedRX * m_cameraRotationSpeed);
					Vector4FT tempRightAxis = GetRightAxis(viewDir, Vector4FT({ 0.f,1.f,0.f ,0.f}));
					RotateAxis(viewDir, tempRightAxis, normalizedRY * m_cameraRotationSpeed);
					viewDir = viewDir * length;
					m_defaultLookAt = viewDir + m_defaultCameraPosition;
					float normalizedMagnitude = 0;
				}

				if (LX != 0 || LY != 0)
				{
					//determine how far the controller is pushed
					float magnitude = sqrt(LX*LX + LY * LY);

					if (LX * LX < 9000000)
					{
						LX = 0;
					}
					if (LY * LY < 9000000)
					{
						LY = 0;
					}
					//determine the direction the controller is pushed
					float normalizedLX = LX / magnitude;
					float normalizedLY = LY / magnitude;

					Vector4FT viewDir = m_defaultLookAt - m_defaultCameraPosition; 
					Vector4FT rightDir = GetRightAxis(viewDir, Vector4FT({ 0.f,1.f,0.f,0.f }));
					Vector4FT velocity = vectorNormalize(viewDir) * normalizedLY + rightDir * normalizedLX;
					cameraDistance = velocity * m_cameraMoveBaseSpeed;
					m_defaultLookAt += cameraDistance;
					m_defaultCameraPosition += cameraDistance;
				}
				////check if the controller is outside a circular dead zone
				//if (magnitude > GAMEPAD_LEFT_THUMB_DEADZONE)
				//{
				//	//clip the magnitude at its expected maximum value
				//	if (magnitude > 32767) magnitude = 32767;

				//	//adjust magnitude relative to the end of the dead zone
				//	magnitude -= GAMEPAD_LEFT_THUMB_DEADZONE;

				//	//optionally normalize the magnitude with respect to its expected range
				//	//giving a magnitude value of 0.0 to 1.0
				//	normalizedMagnitude = magnitude / (32767 - GAMEPAD_LEFT_THUMB_DEADZONE);
				//}
				//else //if the controller is in the deadzone zero out the magnitude
				//{
				//	magnitude = 0.0;
				//	normalizedMagnitude = 0.0;
				//}
				constBuff.viewMatrix = BuildViewMatrix(m_defaultCameraPosition, m_defaultLookAt, { 0.f,1.f,0.f });
			}
		}
		else
		{
			// Controller is not connected 
		}
	}
}