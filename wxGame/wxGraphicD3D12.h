#pragma once
#include "GraphicD3D12.h"
#include "WndStuff.h"
#include "Mathmatic.h"
#include "FileLoader.h"
#include "DirectXMath.h"
#include <vector>
#include <wrl.h>

#define SWAP_CHAIN_BUFFER_COUNT 3
using namespace Mathmatic;
using namespace DirectX;

class wxGraphicD3D12 : public GraphicD3D12
{
public:
	wxGraphicD3D12();
	~wxGraphicD3D12();
	virtual void Initialize(void);
	virtual void LoadPipeline(void);
	virtual void LoadGraphicAssets(void);
	virtual void OnRender(void);
	virtual void OnUpdate(void);
	void Close(void);
	void WaitForGpuPrevFrame();
	void PopulateCommandList();
	void UpdateConstantBuffer(void);
	std::vector<unsigned char> GenerateTextureData();

	static const UINT TextureWidth = 256;
	static const UINT TextureHeight = 256;
	static const UINT TexturePixelSize = 4;	// The number of bytes used to represent a pixel in the texture.

private:

	struct constBuffer
	{
		Matrix4X4FT rotatYMatrix;
		Matrix4X4FT viewMatrix;
		Matrix4X4FT perspectiveMatrix;
		Matrix4X4FT wvpMatrix;

		constBuffer()// :rotatYMatrix(0),projMatrix(0),perspectiveMatrix(0), lightMaitrx(0)
		{
			MatrixIdentity(rotatYMatrix);
			MatrixIdentity(viewMatrix);
			MatrixIdentity(perspectiveMatrix);
			MatrixIdentity(wvpMatrix);
		}
		/*XMMATRIX rotatYMatrix;
		XMMATRIX viewMatrix;
		XMMATRIX perspectiveMatrix;
		XMMATRIX wvpMatrix;
		constBuffer():rotatYMatrix(XMMatrixIdentity()), viewMatrix(XMMatrixIdentity()),
					  perspectiveMatrix(XMMatrixIdentity()), wvpMatrix(XMMatrixIdentity()){}
*/
	};

	struct Vertex
	{
		Vector3FT position;
		Vector4FT color;
		Vector2FT texCoord;
	};

	float angleAxisY;
	constBuffer constBuff;
	bool isWarpDevice;
	void* m_pCBDataBegin;			//constant buffer data pointer
	void* m_pSRDataBegin;
	D3D12_VIEWPORT m_viewport;
	D3D12_RECT m_scissorRect;
	ID3D12Device* m_Device;
	IDXGISwapChain4* m_swapChain;
	ID3D12CommandQueue* m_commandQueue;
	ID3D12DescriptorHeap* m_rtvHeap;
	ID3D12DescriptorHeap* m_srvCbvHeap;
	ID3D12DescriptorHeap* m_dsvHeap;
	ID3D12DescriptorHeap* m_samplerHeap;
	ID3D12CommandAllocator* m_commandAllocators;
	ID3D12GraphicsCommandList* m_commandList;
	ID3D12Resource* m_vertexBuffer;
	ID3D12Resource* m_indexBuffer;
	ID3D12Resource* m_renderTargets[3];
	ID3D12Resource* m_depthStencil;
	ID3D12Resource* m_constantBufferHeap;
	ID3D12Resource* m_texBuffer;
	ID3D12Resource* m_texUploadBuffer;
	uint32_t m_rtvDescriptorSize;
	ID3D12RootSignature* m_rootSignature;
	ID3D12PipelineState* m_pipelineState;
	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
	D3D12_INDEX_BUFFER_VIEW m_indexBufferView;
	unsigned int m_numIndices;
	ID3D12Fence* m_fence;
	long int m_fenceValue;
	unsigned int m_frameIndex;
	HANDLE m_fenceEvent;
};