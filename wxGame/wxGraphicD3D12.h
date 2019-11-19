#pragma once

#include "GraphicD3D12.h"
#include "Mathmatic.h"
#include "FileLoader.h"
#include "BMPDecoder.h"
#include "SceneObject.h"
#include "SceneNode.h"
#include "SceneManager.h"
#include "Intersection.h"
#include "Primitive.h"

#define SUNLIGHT_COUNT (int)1
#define SSAO_CONSTANT_COUNT (int)1
#define DEFAULT_DSV_MAP_COUNT (int)1
#define SHADOW_DSV_MAP_COUNT (int)1
#define CAMERA_DSV_MAP_COUNT (int)1
#define CONSTANTMATRIX_COUNT (int)1
#define SHADOW_CONSTANTMATRIX_COUNT (int)1
#define RANDOM_VECTOR_MAP_COUNT (int)1
#define GAMEPAD_LEFT_THUMB_DEADZONE 16354 
#define GAMEPAD_RIGHT_THUMB_DEADZONE 16354
#define CONST_256_SQURE 65536
#define E 2.718281828
#define MATCH_OGEX "[\\S\\s]*\\.ogex$"
#define SUFFIX_OGEX ".ogex"
#define MATCH_TEXTURE "_texture_[0-9]*\\.bmp$"
#define SUFFIX_BMP ".bmp"
#define MATCH_NORMALMAP "_normalmap_[0-9]*\\.bmp$"

using namespace DirectX;
using namespace Mathmatic;
using namespace wxGame;

using Microsoft::WRL::ComPtr;

namespace wxGame {
	class wxGraphicD3D12 : public GraphicD3D12
	{
	public:
		wxGraphicD3D12(UINT width, UINT height, std::wstring name);

		virtual void OnInit();
		virtual void OnUpdate(wxTimer *timer);
		virtual void OnRender();
		virtual void OnDestroy();

	private:
		static const UINT FrameCount = 2;
		static const UINT NormalMapCount = 1;
		static const UINT AmbientMapCount = 2;
		static const UINT TextureWidth = 256;
		static const UINT TextureHeight = 256;
		static const UINT TexturePixelSize = 4;	// The number of bytes used to represent a pixel in the texture.

		enum SRV_TYPE
		{
			TYPE_TEXTURE = 0,
			TYPE_NORMAL_MAP,
			TYPE_MATERIAL,
			TYPE_TRANSFORMMATRIX,
			TYPE_SHADOW_MAP,
			TYPE_END
		};
	public:

		struct wxObjConst
		{
			Matrix4X4FT linearTransMatrix;
			BoundingBox boundingBox;
			wxObjConst()
			{
				boundingBox = BoundingBox();
				MatrixIdentity(linearTransMatrix);
			}
		};

		struct wxConstMatrix
		{
			Matrix4X4FT viewMatrix;
			Matrix4X4FT perspectiveMatrix;
			Matrix4X4FT rotatMatrix;
			Matrix4X4FT shadowTransform;
			Matrix4X4FT lightOthgraphicMatrix;
			Matrix4X4FT lightViewMatrix;
			Matrix4X4FT lightTransformNDC;
			Matrix4X4FT invProjMatrix;
			Vector4FT cameraPos;
			Vector4FT viewPos;

			wxConstMatrix()// :rotatYMatrix(0),projMatrix(0),perspectiveMatrix(0), lightMaitrx(0)
			{
				MatrixIdentity(viewMatrix);
				MatrixIdentity(perspectiveMatrix);
				MatrixIdentity(rotatMatrix);
				MatrixIdentity(shadowTransform);
				MatrixIdentity(invProjMatrix);
				cameraPos = { 0.f,0.f,0.f,1.f };
				viewPos = { 0.f,0.f,1.f };
			}
		};

		struct wxMaterial
		{
			Vector4FT diffuse;
			Vector3FT FresnelR0;
			float Roughness;
			wxMaterial()
			{
				diffuse = Vector4FT({ 1.0f, 1.0f, 1.0f, 1.0f });
				Roughness = 0.1f;
				FresnelR0 = Vector3FT({0.01f, 0.01f, 0.01f });
			}
		};

