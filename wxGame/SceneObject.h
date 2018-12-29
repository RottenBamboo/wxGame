#pragma once
#include "Mathmatic.h"
#include <string>
#include <vector>
#include "BaseSceneObject.h"
#include "BMPDecoder.h"
#include "Mesh.h"
#include "OpenGEX.h"
using namespace Mathmatic;
namespace wxGame
{
	enum VertexDataType
	{
		KVertexDataTypeFloat1 = 1,
		KVertexDataTypeFloat2,
		kVertexDataTypeFloat3,
		kVertexDataTypeFloat4,
		kVertexDataTypeDouble1,
		kVertexDataTypeDouble2,
		kVertexDataTypeDouble3,
		kVertexDataTypeDouble4,
		kVertexDataTypeMax
	};
	class SceneObjectVertexArray
	{
	protected:
		const std::string m_strAttribute;
		const uint32_t m_nMorphTargetIndex;
		void* m_pData;
		size_t m_szData;
		VertexDataType m_DataType;
	public:
		SceneObjectVertexArray(const char* attr, uint32_t morph_index = 0, VertexDataType data_type = VertexDataType::kVertexDataTypeFloat3, void* data = nullptr, size_t data_size = 0): m_strAttribute(attr), m_nMorphTargetIndex(morph_index), m_DataType(data_type), m_pData(data), m_szData(data_size) {}
		SceneObjectVertexArray(SceneObjectVertexArray& arr) = default;
		SceneObjectVertexArray(SceneObjectVertexArray&& arr) = default;
		
		size_t GetVertexCount() const
		{
			size_t size = m_szData;
			switch (m_DataType) {
			case VertexDataType::KVertexDataTypeFloat1:
				size /= 1;
				break;
			case VertexDataType::KVertexDataTypeFloat2:
				size /= 2;
				break;
			case VertexDataType::kVertexDataTypeFloat3:
				size /= 3;
				break;
			case VertexDataType::kVertexDataTypeFloat4:
				size /= 4;
				break;
			case VertexDataType::kVertexDataTypeDouble1:
				size /= 1;
				break;
			case VertexDataType::kVertexDataTypeDouble2:
				size /= 2;
				break;
			case VertexDataType::kVertexDataTypeDouble3:
				size /= 3;
				break;
			case VertexDataType::kVertexDataTypeDouble4:
				size /= 4;
				break;
			default:
				size = 0;
				break;

			}
			return size;
		}
		size_t GetElementCount() const { return m_szData; }
		size_t GetDataSize() const
		{
			size_t size = m_szData;
			switch (m_DataType) 
			{
			case VertexDataType::KVertexDataTypeFloat1:
			case VertexDataType::KVertexDataTypeFloat2:
			case VertexDataType::kVertexDataTypeFloat3:
			case VertexDataType::kVertexDataTypeFloat4:
				size *= sizeof(float);
				break;
			case VertexDataType::kVertexDataTypeDouble1:
			case VertexDataType::kVertexDataTypeDouble2:
			case VertexDataType::kVertexDataTypeDouble3:
			case VertexDataType::kVertexDataTypeDouble4:
				size *= sizeof(double);
				break;
			default:
				size = 0;
				assert(0);
				break;
			}
			return size;
		}
		const void* GetData() const { return m_pData; }
		VertexDataType GetDataType() const { return m_DataType; }
		const uint32_t GetMorphTargetIndex() const { return m_nMorphTargetIndex; }
		const std::string GetAttribute() const { return m_strAttribute; }
		void ParseVertexArray()
		{
			ODDL::DataStructure<FloatDataType> DSFDT;
			const char* temp_Data = (const char*)(m_pData);
			DSFDT.ParseData(temp_Data);
		}

		SceneObjectVertexArray Compound(const SceneObjectVertexArray& lhs, const SceneObjectVertexArray& rhs)
		{
			SceneObjectVertexArray temp();
			size_t RVertexCount = rhs.GetVertexCount();
			if (this->GetVertexCount() == RVertexCount)
			{
				const char* totalData = nullptr;
				for (int i = 0; i < RVertexCount; i++)
				{
					totalData = (const char*)(this->GetData());
					totalData[RVertexCount];
				}
			}
		}
		friend std::ostream& operator<<(std::ostream& out, const SceneObjectVertexArray& obj) {}
	};


