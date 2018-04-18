#include"stdafx.h"
#include "wxGraphicD3D12.h"
#include "d3dx12.h"
#include "BMPDecoder.h"

std::wstring m_assetsPath = {0};
std::wstring GetAssetFullPath(LPCWSTR assetName)
{
	return m_assetsPath + assetName;
}

wxGraphicD3D12::wxGraphicD3D12() :
	angleAxisY(0.0f),
	isWarpDevice(false),
	m_rootSignature(nullptr)
{
	m_viewport = { 0.0f, 0.0f, (float)m_width, (float)m_height, 0.0f, 1.0f };
	m_scissorRect = { 0, 0, (long)m_width, (long)m_height };
}

wxGraphicD3D12::~wxGraphicD3D12()
{
	Close();
}

void wxGraphicD3D12::Initialize(void)
{
	LoadPipeline();
	LoadGraphicAssets();
}

void wxGraphicD3D12::LoadGraphicAssets(void)
{
	FileLoader fileLoader;
	BMPDecoder bmpDecoder;
	DataBuffer dataBuffer;
	ImageCommon imgCommon;
#ifdef _X86
	FileLoader::FileInfo fileVS = fileLoader.LoadBinary("../Debug/wxVS.cso"); //"../Debug/wxVS.cso"
	FileLoader::FileInfo filePS = fileLoader.LoadBinary("../Debug/wxPS.cso"); //"../Debug/wxPS.cso"
	dataBuffer = bmpDecoder.BMPLoader("../wxAsset/profile.bmp");
	imgCommon = bmpDecoder.BMPParser(dataBuffer);

#else
	FileLoader::FileInfo fileVS = fileLoader.LoadBinary("../x64/Debug/wxVS.cso"); //"../Debug/wxVS.cso"
	FileLoader::FileInfo filePS = fileLoader.LoadBinary("../x64/Debug/wxPS.cso"); //"../Debug/wxPS.cso"
	dataBuffer = bmpDecoder.BMPLoader("../wxAsset/profile.bmp");
	imgCommon = bmpDecoder.BMPParser(dataBuffer);
#endif
	HRESULT hr;

	hr = m_Device->GetDeviceRemovedReason();
	D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};
	featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

	if (m_Device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData)))
	{
		featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
	}

	D3D12_DESCRIPTOR_RANGE1 ranges[3];
	ranges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
	ranges[0].BaseShaderRegister = 0;
	ranges[0].RegisterSpace = 0;
	ranges[0].Flags = D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC;
	ranges[0].NumDescriptors = 1;
	ranges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	ranges[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	ranges[1].BaseShaderRegister = 0;
	ranges[1].RegisterSpace = 0;
	ranges[1].Flags = D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC;
	ranges[1].NumDescriptors = 1;
	ranges[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	ranges[2].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
	ranges[2].BaseShaderRegister = 0;
	ranges[2].RegisterSpace = 0;
	ranges[2].Flags = D3D12_DESCRIPTOR_RANGE_FLAG_NONE;
	ranges[2].NumDescriptors = 1;
	ranges[2].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3D12_ROOT_PARAMETER1 rootParameters[3];
	rootParameters[0].DescriptorTable.NumDescriptorRanges = 1;
	rootParameters[0].DescriptorTable.pDescriptorRanges = &ranges[0];
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	rootParameters[1].DescriptorTable.NumDescriptorRanges = 1;
	rootParameters[1].DescriptorTable.pDescriptorRanges = &ranges[1];
	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	rootParameters[2].DescriptorTable.NumDescriptorRanges = 1;
	rootParameters[2].DescriptorTable.pDescriptorRanges = &ranges[2];
	rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	D3D12_STATIC_SAMPLER_DESC sampler = {};
	sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
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

	D3D12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Version = featureData.HighestVersion;
	rootSignatureDesc.Desc_1_1.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	rootSignatureDesc.Desc_1_1.NumParameters = 3;
	rootSignatureDesc.Desc_1_1.pParameters = rootParameters;
	rootSignatureDesc.Desc_1_1.NumStaticSamplers = 0;// 1;
	rootSignatureDesc.Desc_1_1.pStaticSamplers = nullptr;// &sampler;

	ID3DBlob* signature;
	ID3DBlob* error;
	hr = D3D12SerializeVersionedRootSignature(&rootSignatureDesc, &signature, &error);

	hr = m_Device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature));

	hr = m_Device->GetDeviceRemovedReason();
	// Create the pipeline state objects
	D3D12_INPUT_ELEMENT_DESC InputElementDescs[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	D3D12_RASTERIZER_DESC RasterizerDefault;
	RasterizerDefault.FillMode = D3D12_FILL_MODE_SOLID;
	RasterizerDefault.CullMode = D3D12_CULL_MODE_BACK;	//this specified the not drawing face of the object
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

	// Describe and create the graphics pipeline state objects (PSO).
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.InputLayout = { InputElementDescs, sizeof(InputElementDescs) / sizeof(D3D12_INPUT_ELEMENT_DESC) };
	psoDesc.pRootSignature = m_rootSignature;
	psoDesc.VS = { fileVS.fileItself, (unsigned int)fileVS.fileLength };
	psoDesc.PS = {filePS.fileItself, (unsigned int)filePS.fileLength};
	psoDesc.RasterizerState = RasterizerDefault;
	psoDesc.BlendState = {false, true, defaultRenderTargetBlendDesc,defaultRenderTargetBlendDesc ,defaultRenderTargetBlendDesc,\
		defaultRenderTargetBlendDesc , defaultRenderTargetBlendDesc , defaultRenderTargetBlendDesc, defaultRenderTargetBlendDesc,\
		defaultRenderTargetBlendDesc };
	psoDesc.DepthStencilState = defaultDepthStencil;
	psoDesc.SampleMask = 0xffffffff;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R10G10B10A2_UNORM;
	psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	psoDesc.SampleDesc = { 1, 0 };

	hr = m_Device->CreateGraphicsPipelineState(&psoDesc,  __uuidof(m_pipelineState), (void**)&m_pipelineState);

	// Create the command list.
	hr = m_Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocators, m_pipelineState, __uuidof(m_commandList), (void**)&m_commandList);

	hr = m_Device->GetDeviceRemovedReason();
	//Create the vertex buffer.

	//Define the geometry for triangle.
	//vertices for indices
	//camera located at the positive value point of the z axis,then look at the negative direction
	Vertex triangleVertices[] = {
		{ { -0.25f, 0.25f,-0.25f ,1.0f },{ 1.0f, 0.0f, 0.0f, 1.0f },{ 0.5f, 0.0f } },//0 right corner of the frontface if you look forward to the positive Z axis.
		{ { -0.25f,-0.25f,-0.25f ,1.0f },{ 0.0f, 1.0f, 0.0f, 1.0f },{ 0.0f, 1.0f } },//1
		{ { 0.25f,  0.25f,-0.25f ,1.0f },{ 1.0f, 1.0f, 1.0f, 1.0f },{ 0.0f, 0.0f } },//2
		{ { 0.25f, -0.25f,-0.25f ,1.0f },{ 0.0f, 0.0f, 1.0f, 1.0f },{ 1.0f, 1.0f } },//3
		{ { 0.25f,  0.25f, 0.25f ,1.0f },{ 0.0f, 1.0f, 0.0f, 1.0f },{ 0.0f, 0.0f } },//4
		{ { 0.25f, -0.25f, 0.25f ,1.0f },{ 0.0f, 0.0f, 1.0f, 1.0f },{ 0.0f, 0.0f } },//5
		{ { -0.25f, 0.25f, 0.25f ,1.0f },{ 1.0f, 1.0f, 1.0f, 1.0f },{ 0.0f, 0.0f } },//6  if you look forward to the positive Z axis, number 4 vertex is the right coner of the backface 
		{ { -0.25f,-0.25f, 0.25f ,1.0f },{ 1.0f, 0.0f, 0.0f, 1.0f },{ 0.0f, 0.0f } },//7
	};
	const unsigned int vertexBufferSize = sizeof(triangleVertices);

	int triangleIndices[] = {
						0,1,3,
						0,3,2,
						6,7,1,
						6,1,0,
						4,5,7,
						4,7,6,
						2,3,5,
						2,5,4
					};
	const unsigned int indicesBufferSize = sizeof(triangleIndices);
	m_numIndices = indicesBufferSize / sizeof(int);

	//const unsigned int vertexBufferSize = sizeof(triangleVertices);
	//Note: using upload heaps to transfer static data like vertex buffers. is not recommended.
	//Every time the GPU needs it, the upload heap will be marshalled over. Please read up on 
	//Default Heap usage.An upload heap is used here for code simplicity and because there are 
	//very few verts to actually transfer.

	D3D12_HEAP_PROPERTIES commonHeapProperties;
	commonHeapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
	commonHeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	commonHeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	commonHeapProperties.CreationNodeMask = 1;
	commonHeapProperties.VisibleNodeMask = 1;

	//resource Description for vertex buffer
	D3D12_RESOURCE_DESC vertexBufferDesc;
	vertexBufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	vertexBufferDesc.Alignment = 0;
	vertexBufferDesc.Width = vertexBufferSize;
	vertexBufferDesc.Height = 1;
	vertexBufferDesc.DepthOrArraySize = 1;
	vertexBufferDesc.MipLevels = 1;
	vertexBufferDesc.Format = DXGI_FORMAT_UNKNOWN;
	vertexBufferDesc.SampleDesc.Count = 1;
	vertexBufferDesc.SampleDesc.Quality = 0;
	vertexBufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	vertexBufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
	
	commonHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
	hr = m_Device->CreateCommittedResource(&commonHeapProperties, D3D12_HEAP_FLAG_NONE, \
		&vertexBufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, __uuidof(m_vertexBuffer), (void**)&m_vertexBuffer);

	// Copy the triangle data to the vertex buffer.
	void* pVertexDataBegin;
	D3D12_RANGE readRange = { 0,0 };
	m_vertexBuffer->Map(0, &readRange, &pVertexDataBegin);
	memcpy(pVertexDataBegin, triangleVertices, sizeof(triangleVertices));
	m_vertexBuffer->Unmap(0, nullptr);

	//Initialize the vertex buffer view.
	m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
	m_vertexBufferView.StrideInBytes = sizeof(Vertex);
	m_vertexBufferView.SizeInBytes = vertexBufferSize;

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
	hr = m_Device->CreateCommittedResource(&indexHeapProperties, D3D12_HEAP_FLAG_NONE,\
		&indexBufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, __uuidof(m_indexBuffer), (void**)&m_indexBuffer);

	void* pIndexDataBegin;
	readRange = { 0,0 };
	m_indexBuffer->Map(0, &readRange, &pIndexDataBegin);
	memcpy(pIndexDataBegin, triangleIndices, sizeof(triangleIndices));
	m_indexBuffer->Unmap(0, nullptr);

	m_indexBufferView.BufferLocation = m_indexBuffer->GetGPUVirtualAddress();
	m_indexBufferView.Format = DXGI_FORMAT_R32_UINT;
	m_indexBufferView.SizeInBytes = indicesBufferSize;
	
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

	hr = m_Device->CreateCommittedResource(&dsHeapProperties, D3D12_HEAP_FLAG_NONE, &depthStencilDesc, \
		D3D12_RESOURCE_STATE_DEPTH_WRITE, &depthOptimizedClearValue, __uuidof(m_depthStencil), (void**)&m_depthStencil);

	// Create the depth stencil view.
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvBufferDesc = {};
	dsvBufferDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsvBufferDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvBufferDesc.Flags = D3D12_DSV_FLAG_NONE;
	// Create DepthStencilView
	m_Device->CreateDepthStencilView(m_depthStencil, &dsvBufferDesc, m_dsvHeap->GetCPUDescriptorHandleForHeapStart());


	hr = m_Device->GetDeviceRemovedReason();
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
	constantBufferDesc.Width = sizeof(constBuffer);
	constantBufferDesc.Height = 1;
	constantBufferDesc.DepthOrArraySize = 1;
	constantBufferDesc.MipLevels = 1;
	constantBufferDesc.Format = DXGI_FORMAT_UNKNOWN;// DXGI_FORMAT_D24_UNORM_S8_UINT;
	constantBufferDesc.SampleDesc.Count = 1;
	constantBufferDesc.SampleDesc.Quality = 0;
	constantBufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	constantBufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	hr = m_Device->CreateCommittedResource(&cbvHeapProperties, D3D12_HEAP_FLAG_NONE, &constantBufferDesc, \
		D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, __uuidof(m_constantBufferHeap), (void**)&m_constantBufferHeap);

	// Map the constant buffers. Note that unlike D3D11, the resource 
	// does not need to be unmapped for use by the GPU. In this sample, 
	// the resource stays 'permenantly' mapped to avoid overhead with 
	// mapping/unmapping each frame.
	readRange = { 0,0 };		// We do not intend to read from this resource on the CPU.
	m_constantBufferHeap->Map(0, &readRange, &m_pCBDataBegin);
	memcpy(m_pCBDataBegin, &constBuff, sizeof(constBuffer));

	D3D12_CPU_DESCRIPTOR_HANDLE cbvHandle = m_srvCbvHeap->GetCPUDescriptorHandleForHeapStart();

	D3D12_CONSTANT_BUFFER_VIEW_DESC constantBufferView = {};
	constantBufferView.BufferLocation = m_constantBufferHeap->GetGPUVirtualAddress();
	constantBufferView.SizeInBytes = sizeof(constBuffer);
	m_Device->CreateConstantBufferView(&constantBufferView, cbvHandle);

	hr = m_Device->GetDeviceRemovedReason();

	D3D12_HEAP_PROPERTIES texHeapProperties;
	texHeapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
	texHeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	texHeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	texHeapProperties.CreationNodeMask = 1;
	texHeapProperties.VisibleNodeMask = 1;

	D3D12_RESOURCE_DESC texBufferDesc = {};
	texBufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;;
	//texBufferDesc.Alignment = 0;
	texBufferDesc.Width = TextureWidth;
	texBufferDesc.Height = TextureHeight;
	texBufferDesc.DepthOrArraySize = 1;
	texBufferDesc.MipLevels = 1;
	texBufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;// DXGI_FORMAT_D24_UNORM_S8_UINT;
	texBufferDesc.SampleDesc.Count = 1;
	texBufferDesc.SampleDesc.Quality = 0;
	//texBufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	texBufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	hr = m_Device->CreateCommittedResource(&texHeapProperties, D3D12_HEAP_FLAG_NONE, &texBufferDesc, \
		D3D12_RESOURCE_STATE_COPY_DEST, nullptr, __uuidof(m_texBuffer), (void**)&m_texBuffer);

	hr = m_Device->GetDeviceRemovedReason();
	unsigned int subresourceCount = texBufferDesc.DepthOrArraySize * texBufferDesc.MipLevels;
	unsigned __int64 uploadBufferSize;
	m_Device->GetCopyableFootprints(&texBufferDesc, 0, subresourceCount, 0, nullptr, nullptr, nullptr, &uploadBufferSize);

	D3D12_RESOURCE_DESC texUploadDesc = {};
	texUploadDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	texUploadDesc.Alignment = 0;
	texUploadDesc.Width = uploadBufferSize;
	texUploadDesc.Height = 1;
	texUploadDesc.DepthOrArraySize = 1;
	texUploadDesc.MipLevels = 1;
	texUploadDesc.Format = DXGI_FORMAT_UNKNOWN;
	texUploadDesc.SampleDesc.Count = 1;
	texUploadDesc.SampleDesc.Quality = 0;
	texUploadDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	texUploadDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	texHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
	hr = m_Device->CreateCommittedResource(&texHeapProperties, D3D12_HEAP_FLAG_NONE, &texUploadDesc, \
		D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, __uuidof(m_texUploadBuffer), (void**)&m_texUploadBuffer);


	hr = m_Device->GetDeviceRemovedReason();
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


	std::vector<UINT8> texture = GenerateTextureData();
	D3D12_SUBRESOURCE_DATA textureData = {};
	textureData.pData = &texture[0];
	textureData.RowPitch = TextureWidth * TexturePixelSize;
	textureData.SlicePitch = textureData.RowPitch * TextureHeight;

	UpdateSubresources(m_commandList, m_texBuffer, m_texUploadBuffer, 0, 0, 1, &textureData);

	//D3D12_SUBRESOURCE_DATA* pSubResourceData = &srvSubrecData;
	//reinterpret_cast<const unsigned char*>((const_cast<void*>(srvSubrecData.pData)));

	//unsigned __int64* pTotalBytes = nullptr;
	//// size_subres is the counts of the imgCommon subresource.
	//long size_subres = (sizeof(D3D12_PLACED_SUBRESOURCE_FOOTPRINT) + sizeof(unsigned int) + sizeof(__int64)) * subresourceCount;
	//// alloc the size_subres bytes to copy data to intermetiate buffer.
	//DataBuffer subResData(size_subres);
	//// we specified the corresponding type to the sub buffer.
	//void* pSubResData = subResData.GetData();
	//D3D12_PLACED_SUBRESOURCE_FOOTPRINT* pSubLayoutFootprint = reinterpret_cast<D3D12_PLACED_SUBRESOURCE_FOOTPRINT*>(pSubResData);
	//unsigned __int64* pRowSizesInBytes = reinterpret_cast<unsigned __int64*>(pSubLayoutFootprint + subresourceCount);
	//unsigned int* pNumRows = reinterpret_cast<unsigned int*>(pRowSizesInBytes + subresourceCount);

	//m_Device->GetCopyableFootprints(&m_texBuffer->GetDesc(), 0, subresourceCount, 0, pSubLayoutFootprint, pNumRows, pRowSizesInBytes, pTotalBytes);

	//D3D12_RANGE readRange1 = { 0,0 };
	//hr = m_texUploadBuffer->Map(0, &readRange1, &m_pSRDataBegin);

	////copy the corresponding subresource to immediate buffer.
	//for (unsigned itr = 0; itr < subresourceCount; itr++)
	//{
	//	D3D12_MEMCPY_DEST destData = { reinterpret_cast<unsigned char*>(m_pSRDataBegin) + pSubLayoutFootprint[itr].Offset,\
	//		pSubLayoutFootprint[itr].Footprint.RowPitch, pSubLayoutFootprint[itr].Footprint.RowPitch * pNumRows[itr] };

	//	for (unsigned int z = 0; z != pSubLayoutFootprint[z].Footprint.Depth; z++)
	//	{
	//		unsigned char* pDestSlice = reinterpret_cast<unsigned char*>((&destData)->pData) + destData.SlicePitch * z;
	//		unsigned char* pSrcSlice = (unsigned char*)(const_cast<void*>(pSubResourceData[itr].pData)) + pSubResourceData[itr].SlicePitch * z;

	//		for (unsigned int y = 0; y < pNumRows[itr]; y++)
	//		{
	//			memcpy(pDestSlice + destData.RowPitch * y, pSrcSlice + pSubResourceData[itr].RowPitch * y, pRowSizesInBytes[itr]);
	//		}
	//	}
	//}


	//D3D12_MEMCPY_DEST destData = { m_pSRDataBegin };
	//for (unsigned int itr = 0; itr < subresourceCount; itr++)
	//{
	//	D3D12_TEXTURE_COPY_LOCATION destLoc = { m_texUploadBuffer, D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT, pSubLayoutFootprint[itr] };
	//	D3D12_TEXTURE_COPY_LOCATION srctLoc = { m_texBuffer, D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX, itr };
	//	m_commandList->CopyTextureRegion(&destLoc, 0, 0, 0, &srctLoc, nullptr);
	//}

	
	
	D3D12_RESOURCE_BARRIER barrier = {};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = m_texBuffer;
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	m_commandList->ResourceBarrier(1, &barrier);

	D3D12_SHADER_RESOURCE_VIEW_DESC shaderResourceView;
	shaderResourceView.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	shaderResourceView.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	shaderResourceView.Texture2D = { 0U, 1U, 0U, 0.0f };
	shaderResourceView.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;

	D3D12_CPU_DESCRIPTOR_HANDLE srcDescStart = m_srvCbvHeap->GetCPUDescriptorHandleForHeapStart();
	unsigned int srvTextureSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	srcDescStart.ptr += srvTextureSize;

	hr = m_Device->GetDeviceRemovedReason();

	m_Device->CreateShaderResourceView(m_texBuffer, &shaderResourceView, srcDescStart);
	delete[] imgCommon.imData;
	imgCommon.imData = nullptr;

	hr = m_Device->GetDeviceRemovedReason();
	//Create sampler
	D3D12_SAMPLER_DESC samplerDesc = {};
	samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	samplerDesc.MinLOD = 0.0f;
	samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;

	m_Device->CreateSampler(&samplerDesc, m_samplerHeap->GetCPUDescriptorHandleForHeapStart());
	// find the head pointer of the RTV Heap
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_rtvHeap->GetCPUDescriptorHandleForHeapStart();

	hr = m_Device->GetDeviceRemovedReason();
	// Create a RTV for each frame.
	for (unsigned int n = 0; n < SWAP_CHAIN_BUFFER_COUNT; n++)
	{
		m_swapChain->GetBuffer(n, __uuidof(m_renderTargets[n]), (void**)&m_renderTargets[n]);
		m_Device->CreateRenderTargetView(m_renderTargets[n], nullptr, rtvHandle);
		m_rtvDescriptorSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		rtvHandle.ptr += m_rtvDescriptorSize;
	}

	hr = m_Device->GetDeviceRemovedReason();
	//Create synchronization objects and wait until assets have been uploaded to the GPU.
	hr = m_Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, __uuidof(m_fence), (void**)&m_fence);
	m_fenceValue = 1;
	
	//Create an event handle to use for frame synchronization.
	m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	if (m_fenceEvent == nullptr)
	{
		HRESULT_FROM_WIN32(GetLastError());
	}

	// Command lists are created in the recording state, but there is nothing
	// to record yet. The main loop expects it to be closed, so close it now.
	m_commandList->Close();

	// Wait for the command list to execute; we are reusing the same command 
	// list in our main loop but for now, we just want to wait for setup to 
	// complete before continuing.
	WaitForGpuPrevFrame();
}



