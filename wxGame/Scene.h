#pragma once
#include "SceneObject.h"
#include "SceneNode.h"
#include <string>
#include <unordered_map>

namespace wxGame {
	class Scene
	{
	private:
		SceneObjectMaterial* m_pDefaultMaterial;
	public:
		BaseSceneNode* SceneGraph;
		std::unordered_map<std::string, SceneObjectCamera*> Cameras;
		std::unordered_map<std::string, SceneObjectLight*> Lights;
		std::unordered_map<std::string, SceneObjectMaterial*> Materials;
		std::unordered_map < std::string, SceneObjectGeometry*> Geometries;

		std::unordered_multimap < std::string, SceneCameraNode*> CameraNodes;
		std::unordered_multimap<std::string, SceneLightNode*> LightNodes;
		std::unordered_multimap < std::string, SceneGeometryNode*> GeometryNodes;

		std::vector<BaseSceneNode*> AnimatableNodes;
		std::unordered_map<std::string, SceneGeometryNode*> LUT_Name_GeometryNode;
	public:
		Scene() {
			m_pDefaultMaterial = new SceneObjectMaterial("default");
		}
		Scene(const std::string& scene_name) :SceneGraph(new BaseSceneNode(scene_name)) {}
		~Scene() = default;

		SceneObjectCamera* const GetCamera(const std::string& key) const;
		SceneCameraNode* const GetFirstCameraNode() const;
		SceneObjectLight* const GetLight(const std::string& key) const;
		SceneLightNode* const GetFirstLightNode() const;
		SceneObjectGeometry* const GetGeometry(const std::string& key) const;
		SceneGeometryNode* const GetFirstGeometryNode() const;
		SceneObjectMaterial* const GetMaterial(const std::string& key) const;
		SceneObjectMaterial* const GetFirstMaterial() const;
		void LoadBMPRes(void);
	};
}