	enum IndexDataType {
		kIndexDataTypeInt8,
		kIndexDataTypeInt16,
		kIndexDataTypeInt32,
		kIndexDataTypeInt64,
	};
	class SceneObjectIndexArray
	{
	protected:
		unsigned int m_nMaterialIndex;
		size_t m_szRestartIndex;
		IndexDataType m_DataType;
		const void* m_pData;
		const size_t m_szData;
	public:
		SceneObjectIndexArray(const unsigned int material_index = 0, const size_t restart_index = 0, const IndexDataType data_type = IndexDataType::kIndexDataTypeInt16, const void* data = nullptr, const size_t data_size = 0) :m_nMaterialIndex(material_index), m_szRestartIndex(restart_index), m_DataType(data_type), m_pData(data), m_szData(data_size) {}
		SceneObjectIndexArray(SceneObjectIndexArray& arr) = default;
		SceneObjectIndexArray(SceneObjectIndexArray&& arr) = default;
		size_t GetDataSize() const
		{
			size_t size = m_szData;
			switch (m_DataType)
			{
			case IndexDataType::kIndexDataTypeInt8:
				size *= sizeof(int8_t);
				break;
			case IndexDataType::kIndexDataTypeInt16:
				size *= sizeof(int16_t);
				break;
			case IndexDataType::kIndexDataTypeInt32:
				size *= sizeof(int32_t);
				break;
			case IndexDataType::kIndexDataTypeInt64:
				size *= sizeof(int64_t);
				break;
			default:
				size = 0;
				assert(0);
				break;
			}
			return size;
		}
		size_t GetIndexCount() const
		{
			return m_szData;
		}
		const void* GetData()const { return m_pData; }

		friend std::ostream& operator<<(std::ostream& out, const SceneObjectIndexArray& obj) {}
	};

	class SceneObjectMesh : public BaseSceneObject
	{
	protected:
		std::vector<SceneObjectIndexArray> m_IndexArray;
		std::vector<SceneObjectVertexArray> m_VertexArray;
		PrimitiveType m_PrimitiveType;
		bool m_bVisible = true;
		bool m_bShadow = false;
		bool m_bMotionBlur = false;
	public:
		SceneObjectMesh(bool visible = true, bool shadow = true, bool motion_blur = true) :BaseSceneObject(wxSceneType::wxSceneObjectTypeMesh), m_bVisible(visible), m_bShadow(shadow), m_bMotionBlur(motion_blur) {}
		SceneObjectMesh(SceneObjectMesh&& mesh) :BaseSceneObject(wxSceneType::wxSceneObjectTypeMesh), m_IndexArray(std::move(mesh.m_IndexArray))
			, m_VertexArray(std::move(mesh.m_VertexArray)), m_PrimitiveType(mesh.m_PrimitiveType), m_bVisible(mesh.m_bVisible), m_bShadow(mesh.m_bShadow), m_bMotionBlur(mesh.m_bMotionBlur) {}
		void AddIndexArray(SceneObjectIndexArray&& array) { m_IndexArray.push_back(std::move(array)); }
		void AddVertexArray(SceneObjectVertexArray&& array) { m_VertexArray.push_back(std::move(array)); }
		void SetPrimitiveType(PrimitiveType type) { m_PrimitiveType = type; }

		uint32_t GetVertexPropertiesCount() const { return static_cast<uint32_t>(m_VertexArray.size()); }
		size_t GetVertexCount() const { return (m_VertexArray.empty() ? 0 : m_VertexArray[0].GetVertexCount()); }
		const SceneObjectVertexArray& GetVertexPropertyArray(const size_t index) const { return m_VertexArray[index]; }

		size_t GetIndexGroupCount() const { return m_IndexArray.size(); }
		size_t GetIndexCount() const { return (m_IndexArray.empty()?0:m_IndexArray[0].GetIndexCount()); }
		const SceneObjectIndexArray& GetIndexArray(const size_t index) const { return m_IndexArray[index]; }

		const PrimitiveType& GetPrimitiveType() { return m_PrimitiveType; }

		friend std::ostream& operator << (std::ostream& out, const SceneObjectMesh& obj)
		{
			out << static_cast<const BaseSceneObject&>(obj) << std::endl;
			out << "Primitive Type:" << obj.m_PrimitiveType << std::endl;
			out << "This mesh contains 0x" << obj.m_VertexArray.size() << "vertex properties." << std::endl;
			for (size_t i = 0; i < obj.m_VertexArray.size(); i++)
			{
				//out << obj.m_VertexArray[i] << std::endl;
			}
			out << "This mesh contain 0x" << obj.m_IndexArray.size() << "index arrays." << std::endl;
			for (size_t i = 0; i < obj.m_IndexArray.size(); i++)
			{
				//out << obj.m_IndexArray[i] << std::endl;
			}
			out << "Visible:" << obj.m_bVisible << std::endl;
			out << "Shadow:" << obj.m_bShadow << std::endl;
			out << "Motion Blur:" << obj.m_bMotionBlur << std::endl;
			return out;
		}
	};

