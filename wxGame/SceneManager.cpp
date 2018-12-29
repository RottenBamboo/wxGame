#include "SceneManager.h"
#include "OpenGEXDecoder.h"
#include "FileLoader.h"
using namespace wxGame;

SceneManager::~SceneManager()
{
}

int SceneManager::Initialize()
{
	int result = 0;
	m_pScene = new Scene();
	return result;
}

void SceneManager::Finalize()
{
}

void SceneManager::Tick()
{
	if (m_bDirtyFlag)
	{
		m_bDirtyFlag = !(m_bRenderingQueued && m_bPhysicalSimulationQueued && m_bAnimationQueued);
	}
}

int SceneManager::LoadScene(const char* file_name)
{
	if (LoadOGEXScene(file_name))
	{
		//m_pScene->LoadBMPRes();
		m_bDirtyFlag = true;
		m_bRenderingQueued = false;
		m_bPhysicalSimulationQueued = false;
		return 0;
	}
	else
	{
		return -1;
	}
}

void SceneManager::ResetScene()
{
	m_bDirtyFlag = true;
}

bool SceneManager::LoadOGEXScene(const char* file_name)
{
	FileLoader fileLoader;
	FileLoader::FileInfo fileInfo;
	fileInfo = fileLoader.LoadBinary(file_name, false);
	if (fileInfo.fileLength > 0)
	{
		OpenGEXDecoder opengex_decoder;
		m_pScene = opengex_decoder.Decoder(fileInfo.fileItself);
	}

	DataBuffer dataBuffer((void*)fileInfo.fileItself, fileInfo.fileLength, 4);

	if (!m_pScene) 
	{
		return false;
	}
	return true;
}

const Scene& SceneManager::GetSceneForRendering()
{
	return *m_pScene;
}

const Scene& SceneManager::GetSceneForPhysicalSimulation()
{
	return *m_pScene;
}

bool SceneManager::IsSceneChanged()
{
	return m_bDirtyFlag;
}

void SceneManager::NotifySceneIsRenderingQueued()
{
	m_bRenderingQueued = true;
}

void SceneManager::NotifySceneIsPhysicalSimulationQueued()
{
	m_bPhysicalSimulationQueued = true;
}

void SceneManager::NotifySceneIsAnimationQueued()
{
	m_bAnimationQueued = true;
}

BaseSceneNode* SceneManager::GetRootNode()
{
	return m_pScene->SceneGraph;
}

SceneGeometryNode* SceneManager::GetSceneGeometryNode(std::string name)
{
	auto it = m_pScene->LUT_Name_GeometryNode.find(name);
	if (it != m_pScene->LUT_Name_GeometryNode.end())
		return it->second;
	else
		return new SceneGeometryNode();
}

SceneObjectGeometry* SceneManager::GetSceneGeometryObject(std::string key)
{
	return m_pScene->Geometries.find(key)->second;
}