		struct wxLight
		{
			Vector3FT Strength;
			float FalloffStart;                          // point/spot light only
			Vector3FT Direction;// directional/spot light only
			float FalloffEnd;                           // point/spot light only
			Vector3FT Position;  // point/spot light only
			float SpotPower;                            // spot light only
		};

		struct wxSSAOConstant
		{
			Vector4FT offsetVectors[14];
			Vector4FT blurWeights[3];
			float occlusionRadius;
			float occlusionFadeStart;
			float occlusionFadeEnd;
			float surfaceEpsilon;
			Vector2FT ScreenSize;
		};

	private:
		void* m_pCBSSAOBegin;
		wxSSAOConstant m_constSSAOBuff;
		void* m_pCBDataBegin;			//constant buffer data pointer
		wxConstMatrix constBuff;
		void* m_pwxMatDataBegin;
		wxLight m_sunLightBuff;
		void* m_pSunLightDataBegin;
		wxObjConst m_objConst;
		//void* m_pObjConstDataBegin;
		Vector4FT m_offsets[14]; 

		// Pipeline objects.
		CD3DX12_VIEWPORT m_viewport;
		CD3DX12_RECT m_scissorRect;
		CD3DX12_VIEWPORT m_ssao_viewport;
		CD3DX12_RECT m_ssao_scissorRect;
		CD3DX12_VIEWPORT m_shadowmap_Viewport;
		CD3DX12_RECT m_shadowmap_ScissorRect;
		ComPtr<IDXGISwapChain3> m_swapChain;ComPtr<ID3D12Resource> textureUploadHeap;
		ComPtr<ID3D12Device> m_device;
		ComPtr<ID3D12Resource> m_renderTargets[FrameCount];
		ComPtr<ID3D12Resource> m_depthStencil;
		ComPtr<ID3D12Resource> m_constantBuffer;
		ComPtr<ID3D12Resource> m_sunLight;
		ComPtr<ID3D12Resource> m_ssaoBuffer;
		ComPtr<ID3D12Resource> m_indexBuffer;
		ComPtr<ID3D12CommandAllocator> m_commandAllocator;
		ComPtr<ID3D12CommandQueue> m_commandQueue;
		ComPtr<ID3D12RootSignature> m_rootSignature;
		ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
		ComPtr<ID3D12DescriptorHeap> m_srvHeap;
		ComPtr<ID3D12DescriptorHeap> m_dsvHeap;
		ComPtr<ID3D12PipelineState> m_defaultPipelineState;
		ComPtr<ID3D12PipelineState> m_boundingBoxPipelineState;
		ComPtr<ID3D12PipelineState> m_shadowMapPipelineState;
		ComPtr<ID3D12PipelineState> m_DrawNormalPipelineState;
		ComPtr<ID3D12PipelineState> m_SsaoPipelineState;
		ComPtr<ID3D12PipelineState> m_BlurSsaoPipelineState;
		ComPtr<ID3D12GraphicsCommandList> m_commandList;
		UINT m_rtvDescriptorSize;

		// App resources.
		std::vector<D3D12_VERTEX_BUFFER_VIEW> m_vec_VertexBufferView;
		std::vector<D3D12_INDEX_BUFFER_VIEW> m_vec_IndexBufferView;
		std::vector<D3D12_VERTEX_BUFFER_VIEW> m_vec_boundingBoxVertexBufferView;
		std::vector<D3D12_INDEX_BUFFER_VIEW> m_vec_boundingBoxIndexBufferView;

		std::vector<ID3D12Resource*> m_vec_texture;
		std::vector<ComPtr<ID3D12Resource>> m_vec_textureUploadHeap;

