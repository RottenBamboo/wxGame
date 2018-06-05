#pragma once
#define ENUM(temp) enum temp
#include <objbase.h>
namespace wxGame {
	enum wxSceneType
	{
		wxSceneObjectTypeMesh,
		wxSceneObjectTypeMaterial,
		wxSceneObjectTypeTexture,
		wxSceneObjectTypeLightOmni,
		wxSceneObjectTypeLightInfi,
		wxSceneObjectTypeLightSpot,
	};

	class BaseSceneObject
	{
	protected:
		GUID m_Guid;
		wxSceneType m_SceneType;
	protected:
		BaseSceneObject(GUID& guid, wxSceneType type) :m_Guid(guid), m_SceneType(type) {};
		BaseSceneObject(GUID&& guid, wxSceneType type) :m_Guid(guid), m_SceneType(type) {};
		BaseSceneObject(BaseSceneObject&& guid) : m_Guid() {};

	};
}