// Generate a simple black and white checkerboard texture.
std::vector<unsigned char> wxGraphicD3D12::GenerateTextureData()
{
	const UINT rowPitch = TextureWidth * TexturePixelSize;
	const UINT cellPitch = rowPitch >> 3;		// The width of a cell in the checkboard texture.
	const UINT cellHeight = TextureWidth >> 3;	// The height of a cell in the checkerboard texture.
	const UINT textureSize = rowPitch * TextureHeight;

	std::vector<UINT8> data(textureSize);
	UINT8* pData = &data[0];

	for (UINT n = 0; n < textureSize; n += TexturePixelSize)
	{
		UINT x = n % rowPitch;
		UINT y = n / rowPitch;
		UINT i = x / cellPitch;
		UINT j = y / cellHeight;

		if (i % 2 == j % 2)
		{
			pData[n] = 0x00;		// R
			pData[n + 1] = 0x00;	// G
			pData[n + 2] = 0x00;	// B
			pData[n + 3] = 0xff;	// A
		}
		else
		{
			pData[n] = 0xff;		// R
			pData[n + 1] = 0xff;	// G
			pData[n + 2] = 0xff;	// B
			pData[n + 3] = 0xff;	// A
		}
	}

	return data;
}


void wxGraphicD3D12::LoadPipeline(void)
{
	ID3D12Device* pDevice = nullptr;
#if _DEBUG
	ID3D12Debug* pInterDebug;
	HRESULT hr = D3D12GetDebugInterface(__uuidof(pInterDebug), (void**)&pInterDebug);
	if (SUCCEEDED(hr))
		pInterDebug->EnableDebugLayer();
	else
		std::cout << "can't enable debug layer" << std::endl;
#endif
	IDXGIFactory4* pDxgiFactory;
	hr = CreateDXGIFactory2(0, __uuidof(pDxgiFactory), (void**)&pDxgiFactory);
	if (!SUCCEEDED(hr))
		std::cout << "Create factory failed" << std::endl;

	bool isWarpDriver = false;
	IDXGIAdapter1* pDxgiAdapter;
	if (!isWarpDriver)
	{
		for (uint32_t Idx = 0; DXGI_ERROR_NOT_FOUND != pDxgiFactory->EnumAdapters1(Idx, &pDxgiAdapter); ++Idx)
		{
			DXGI_ADAPTER_DESC1 descAdap;
			pDxgiAdapter->GetDesc1(&descAdap);
			if (descAdap.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
				continue;

			pDxgiAdapter->GetDesc1(&descAdap);
			hr = D3D12CreateDevice(pDxgiAdapter, D3D_FEATURE_LEVEL_11_0, __uuidof(m_Device), (void**)&m_Device);
			if (!SUCCEEDED(hr))
				std::cout << "Create device failed" << std::endl;
		}
	}

	D3D12_FEATURE_DATA_D3D12_OPTIONS D3D12options = {};
	if (!SUCCEEDED(m_Device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS, &D3D12options, sizeof(D3D12options))))
	{
		std::cout << "graphics driver don't support Direct3D 12 features" << std::endl;
	}

	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS multiSampleQuality;
	multiSampleQuality.Format = DXGI_FORMAT_R10G10B10A2_UNORM;
	multiSampleQuality.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
	multiSampleQuality.NumQualityLevels = 0;
	multiSampleQuality.SampleCount = 4;
	if (!SUCCEEDED(m_Device->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &multiSampleQuality, sizeof(multiSampleQuality))))
	{
		std::cout << "Device do not support 4XMSAA" << std::endl;
	}
	std::cout << "MSAA quality level is : " << multiSampleQuality.NumQualityLevels << std::endl;

	// Describe and create the command queue.
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	m_Device->CreateCommandQueue(&queueDesc, __uuidof(m_commandQueue),(void**)&m_commandQueue);
	//hr = m_Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, __uuidof(m_commandAllocators), (void**)&m_commandAllocators);

	//Describe and create the swap chain.
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.Width = m_width;
	swapChainDesc.Height = m_height;
	swapChainDesc.Format = DXGI_FORMAT_R10G10B10A2_UNORM;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = SWAP_CHAIN_BUFFER_COUNT;
	swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

	IDXGISwapChain1* swapChain;
	hr = pDxgiFactory->CreateSwapChainForHwnd(m_commandQueue, WndStuff::GetHwnd(), &swapChainDesc, nullptr, nullptr, &swapChain);
	//HDR support
	m_swapChain = (IDXGISwapChain4*)swapChain;
	IDXGIOutput* output;
	hr = m_swapChain->GetContainingOutput(&output);
	UINT colorSpaceSupport;
	if (SUCCEEDED(hr))
	{
		IDXGIOutput6* output6;
		hr = output->QueryInterface(__uuidof(output6), (void**)&output6);
		if (SUCCEEDED(hr))
		{
			DXGI_OUTPUT_DESC1 descOutput;
			hr = output6->GetDesc1(&descOutput);
			if (SUCCEEDED(hr))
			{
				hr = E_FAIL;
				if (descOutput.ColorSpace == DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020)
					hr = m_swapChain->CheckColorSpaceSupport(DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020, &colorSpaceSupport);

				if (SUCCEEDED(hr))
				{
					if ((colorSpaceSupport & DXGI_SWAP_CHAIN_COLOR_SPACE_SUPPORT_FLAG_PRESENT) && SUCCEEDED(m_swapChain->SetColorSpace1(DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020)))
					{
						std::cout << "Setting HDR mode success!" << std::endl;
					}
					else
					{
						std::cout << "Setting HDR mode failed!" << std::endl;
					}
				}
				else
				{
					std::cout << "This monitor do not support HDR mode!" << std::endl;
				}
			}
		}
	}

	m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

	// Describe and create a shader resource view (SRV) descriptor heap and constant buffer view(CBV) descriptor heap.
	D3D12_DESCRIPTOR_HEAP_DESC srvCbvHeapDesc = {};
	srvCbvHeapDesc.NumDescriptors = 1 + 1;		//first one for the constant buffer which keep Matrix in storage.Second one for SRV(texture), 
	srvCbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvCbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	hr = m_Device->CreateDescriptorHeap(&srvCbvHeapDesc, __uuidof(m_srvCbvHeap), (void**)&m_srvCbvHeap);

	// Describe and create a depth stencil view (DSV) descriptor heap.
	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	m_Device->CreateDescriptorHeap(&dsvHeapDesc, __uuidof(m_dsvHeap), (void**)&m_dsvHeap);

	//Create descriptor heaps for sampler
	D3D12_DESCRIPTOR_HEAP_DESC samplerHeapDesc = {};
	samplerHeapDesc.NumDescriptors = 1;
	samplerHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
	samplerHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	hr = m_Device->CreateDescriptorHeap(&samplerHeapDesc, __uuidof(m_samplerHeap), (void**)&m_samplerHeap);

	// Create descriptor heaps
	// Describe and create a render target view (RTV) descriptor heap.
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
	rtvHeapDesc.NumDescriptors = 3;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	hr = m_Device->CreateDescriptorHeap(&rtvHeapDesc, __uuidof(m_rtvHeap), (void**)&m_rtvHeap);

	hr = m_Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, __uuidof(m_commandAllocators), (void**)&m_commandAllocators);

}

