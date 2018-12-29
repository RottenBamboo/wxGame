#include "Scene.h"

using namespace wxGame;

SceneObjectCamera* const Scene::GetCamera(const std::string& key) const
{
	std::unordered_map<std::string, SceneObjectCamera*>::const_iterator itr = Cameras.find(key);
	if (itr == Cameras.end())
		return nullptr;
	else
	{
		return itr->second;
	}
}

SceneCameraNode* const Scene::GetFirstCameraNode() const
{
	return(Cameras.empty() ? nullptr : CameraNodes.cbegin()->second);
}

SceneObjectLight* const Scene::GetLight(const std::string& key) const
{
	std::unordered_map<std::string, SceneObjectLight*>::const_iterator itr = Lights.find(key);
	if (itr == Lights.end())
		return nullptr;
	else
	{
		return itr->second;
	}
}

SceneLightNode* const Scene::GetFirstLightNode() const
{
	return (LightNodes.empty() ? nullptr : LightNodes.cbegin()->second);
}

SceneObjectMaterial* const Scene::GetMaterial(const std::string& key) const
{
	std::unordered_map<std::string, SceneObjectMaterial*>::const_iterator itr = Materials.find(key);
	if (itr == Materials.end())
		return m_pDefaultMaterial;
	else
	{
		return itr->second;
	}
}

SceneObjectMaterial* const Scene::GetFirstMaterial() const
{
	return (Materials.empty() ? nullptr : Materials.cbegin()->second);
}

SceneObjectGeometry* const Scene::GetGeometry(const std::string& key) const
{
	std::unordered_map < std::string, SceneObjectGeometry*>::const_iterator itr = Geometries.find(key);
	if (itr == Geometries.end())
		return nullptr;
	else
	{
		return itr->second;
	}
}

SceneGeometryNode* const Scene::GetFirstGeometryNode() const
{
	return (GeometryNodes.empty() ? nullptr : GeometryNodes.cbegin()->second);
}

void Scene::LoadBMPRes()
{
	std::unordered_map<std::string, SceneObjectMaterial*>::iterator itr = Materials.begin();
	for (; itr != Materials.end(); itr++)
	{
		SceneObjectMaterial* ptr = itr->second;
		if (ptr)
		{
			ptr->LoadTextures();
		}
	}
}
