//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

#include "stdafx.h"
#include "wxGraphicD3D12.h"
#include "SceneManager.h"
#define ASSET_DIRECTORY	"../wxAsset/"
wxGraphicD3D12::wxGraphicD3D12(UINT width, UINT height, std::wstring name) :
	GraphicD3D12(width, height, name),
	m_frameIndex(0),
	m_viewport(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height)),
	m_scissorRect(0, 0, static_cast<LONG>(width), static_cast<LONG>(height)),
	m_rtvDescriptorSize(0)
{
}

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
		dsvHeapDesc.NumDescriptors = 1;
		dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		m_device->CreateDescriptorHeap(&dsvHeapDesc, __uuidof(m_dsvHeap), (void**)&m_dsvHeap);

		// Describe and create a render target view (RTV) descriptor heap.
		D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
		rtvHeapDesc.NumDescriptors = FrameCount;
		rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		ThrowIfFailed(m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap)));

		// Describe and create a shader resource view (SRV) heap for the texture.
		D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
		srvHeapDesc.NumDescriptors = m_vec_TextureTitle.size() + GetSceneGeometryNodeCount() + CONSTANTMATRIX_COUNT + SUNLIGHT_COUNT;	//GeometryNode count is equal with material count
		srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
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
		fileLoader.GetNameByNameAndSuffix(m_vec_TextureTitle, ASSET_DIRECTORY, MATCH_TEXTURE, SUFFIX_BMP, it);
	}
}
// Load the sample assets.
void wxGraphicD3D12::LoadAssets()
{
	HRESULT hr;
	// Create the root signature.
	{
		D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};

		// This is the highest version the sample supports. If CheckFeatureSupport succeeds, the HighestVersion returned will not be greater than this.
		featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

		if (FAILED(m_device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
		{
			featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
		}

		CD3DX12_DESCRIPTOR_RANGE1 ranges[2];
		ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);
		ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 3, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);

		CD3DX12_ROOT_PARAMETER1 rootParameters[2];
		rootParameters[0].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_PIXEL);
		rootParameters[1].InitAsDescriptorTable(1, &ranges[1], D3D12_SHADER_VISIBILITY_ALL);

		D3D12_STATIC_SAMPLER_DESC sampler = {};
		sampler.Filter = D3D12_FILTER_ANISOTROPIC;
		sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		sampler.MipLODBias = 0;
		sampler.MaxAnisotropy = 0;
		sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
		sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
		sampler.MinLOD = 0.0f;
		sampler.MaxLOD = D3D12_FLOAT32_MAX;
		sampler.ShaderRegister = 0;
		sampler.RegisterSpace = 0;
		sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

		CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
		rootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, 1, &sampler, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

		ComPtr<ID3DBlob> signature;
		ComPtr<ID3DBlob> error;
		ThrowIfFailed(D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, featureData.HighestVersion, &signature, &error));
		ThrowIfFailed(m_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature)));
	}

	// Create the pipeline state, which includes compiling and loading shaders.
	{
		ComPtr<ID3DBlob> vertexShader;
		ComPtr<ID3DBlob> pixelShader;

#if defined(_DEBUG)
		// Enable better shader debugging with the graphics debugging tools.
		UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
		UINT compileFlags = 0;
#endif
		ThrowIfFailed(D3DCompileFromFile(GetAssetFullPath(L"..\\..\\..\\shaders.hlsl").c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "VSMain", "vs_5_0", compileFlags, 0, &vertexShader, nullptr));
		ThrowIfFailed(D3DCompileFromFile(GetAssetFullPath(L"..\\..\\..\\shaders.hlsl").c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "PSMain", "ps_5_0", compileFlags, 0, &pixelShader, nullptr));

		// Define the vertex input layout.
		D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 16, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		};
		D3D12_RASTERIZER_DESC RasterizerDefault;
		RasterizerDefault.FillMode = D3D12_FILL_MODE_SOLID;
		RasterizerDefault.CullMode = D3D12_CULL_MODE_NONE;	//this specified the not drawing face of the object
		RasterizerDefault.FrontCounterClockwise = true;
		RasterizerDefault.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
		RasterizerDefault.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
		RasterizerDefault.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
		RasterizerDefault.DepthClipEnable = FALSE;
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
		psoDesc.RasterizerState = RasterizerDefault;// CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		psoDesc.DepthStencilState = defaultDepthStencil;
		psoDesc.SampleMask = UINT_MAX;
		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		psoDesc.NumRenderTargets = 1;
		psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		psoDesc.SampleDesc.Count = 1;
		ThrowIfFailed(m_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState)));
	}

	// Create the command list.
	ThrowIfFailed(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator.Get(), m_pipelineState.Get(), IID_PPV_ARGS(&m_commandList)));

	m_TypedDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	ParserDataFromScene(m_vec_AssetFileTitle);
	CreateTexture(m_vec_TextureTitle);

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
	depthStencilDesc.MipLevels = 0;
	depthStencilDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_CLEAR_VALUE depthOptimizedClearValue = {};
	depthOptimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
	depthOptimizedClearValue.DepthStencil.Depth = 1.0f;
	depthOptimizedClearValue.DepthStencil.Stencil = 0;

	m_device->CreateCommittedResource(&dsHeapProperties, D3D12_HEAP_FLAG_NONE, &depthStencilDesc, \
		D3D12_RESOURCE_STATE_DEPTH_WRITE, &depthOptimizedClearValue, __uuidof(m_depthStencil), (void**)&m_depthStencil);

	// Create the depth stencil view.
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvBufferDesc = {};
	dsvBufferDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsvBufferDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvBufferDesc.Flags = D3D12_DSV_FLAG_NONE;
	// Create DepthStencilView
	m_device->CreateDepthStencilView(m_depthStencil.Get(), &dsvBufferDesc, m_dsvHeap->GetCPUDescriptorHandleForHeapStart());

	{
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
		constantBufferDesc.Width = sizeof(wxConstBuffer);
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

		// Map the constant buffers. Note that unlike D3D11, the resource 
		// does not need to be unmapped for use by the GPU. In this sample, 
		// the resource stays 'permenantly' mapped to avoid overhead with 
		// mapping/unmapping each frame.
		D3D12_RANGE readRange = { 0,0 };		// We do not intend to read from this resource on the CPU.
		m_constantBuffer->Map(0, &readRange, &m_pCBDataBegin);

		D3D12_CPU_DESCRIPTOR_HANDLE cbvHandle = m_srvHeap->GetCPUDescriptorHandleForHeapStart();
		cbvHandle.ptr += m_TypedDescriptorSize * (m_textureSRVCount + m_MaterialCount + SUNLIGHT_COUNT);

		D3D12_CONSTANT_BUFFER_VIEW_DESC constantBufferView = {};
		constantBufferView.BufferLocation = m_constantBuffer->GetGPUVirtualAddress();
		constantBufferView.SizeInBytes = sizeof(wxConstBuffer);
		m_device->CreateConstantBufferView(&constantBufferView, cbvHandle);
	}

	CreateSunLightBuffer();

	// Note: ComPtr's are CPU objects but this resource needs to stay in scope until
	// the command list that references it has finished executing on the GPU.
	// We will flush the GPU at the end of this method to ensure the resource is not
	// prematurely destroyed.

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