void wxGraphicD3D12::WaitForGpuPrevFrame()
{
	const long int fence = m_fenceValue;
	HRESULT hr = m_commandQueue->Signal(m_fence, fence);
	m_fenceValue++;

	//Wait until the fence has been processed.
	if (m_fence->GetCompletedValue() < fence)
	{
		hr = m_fence->SetEventOnCompletion(fence, m_fenceEvent);
		hr = WaitForSingleObject(m_fenceEvent, 0xFFFFFFFF);
	}

	m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
	// m_frameIndex = (m_frameIndex++) % SWAP_CHAIN_BUFFER_COUNT;
}

void wxGraphicD3D12::PopulateCommandList()
{
	// Command list allocators can only be reset when the associated command lists have finished execution on the GPU;
	// Apps should use fences to determine GPU execution progress.
	m_commandAllocators->Reset();

	// However, when ExecuteCommandList() is called on a particular command list, that command list can then be reset
	//at any time and must be before re-recording.
	m_commandList->Reset(m_commandAllocators, m_pipelineState);
	
	// Set neccessary state.
	m_commandList->RSSetViewports(1, &m_viewport);
	m_commandList->RSSetScissorRects(1, &m_scissorRect);

	m_commandList->SetGraphicsRootSignature(m_rootSignature);
	ID3D12DescriptorHeap* ppHeaps[] = { m_srvCbvHeap , m_samplerHeap };
	m_commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

	//graphics root descriptor table for transformation matrix
	m_commandList->SetGraphicsRootDescriptorTable(0, m_srvCbvHeap->GetGPUDescriptorHandleForHeapStart());

	//// Indicate that the back buffer will be used as a texture.
	//D3D12_RESOURCE_BARRIER barrier = {};
	//barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	//barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	//barrier.Transition.pResource = m_texBuffer;
	//barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	//barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	//barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	//m_commandList->ResourceBarrier(1, &barrier);

	//graphics root descriptor table for texutre
	D3D12_GPU_DESCRIPTOR_HANDLE srcDescStart = m_srvCbvHeap->GetGPUDescriptorHandleForHeapStart();
	unsigned int SrvTextureSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	srcDescStart.ptr += SrvTextureSize;
	m_commandList->SetGraphicsRootDescriptorTable(1, srcDescStart);

	//graphics root descriptor table for sampler
	m_commandList->SetGraphicsRootDescriptorTable(2, m_samplerHeap->GetGPUDescriptorHandleForHeapStart());
	// Indicate that the back buffer will be used as a render target.
	D3D12_RESOURCE_BARRIER resourceBarrierRT;
	resourceBarrierRT.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	resourceBarrierRT.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	resourceBarrierRT.Transition.pResource = m_renderTargets[m_frameIndex];
	resourceBarrierRT.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	resourceBarrierRT.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	resourceBarrierRT.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	m_commandList->ResourceBarrier(1, (const D3D12_RESOURCE_BARRIER*)&resourceBarrierRT);

	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle;
	rtvHandle.ptr = m_rtvHeap->GetCPUDescriptorHandleForHeapStart().ptr + m_frameIndex * m_rtvDescriptorSize;	//find render target Heap
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = m_dsvHeap->GetCPUDescriptorHandleForHeapStart();	//find depth Stencil Heap
	m_commandList->OMSetRenderTargets(1, &rtvHandle, false, &dsvHandle);		//depthstencilheap bound to this.

	// Record commands.
	const float clearColor[] = { 0.0f,0.2f,0.4f,1.0f };
	m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
	m_commandList->ClearDepthStencilView(m_dsvHeap->GetCPUDescriptorHandleForHeapStart(), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
	m_commandList->IASetIndexBuffer(&m_indexBufferView);
	m_commandList->DrawIndexedInstanced(m_numIndices, 1, 0, 0, 0);
	//m_commandList->DrawInstanced(6, 1, 0, 0);

	D3D12_RESOURCE_BARRIER resourceBarrierPresent;
	resourceBarrierPresent.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	resourceBarrierPresent.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	resourceBarrierPresent.Transition.pResource = m_renderTargets[m_frameIndex];
	resourceBarrierPresent.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	resourceBarrierPresent.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	resourceBarrierPresent.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	//Indicate that the back buffer will now be used to present
	m_commandList->ResourceBarrier(1, (const D3D12_RESOURCE_BARRIER*)&resourceBarrierPresent);
	m_commandList->Close();

}

// Render the scene.
void wxGraphicD3D12::OnRender(void)
{
	// Record all the commands we need to render the scene into the command list.
	PopulateCommandList();

	//Execute the command list.
	ID3D12CommandList* ppCommandLists[] = { m_commandList };
	m_commandQueue->ExecuteCommandLists(sizeof(ppCommandLists) / sizeof(ID3D12CommandList), ppCommandLists);

	// Present the frame.
	m_swapChain->Present(1, 0);

	WaitForGpuPrevFrame();
}

void wxGraphicD3D12::OnUpdate(void)
{
	UpdateConstantBuffer();
}

void wxGraphicD3D12::UpdateConstantBuffer(void)
{
	angleAxisY -= 0.01f;
	int k = static_cast<int>(angleAxisY) / 360;
	angleAxisY = angleAxisY + k * 360;
	constBuffer matResult;
	//matResult.rotatYMatrix = XMMatrixRotationY(angleAxisY);
	//matResult.viewMatrix = XMMatrixLookToLH({ 0,0,10 }, { 0,0,-1 }, { 0,1,0 });
	//matResult.perspectiveMatrix = XMMatrixPerspectiveFovLH(0.4f * 3.1415f, m_aspect_ratio, 1.0f, 1000.0f);
	//matResult.wvpMatrix = matResult.viewMatrix * matResult.perspectiveMatrix * matResult.rotatYMatrix;

	matResult.rotatYMatrix = MatrixRotationY(angleAxisY);
	matResult.viewMatrix = BuildViewMatrix({ 0,0,-2 }, { 0,0,1 }, { 0,1,0 });
	
	matResult.perspectiveMatrix = BuildPerspectiveMatrixForLH(0.4f * PI, m_aspect_ratio, 1.0f, 1000.0f);

	matResult.wvpMatrix = MatrixMultiMatrix(MatrixMultiMatrix(matResult.viewMatrix, matResult.perspectiveMatrix), matResult.rotatYMatrix);
	
	memcpy(m_pCBDataBegin, &matResult, sizeof(constBuffer));
}

void wxGraphicD3D12::Close()
{
	// Ensure that the GPU is no longer referencing resources that are about to be
	// cleaned up by the destructor.
	m_constantBufferHeap->Unmap(0,nullptr);
	{
		const __int64 fence = m_fenceValue;
		const __int64 lastCompletedFence = m_fence->GetCompletedValue();

		// Signal and increment the fence value.
		m_commandQueue->Signal(m_fence, m_fenceValue);
		m_fenceValue++;

		// Wait until the previous frame is finished.
		if (lastCompletedFence < fence)
		{
			m_fence->SetEventOnCompletion(fence, m_fenceEvent);
			WaitForSingleObject(m_fenceEvent, INFINITE);
		}
	}

	for (UINT i = 0; i < SWAP_CHAIN_BUFFER_COUNT; i++)
	{
		delete m_renderTargets[i];
	}
}