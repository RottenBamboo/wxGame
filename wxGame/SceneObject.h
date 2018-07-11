#pragma once
#include "Mathmatic.h"
#include <string>
#include <vector>
#include "BaseSceneObject.h"
#include "BMPDecoder.h"
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
	class SceneObjectVertexArray : public BaseSceneObject
	{
	protected:
		const std::string m_strAttribute;
		const int m_nMorphTargetIndex;
		void* m_pData;
		size_t m_szData;
		VertexDataType m_DataType;
	public:
		SceneObjectVertexArray(const char* attr, void* data, size_t data_size, VertexDataType data_type, uint32_t morph_index = 0) :BaseSceneObject(wxSceneObjectTypeVertexArray), m_strAttribute(attr), m_nMorphTargetIndex(morph_index), m_pData(data), m_szData(data_size) {}

		//const VertexDataType m_DataType;
	public:
		SceneObjectVertexArray();
		~SceneObjectVertexArray();
	};


	enum IndexDataType {
		KIndexDataTypeInt16,
		KIndexDayaTypeInt32,
	};
	class SceneObjectIndexArray : public BaseSceneObject
	{
		unsigned int m_MaterialIndex;
		size_t m_RestartIndex;
		IndexDataType m_DataType;
		union {
			unsigned int* m_pSata16;
			unsigned int* m_pData32;
		};
	};

	class SceneObjectMesh : public BaseSceneObject
	{
	protected:
		std::vector<SceneObjectIndexArray> m_IndexArray;
		std::vector<SceneObjectVertexArray> m_VertexArray;
		bool m_bVisible = true;
		bool m_bShadow = false;
		bool m_bMotionBlur = false;
	public:
		SceneObjectMesh() :BaseSceneObject(wxSceneType::wxSceneObjectTypeMesh) {}
	};

	template<typename T>
	struct ParameterMap
	{
		bool bUsingSingValue = true;

		union parameterMap {
			T Value;
			ImageCommon* Map;
		};
	};

	typedef ParameterMap<Vector2FT> Color;
	typedef ParameterMap<Vector3FT> Normal;
	typedef ParameterMap<float>		Parameter;

	class SceneObjectMaterial : public BaseSceneObject
	{
	protected:
		Color m_BaseColor;
		Parameter m_metallic;
		Parameter m_Roughness;
		Normal m_Normal;
		Parameter m_Specular;
		Parameter m_AmbientOcclusion;
	public:
		SceneObjectMaterial() :BaseSceneObject(wxSceneType::wxSceneObjectTypeMaterial) {};
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