void wxGraphicD3D12::CreateTexture(std::vector<std::string>& title)
{
	BMPDecoder bmpDecoder;
	DataBuffer dataBuffer;
	ImageCommon imgCommon;
	HRESULT hr;
	m_vec_texture.resize(title.size());
	m_vec_textureUploadHeap.resize(title.size());
	for(int i = 0; i!= title.size(); i++)
	{
#ifdef _X86
		dataBuffer = bmpDecoder.BMPLoader((ASSET_DIRECTORY + title[i]).c_str());
		imgCommon = bmpDecoder.BMPParser(dataBuffer);

#else
		dataBuffer = bmpDecoder.BMPLoader((ASSET_DIRECTORY + title[i]).c_str());
		imgCommon = bmpDecoder.BMPParser(dataBuffer);
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

			const UINT64 uploadBufferSize = GetRequiredIntermediateSize(m_vec_texture[i], 0, 1);

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
				// DXGI does not have 24bit formats so we have to extend it to 32bit
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

			UpdateSubresources(m_commandList.Get(), m_vec_texture[i], m_vec_textureUploadHeap[i].Get(), 0, 0, 1, &srvSubrecData);
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
			delete[] imgCommon.imData;
			imgCommon.imData = nullptr;
		}
	}
}

// Update frame-based values.
void wxGraphicD3D12::OnUpdate()
{
	UpdateConstantBuffer();
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

void wxGraphicD3D12::PopulateCommandList()
{
	// Command list allocators can only be reset when the associated 
	// command lists have finished execution on the GPU; apps should use 
	// fences to determine GPU execution progress.
	ThrowIfFailed(m_commandAllocator->Reset());

	// However, when ExecuteCommandList() is called on a particular command 
	// list, that command list can then be reset at any time and must be before 
	// re-recording.
	ThrowIfFailed(m_commandList->Reset(m_commandAllocator.Get(), m_pipelineState.Get()));

	// Set necessary state.
	m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());

	ID3D12DescriptorHeap* ppHeaps[] = { m_srvHeap.Get() };
	m_commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

	D3D12_GPU_DESCRIPTOR_HANDLE srvConstantBuff = m_srvHeap->GetGPUDescriptorHandleForHeapStart();		//begin from m_vec_cbvMat and Matrix4X4FT constantBuff and m_sunLightBuff
	srvConstantBuff.ptr += m_TypedDescriptorSize * (m_textureSRVCount);
	m_commandList->SetGraphicsRootDescriptorTable(1, srvConstantBuff);

	m_commandList->RSSetViewports(1, &m_viewport);
	m_commandList->RSSetScissorRects(1, &m_scissorRect);

	// Indicate that the back buffer will be used as a render target.
	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_frameIndex, m_rtvDescriptorSize);
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = m_dsvHeap->GetCPUDescriptorHandleForHeapStart();
	m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

	m_commandList->ClearDepthStencilView(m_dsvHeap->GetCPUDescriptorHandleForHeapStart(), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	// Record commands.
	const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
	m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
	m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	for (int i = 0; i < m_vec_VertexBufferView.size(); i++)
	{
		D3D12_GPU_DESCRIPTOR_HANDLE srvOffset = m_srvHeap->GetGPUDescriptorHandleForHeapStart();
		srvOffset.ptr += i * m_TypedDescriptorSize;
		m_commandList->SetGraphicsRootDescriptorTable(0, srvOffset);
		m_commandList->IASetVertexBuffers(0, 1, &(m_vec_VertexBufferView[i]));
		m_commandList->IASetIndexBuffer(&m_vec_IndexBufferView[i]);
		m_commandList->DrawIndexedInstanced(m_vec_numIndices[i], 1, 0, 0, 0);
	}

	// Indicate that the back buffer will now be used to present.
	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

	ThrowIfFailed(m_commandList->Close());
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
	angleAxisY -= 0.01f;
	if (angleAxisY < -360.f)
	{
		angleAxisY = 0.f;
	}
	int k = static_cast<int>(angleAxisY) / 360;
	angleAxisY = angleAxisY + k * 360;
	//wxConstBuffer matResult;
	constBuff.rotatMatrix = MatrixRotationY(angleAxisY);
	constBuff.viewMatrix = BuildViewMatrix({ 0,-2,-50.f }, { 0,0,0.f }, { 0.f,1.f,0.f });
	constBuff.perspectiveMatrix = BuildPerspectiveMatrixForLH(0.25f * PI, m_aspectRatio, 1.0f, 100.0f);
	memcpy(m_pCBDataBegin, &constBuff, sizeof(wxConstBuffer));
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
						if (_itr != scene.Geometries.end())
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
							}
							SceneObjectTransform* transform = geoNode->GetTransform(0);
							if (transform)
							{
								constBuff.linearTransMatrix = *(transform->GetTransformMatrix());
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
								Color tempColor = matNode->GetDiffuseColor();
								mat.diffuse = Vector4FT({ tempColor.Value.element[0], tempColor.Value.element[1], tempColor.Value.element[2] , 1.0f});
								//mat.m_AmbientOcclusion = matNode->GetAmbientOcclusion();
								mat.FresnelR0 = Vector3FT({ matNode->GetSpecular().Value.element[0], matNode->GetSpecular().Value.element[1], matNode->GetSpecular().Value.element[2] });
								m_vec_matRes.push_back(ComPtr<ID3D12Resource>());
								m_vec_cbvMat.push_back(D3D12_CONSTANT_BUFFER_VIEW_DESC());
								m_vec_matStut.push_back(mat);
								m_MaterialCount++;
							}
						}
					}
				}
			}
	}

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
		cbvHandle.ptr += m_TypedDescriptorSize * (m_textureSRVCount + i);

		m_vec_cbvMat[i].BufferLocation = m_vec_matRes[i]->GetGPUVirtualAddress();
		m_vec_cbvMat[i].SizeInBytes = ALIGN_256(sizeof(wxMaterial));
		m_device->CreateConstantBufferView(&m_vec_cbvMat[i], cbvHandle);
	}
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

	m_sunLightBuff.Direction = Vector3FT({ -1.f,-1.f,-1.f });
	m_sunLightBuff.Position = Vector3FT({ 100.f,100.f,100.f });
	m_sunLightBuff.Strength = Vector3FT({ 0.8f,0.8f,0.8f });
	m_sunLightBuff.FalloffEnd = 0.f;
	m_sunLightBuff.FalloffStart = 0.f;
	m_sunLightBuff.SpotPower = 0.f;

	D3D12_RANGE readRange = { 0,0 };		// We do not intend to read from this resource on the CPU.
	m_sunLight->Map(0, &readRange, &m_psunLightDataBegin);
	memcpy(m_psunLightDataBegin, &m_sunLightBuff, sizeof(wxLight));
	D3D12_CPU_DESCRIPTOR_HANDLE cbvHandle = m_srvHeap->GetCPUDescriptorHandleForHeapStart();
	cbvHandle.ptr += m_TypedDescriptorSize * (m_textureSRVCount + m_MaterialCount);
	
	m_cbvSunLight.BufferLocation = m_sunLight->GetGPUVirtualAddress();
	m_cbvSunLight.SizeInBytes = ALIGN_256(sizeof(wxLight));
	m_device->CreateConstantBufferView(&m_cbvSunLight, cbvHandle);
}