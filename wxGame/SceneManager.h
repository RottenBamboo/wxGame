#pragma once
#include "Scene.h"

namespace wxGame {
	class SceneManager
	{
	public:
		virtual ~SceneManager();

		virtual int Initialize();
		virtual void Finalize();

		virtual void Tick();

		int LoadScene(const char* scene_file_name);

		bool IsSceneChanged();
		void NotifySceneIsRenderingQueued();
		void NotifySceneIsPhysicalSimulationQueued();
		void NotifySceneIsAnimationQueued();

		const Scene& GetSceneForRendering();
		const Scene& GetSceneForPhysicalSimulation();

		void ResetScene();

		BaseSceneNode* GetRootNode();
		SceneGeometryNode* GetSceneGeometryNode(std::string name);
		SceneObjectGeometry* GetSceneGeometryObject(std::string key);

	protected:
		bool LoadOGEXScene(const char* file_name);
	protected:
		Scene* m_pScene;
		bool m_bRenderingQueued = false;
		bool m_bPhysicalSimulationQueued = false;
		bool m_bAnimationQueued = false;
		bool m_bDirtyFlag = false;
	};
	extern std::vector<SceneManager*> g_vecpSceneManager;
}