	class SceneObjectTexture :public BaseSceneObject
	{
	protected:
		uint32_t m_nTexCoordIndex;
		std::string m_Name;
		ImageCommon* m_pImage;
		std::vector<Matrix4X4FT> m_Transforms;

	public:
		SceneObjectTexture() :BaseSceneObject(wxSceneType::wxSceneObjectTypeTexture), m_nTexCoordIndex(8) {}
		SceneObjectTexture(std::string& name) :BaseSceneObject(wxSceneType::wxSceneObjectTypeTexture), m_nTexCoordIndex(8), m_Name(name) {}
		SceneObjectTexture(uint32_t coord_index, ImageCommon*& image) :BaseSceneObject(wxSceneType::wxSceneObjectTypeTexture), m_nTexCoordIndex(coord_index), m_pImage(image) {}
		SceneObjectTexture(uint32_t coord_index, ImageCommon*&& image) :BaseSceneObject(wxSceneType::wxSceneObjectTypeTexture), m_nTexCoordIndex(coord_index), m_pImage(std::move(image)) {}
		SceneObjectTexture(SceneObjectTexture&) = default;
		SceneObjectTexture(SceneObjectTexture&&) = default;
		void AddTransform(Matrix4X4FT& matrix) { m_Transforms.push_back(matrix); }
		void SetName(std::string& name) { m_Name = name; }
		void LoadTexture() {
			if (!m_pImage)
			{
				BMPDecoder bmpDecoder;
				DataBuffer dataBuffer;
				m_pImage = &bmpDecoder.BMPParser(dataBuffer);
			}
		}
		friend std::ostream& operator<< (std::ostream& out, const SceneObjectTexture& obj)
		{
			//out << static_cast<const BaseSceneObject<&>(obj) << std::endl;
			out << "Coord Index:" << obj.m_nTexCoordIndex << std::endl;
			out << "Name:" << obj.m_Name << std::endl;
			//if(obj.m_pImage<<std::endl;)
		}
	};
	template<typename T>
	struct ParameterMap
	{
		T Value;
		SceneObjectTexture* Map;
		ParameterMap() = default;
		ParameterMap(const T value) :Value(value) {};
		ParameterMap(const SceneObjectTexture*& value) :Map(value) {};
		ParameterMap(const ParameterMap& rhs) = default;
		ParameterMap(ParameterMap&& rhs) = default;
		ParameterMap& operator=(const ParameterMap& rhs) = default;
		ParameterMap& operator=(ParameterMap&& rhs) = default;
		ParameterMap& operator=(SceneObjectTexture* const & rhs)
		{
			Map = rhs;
			return *this;
		}
		~ParameterMap() = default;
		friend std::ostream& operator<<(std::ostream& out, const ParameterMap& obj)
		{
			out << "Parameter Valve:" << obj.Value << std::endl;
			if (obj.Map) {
				out << "Parameter Map:" << "obj.Map" << std::endl;
				return out;
			}
		}
	};

	typedef ParameterMap<Vector4FT> Color;
	typedef ParameterMap<Vector3FT> Normal;
	typedef ParameterMap<float>		Parameter;

