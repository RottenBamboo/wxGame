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

#pragma once

#include "GraphicD3D12.h"
#include "Mathmatic.h"
#include "FileLoader.h"
#include "BMPDecoder.h"
#include "SceneObject.h"
#include "SceneNode.h"
#include "SceneManager.h"

using namespace DirectX;
using namespace Mathmatic;
using namespace wxGame;

// Note that while ComPtr is used to manage the lifetime of resources on the CPU,
// it has no understanding of the lifetime of resources on the GPU. Apps must account
// for the GPU lifetime of resources to avoid destroying objects that may still be
// referenced by the GPU.
// An example of this can be found in the class method: OnDestroy().
using Microsoft::WRL::ComPtr;

namespace wxGame {
	class wxGraphicD3D12 : public GraphicD3D12
	{
	public:
		wxGraphicD3D12(UINT width, UINT height, std::wstring name);

		virtual void OnInit();
		virtual void OnUpdate();
		virtual void OnRender();
		virtual void OnDestroy();

	private:
		static const UINT FrameCount = 2;
		static const UINT TextureWidth = 256;
		static const UINT TextureHeight = 256;
		static const UINT TexturePixelSize = 4;	// The number of bytes used to represent a pixel in the texture.

		struct Vertex
		{
			Vector4FT position;
			Vector2FT uv;
		};

		struct constBuffer
		{
			Matrix4X4FT linearTransMatrix;
			Matrix4X4FT viewMatrix;
			Matrix4X4FT perspectiveMatrix;
			Matrix4X4FT rotatMatrix;

			constBuffer()// :rotatYMatrix(0),projMatrix(0),perspectiveMatrix(0), lightMaitrx(0)
			{
				MatrixIdentity(linearTransMatrix);
				MatrixIdentity(viewMatrix);
				MatrixIdentity(perspectiveMatrix);
				MatrixIdentity(rotatMatrix);
			}
		};

		void* m_pCBDataBegin;			//constant buffer data pointer
		constBuffer constBuff;

		// Pipeline objects.
		CD3DX12_VIEWPORT m_viewport;
		CD3DX12_RECT m_scissorRect;
		ComPtr<IDXGISwapChain3> m_swapChain;ComPtr<ID3D12Resource> textureUploadHeap;
		ComPtr<ID3D12Device> m_device;
		ComPtr<ID3D12Resource> m_renderTargets[FrameCount];
		ComPtr<ID3D12Resource> m_depthStencil;
		ComPtr<ID3D12Resource> m_constantBuffer;
		ComPtr<ID3D12Resource> m_indexBuffer;
		ComPtr<ID3D12CommandAllocator> m_commandAllocator;
		ComPtr<ID3D12CommandQueue> m_commandQueue;
		ComPtr<ID3D12RootSignature> m_rootSignature;
		ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
		ComPtr<ID3D12DescriptorHeap> m_srvHeap;
		ComPtr<ID3D12DescriptorHeap> m_dsvHeap;
		ComPtr<ID3D12PipelineState> m_pipelineState;
		ComPtr<ID3D12GraphicsCommandList> m_commandList;
		UINT m_rtvDescriptorSize;

		// App resources.
		ComPtr<ID3D12Resource> m_vertexBuffer;
		D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
		std::vector<D3D12_VERTEX_BUFFER_VIEW> m_vec_VertexBufferView;
		D3D12_INDEX_BUFFER_VIEW m_indexBufferView;
		std::vector<D3D12_INDEX_BUFFER_VIEW> m_vec_IndexBufferView;
		ComPtr<ID3D12Resource> m_texture;
		std::vector<ComPtr<ID3D12Resource>> m_vec_texture;
		ComPtr<ID3D12Resource> m_textureUploadHeap;
		std::vector<ComPtr<ID3D12Resource>> m_vec_textureUploadHeap;

		std::vector<std::string> m_vec_AssetFileTitle;
		std::vector<std::string> m_vec_TextureTitle;
		unsigned int m_numIndices;
		std::vector<unsigned int> m_vec_numIndices;
		float angleAxisY;
		D3D12_SHADER_RESOURCE_VIEW_DESC srv;
		// Synchronization objects.
		UINT m_frameIndex;
		HANDLE m_fenceEvent;
		ComPtr<ID3D12Fence> m_fence;
		UINT64 m_fenceValue;
		int m_textureResCount;
		int m_TypedDescriptorSize;

		void LoadPipeline();
		void LoadAssets();
		void RetrievalAssetDirectory();
		void CreateTexture(std::vector<std::string>&);
		void LoadVertexIndexDataFromFile(std::vector<std::string>&);
		void LoadDefaultVertexIndexData();
		void CreateVertexBuffer(Vertex&, size_t);
		void CreateIndexBuffer(int&, size_t);
		void LoadSceneNode(const BaseSceneNode& baseSceneNode);
		std::vector<UINT8> GenerateTextureData();
		void PopulateCommandList();
		void WaitForPreviousFrame();
		void UpdateConstantBuffer(void);
	};

	extern SceneManager* g_pSceneManager;
}