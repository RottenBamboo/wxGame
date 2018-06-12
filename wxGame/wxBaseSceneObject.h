#pragma once
#define ENUM(temp) enum temp
#include <objbase.h>
#include <utility>
namespace wxGame {
	enum wxSceneType
	{
		wxSceneObjectTypeInValid,
		wxSceneObjectTypeMesh,
		wxSceneObjectTypeMaterial,
		wxSceneObjectTypeTexture,
		wxSceneObjectTypeLightOmni,
		wxSceneObjectTypeLightDir,
		wxSceneObjectTypeLightSpot,
		wxSceneObjectTypeCamera,
		wxSceneObjectTypeAnimationClip,
		wxSceneObjectTypeClip,
		wxSceneObjectTypeVertexArray,
		wxSceneObjectTypeIndexArray,
		wxSceneObjectTypeGeometry,
		wxSceneObjectTypeTransform,
		wxSceneObjectTypeTranslate,
		wxSceneObjectTypeRotate,
		wxSceneObjectTypeScale,
		wxSceneObjectTypeTrack,
	};
	
	class BaseSceneObject
	{
	protected:
		GUID m_Guid;
		wxSceneType m_SceneType;
	protected:
		BaseSceneObject(GUID& guid, wxSceneType type) :m_Guid(guid), m_SceneType(type) {};
		BaseSceneObject(GUID&& guid, wxSceneType type) :m_Guid(std::move(guid)), m_SceneType(type) {};
		BaseSceneObject(BaseSceneObject&& obj) : m_Guid(std::move(obj.m_Guid)), m_SceneType(obj.m_SceneType) {};
		BaseSceneObject& operator=(BaseSceneObject&& obj) {
			this->m_Guid = std::move(obj.m_Guid); this->m_SceneType = obj.m_SceneType; \
				return *this;
		}
		virtual ~BaseSceneObject() {};
	private:
		BaseSceneObject() = delete;
		BaseSceneObject(BaseSceneObject& obj) = delete;
		BaseSceneObject& operator=(BaseSceneObject& obj) = delete;
	public:
		const GUID& GetGuid() const { return m_Guid; }
		const wxSceneType GetType() const { return m_SceneType; }
		 
		friend std::ostream& operator<<(std::ostream& out, const BaseSceneObject& obj)
		{
			out << "BaseSceneObject" << std::endl;
			out << "____________" << std::endl;
			out << "GUID" << obj.m_Guid << std::endl;
			out << "Type" << obj.m_SceneType << std::endl;
			return out;
		}

	};
}