	class SceneObjectMaterial : public BaseSceneObject
	{
	protected:
		std::string m_Name;
		Color m_BaseColor;
		Parameter m_metallic;
		Parameter m_Roughness;
		Normal m_Normal;
		Parameter m_Specular;
		Parameter m_AmbientOcclusion;
	public:
		SceneObjectMaterial(const std::string& name) :BaseSceneObject(wxSceneType::wxSceneObjectTypeMaterial), m_Name(name) {}
		SceneObjectMaterial(std::string&& name) :BaseSceneObject(wxSceneType::wxSceneObjectTypeMaterial), m_Name(std::move(name)) {}
		SceneObjectMaterial(void) :BaseSceneObject(wxSceneType::wxSceneObjectTypeMaterial), m_Name(""), m_BaseColor(Vector4FT(1.0f)), m_metallic(0.0f), m_Roughness(0.0f), m_Normal(Vector3FT({ 0.0f,0.0f,1.0f })), m_Specular(0.0f), m_AmbientOcclusion(1.0f) {}
		void SetName(const std::string& name) { m_Name = name; }
		void SetName(std::string&& name) { m_Name = std::move(name); }
		void SetColor(std::string& attrib, Vector4FT& color)
		{
			if (attrib == "diffuse")
			{
				m_BaseColor = Color(color);
			}
		}
		void SetParam(std::string& attrib, float param) {};
		void SetTexture(std::string & attrib, std::string& textureName)
		{
			if (attrib == "diffuse")
			{
				m_BaseColor = new SceneObjectTexture(textureName);
			}
		}
		void SetTexture(std::string& attrib, SceneObjectTexture*& texture)
		{
			if (attrib == "diffuse")
			{
				m_BaseColor = texture;
			}
		}

		void LoadTextures()
		{
			if (m_BaseColor.Map)
			{
				m_BaseColor.Map->LoadTexture();
			}
		}
	};

	class SceneObjectGeometry :public BaseSceneObject
	{
	protected:
		std::vector<SceneObjectMesh*> m_Mesh;
		bool m_bVisible;
		bool m_bShadow;
		bool m_bMotionBlur;
	public:
		SceneObjectGeometry() :BaseSceneObject(wxSceneType::wxSceneObjectTypeGeometry) {}
		void SetVisibility(bool visible) { m_bVisible = visible; }
		const bool Visible() { return m_bVisible; }
		void SetIfCastShadow(bool shadow) { m_bShadow = shadow; }
		const bool CastShadow() { return m_bShadow; }
		void SetIfMotionBlur(bool motion_blur) { m_bMotionBlur = motion_blur; }
		const bool MotionBlur() { return m_bMotionBlur; }
		void AddMesh(SceneObjectMesh*& mesh) { m_Mesh.push_back(std::move(mesh)); }
		SceneObjectMesh* const GetMesh() { return(m_Mesh.empty() ? nullptr : m_Mesh[0]);}
	};

	typedef	float(*AttenFunc)(float, float);

	class SceneObjectLight : public BaseSceneObject
	{
	protected:
		Color m_LightColor;
		float m_Intensity;
		AttenFunc m_LightAttenuation;
		float m_fNearClipDistance;
		float mfFarClipDistance;
		bool m_bCastShadows;
	protected:
		SceneObjectLight() :BaseSceneObject(wxSceneType::wxSceneObjectTypeLight) {};

	};

	class SceneObjectOmniLight :public SceneObjectLight
	{
	public:
		using SceneObjectLight::SceneObjectLight;
	};

	class SceneObjectSpotLight :public SceneObjectLight
	{
	protected:
		float m_fConeAngle;
		float m_fPenumBraAngle;
	public:
		using SceneObjectLight::SceneObjectLight;
	};

	class SceneObjectCamera :public BaseSceneObject
	{
	protected:
		float m_fAspect;
		float m_fNearClipDistance;
		float m_fFarClipDistance;
	public:
		SceneObjectCamera() :BaseSceneObject(wxSceneType::wxSceneObjectTypeCamera) {};
	};

	class SceneObjectOrthogonalCamera:public SceneObjectCamera
	{

	};

	class SceneObjectPerspectiveCamera :public SceneObjectCamera
	{
	protected:
		float m_fFov;
	};

	class SceneObjectTransform
	{
	protected:
		Matrix4X4FT m_matrix;
		bool m_bSceneObjectOnly;
	public:
		SceneObjectTransform() { MatrixIdentity(m_matrix); m_bSceneObjectOnly = false; }
		SceneObjectTransform(const Matrix4X4FT& matrix, const bool object_only = false) { m_matrix = matrix; m_bSceneObjectOnly = object_only; }
		const Matrix4X4FT* GetTransformMatrix() { return &m_matrix; }
		friend std::ostream& operator<<(std::ostream& out, const SceneObjectTransform& obj)
		{
			//out << "Transform Matrix:" << obj.m_matrix << std::endl;
			out << "Is Object Local:" << obj.m_bSceneObjectOnly << std::endl;

			return out;
		}
	};

	class SceneObjectTranslation :public SceneObjectTransform
	{
	public:
		SceneObjectTranslation(const Vector3FT& vec3)
		{
			MatrixTranslation(vec3);
		}
	};
}