		ComPtr<ID3D12Resource> m_shadowDepthMap;
		ComPtr<ID3D12Resource> m_NormalMap;
		ComPtr<ID3D12Resource> m_cameraDepthMap;
		ComPtr<ID3D12Resource> m_AmbientMap1;
		ComPtr<ID3D12Resource> m_AmbientMap2;
		ComPtr<ID3D12Resource> m_RandomVectorMap;
		ComPtr<ID3D12Resource> m_RandomVectorMapUploadBuffer;
		std::vector<std::string> m_vec_AssetFileTitle;
		std::vector<std::string> m_vec_TextureTitle;
		unsigned int m_numIndices;
		std::vector<unsigned int> m_vec_numIndices;
		std::vector<unsigned int> m_vec_boundingBoxNumIndices;
		std::vector<ComPtr<ID3D12Resource>> m_vec_matRes;
		std::vector<wxMaterial> m_vec_matStut;
		std::vector<ComPtr<ID3D12Resource>> m_vec_objConstRes;
		std::vector<wxObjConst> m_vec_objConstStut;
		std::vector<void*> m_pObjConstDataBegin;			//constant buffer data pointer
		D3D12_CONSTANT_BUFFER_VIEW_DESC m_cbvSunLight;
		std::vector<BoundingBox> m_vec_boundingBox;

		float angleAxisY;
		float angleAxisYPerSecond;
		float mSunAngleAxisY;
		double mSunAngleAxisYPerSec;
		// Synchronization objects.
		UINT m_frameIndex;
		HANDLE m_fenceEvent;
		ComPtr<ID3D12Fence> m_fence;
		UINT64 m_fenceValue;
		int m_textureSRVCount;
		int m_MaterialCount;
		int m_TypedDescriptorSize;
		int m_DepthStencilDescriptorSize;
		DWORD m_controllerState;

		void LoadPipeline();
		void LoadAssets();
		void CreatePipelineStateObject();
		void RetrievalAssetDirectory();
		void CreateTexture(std::vector<std::string>&);
		void ParserDataFromScene(std::vector<std::string>&);
		void LoadDataFromOGEX(std::vector<std::string>&);
		void CreateVertexBuffer(std::vector<D3D12_VERTEX_BUFFER_VIEW>& vec_VertexBufferView, Vertex&, size_t);
		void CreateIndexBuffer(std::vector<D3D12_INDEX_BUFFER_VIEW>& vec_IndexBufferView, int&, size_t);
		void CreateConstantMaterialBuffer(std::vector<wxMaterial>&);
		void CreateSunLightBuffer();
		void CreateObjConst(std::vector<wxObjConst>&);
		void CreateConstantMatrix();
		void CreateSSAOBuffer();
		void PopulateCommandList();
		void PopulateShadowMapCommandList();
		void PopulateNormalCommandList();
		void PopulateSSAOCommandList();
		void PopulateBlurSSAOCommandList(ComPtr<ID3D12Resource> resourcePtr, bool isHorizontal);
		void DrawBoundingBox();
		void WaitForPreviousFrame();
		void UpdateConstantBuffer(wxTimer* timer);
		void UpdateShadowMatrix(void);
		void CheckControllerInput(wxTimer* timer);
		int	GetSceneGeometryNodeCount();
		void GenerateShadowMap();
		void GenerateCameraDepthMap();
		void GenerateNormalMap();
		void GenerateAmbientMap();
		void GenerateRandomVectorTexture();
		void BuildOffsetVectors();
		void UpdateSunLight(wxTimer *timer);
		void UpdateSSAO(wxTimer *timer);
		void UpdateBlurWidget();
		std::vector<Vector4FT> CalcGaussWeights(float sigma);

		float m_cameraMoveBaseSpeed;// = 0.5f;
		float m_cameraRotationSpeed;// = 0.015f;
		Vector4FT m_defaultCameraPosition;// = {{ 0.f,2,50.f,1.f }};
		Vector4FT m_defaultLookAt;// = { 0,0,0.f,0.f };
		Vector4FT m_defaultUp;// = { 0.f,1.f,0.f,0.f };
		Vector4FT cameraDistance;// = { 0.f,0.f,0.f,1.f };
	};

}