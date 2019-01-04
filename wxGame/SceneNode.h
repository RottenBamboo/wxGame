#pragma once
#include <string>
#include <map>
#include <list>
#include <vector>
#include "BaseTreeNode.h"
#include "SceneObject.h"
#include "Mathmatic.h"
namespace wxGame
{
	class BaseSceneNode //: public TreeNode
	{
	protected:
		std::string m_strName;
		std::list<BaseSceneNode*> m_Children;
		std::vector<SceneObjectTransform*> m_Transforms;
		Matrix4X4FT m_RunTimeTransform;
	protected:
		virtual void dump(std::ostream& out) const {};
	public:
		const std::string GetName() const { return m_strName; }
		BaseSceneNode() {}
		BaseSceneNode(const char* name) { m_strName = name; }
		BaseSceneNode(const std::string& name) { m_strName = name; }
		BaseSceneNode(const std::string&& name) { m_strName = std::move(name); }
		virtual ~BaseSceneNode() {}
		void AppendChild(BaseSceneNode*&& baseNode)
		{
			m_Children.push_back(std::move(baseNode));
		}
		/*void AppendChild(SceneObjectTransform*&& tramsform)
		{
			m_Transforms.insert(std::move(tramsform));
		}*/
		void AppendTransform(SceneObjectTransform* const transform)
		{
			m_Transforms.push_back(transform);
		}
		SceneObjectTransform* GetTransform(size_t pos)
		{
			if (pos < m_Transforms.size())
			{
				return m_Transforms[pos];
			}
		}
	};

	template<typename T>
	class SceneNode:public BaseSceneNode
	{
	protected:
		std::string m_pSceneObject;
	protected:
		virtual void dump(std::ostream& out) const
		{
			out << m_pSceneObject << std::endl;
		}
	public:
		using BaseSceneNode::BaseSceneNode;
		SceneNode() = default;
		//SceneNode(const T*& object) {m_pSceneObject = object;}
		//SceneNode(const T*&& object) { m_pSceneObject = std::move(object); }
		//void AddSceneObjectRef(T*& const object) { m_pSceneObject = object; }
		//void AddSceneObjectRef(T*&& const object) { m_pSceneObject = std::move(object); }
		void AddSceneObjectRef(const std::string& object) { m_pSceneObject = object; }
		const std::string& GetSceneObjectRef() { return m_pSceneObject; }
	};

	typedef BaseSceneNode SceneEmptyNode;
	class SceneGeometryNode :public SceneNode<SceneObjectGeometry>
	{
	protected:
		bool m_bVisible;
		bool m_bShadow;
		bool m_bMotionBlur;
		std::vector<SceneObjectMaterial*> m_Materials;
		std::string m_GeometryObjs;
	protected:
		virtual void dump(std::ostream& out)
		{
			SceneNode::dump(out);
			out << "Visible:" << m_bVisible << std::endl;
			out << "Shadow:" << m_bShadow << std::endl;
			out << "Motion Blur:" << m_bMotionBlur << std::endl;
			out << "Material(s):" << std::endl;
			for (auto material : m_Materials) {
				out << *material << std::endl;
			}
		}
	public:
		using SceneNode::SceneNode;
		void SetVisibility(bool visible) { m_bVisible = visible; }
		const bool Visible() { return m_bVisible; }
		void SetIfCastShadow(bool shadow) { m_bShadow = shadow; }
		const bool CastShadow() { return m_bShadow; }
		void SetIfMotionBlur(bool motion_blur) { m_bMotionBlur = motion_blur; }
		const bool MotionBlur() { return m_bMotionBlur; }
		using SceneNode::AddSceneObjectRef;
		void AddSceneObjectRef(SceneObjectMaterial* const &object) { m_Materials.push_back(object); }
		void AddGeometryObjectRef(const char* object) { m_GeometryObjs = object; }
		std::string GetGeometryObjectRef() { return m_GeometryObjs; }
	};

	class SceneLightNode :public SceneNode<SceneObjectLight>
	{
	protected:
		Vector3FT m_Target;
	public:
		using SceneNode::SceneNode;
		void SetTarget(Vector3FT& target) { m_Target = target; }
		const Vector3FT& GetTarget() { return m_Target; }
	};

	class SceneCameraNode :public SceneNode<SceneObjectCamera>
	{
	protected:
		Vector3FT m_Target;
	public:
		using SceneNode::SceneNode;
		void SetTarget(Vector3FT& target) { m_Target = target; }
		const Vector3FT& GetTarget() { return m_Target; }
	};
}