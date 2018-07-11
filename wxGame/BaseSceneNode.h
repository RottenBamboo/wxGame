#pragma once
#include <string>
#include <list>
#include "BaseTreeNode.h"
#include "SceneObject.h"
namespace wxGame
{
	class BaseSceneNode //: public TreeNode
	{
	protected:
		std::string m_strName;
		std::list<BaseSceneNode*> m_Children;
		std::list<SceneObjectTransform*> m_Transforms;
	protected:
		virtual void dump(std::ostream& out) const {};
	public:
		BaseSceneNode() {}
		BaseSceneNode(const char* name) { m_strName = name; }
		BaseSceneNode(const std::string& name) { m_strName = name; }
		BaseSceneNode(const std::string&& name) { m_strName = std::move(name); }
		virtual ~BaseSceneNode() {}
		void AppendChild(BaseSceneNode*&& baseNode)
		{
			m_Children.push_back(std::move(baseNode));
		}
		void AppendChild(SceneObjectTransform*&& tramsform)
		{
			m_Transforms.push_back(std::move(tramsform));
		}
		//std::vector<
	};

	template<typename T>
	class SceneNode:public BaseSceneNode
	{
	protected:
		T * m_pSceneObject;
	protected:
		virtual void dump(std::ostream& out) const
		{
			if (m_pSceneObject)
				out << *m_pSceneObject << std::endl;
		}
	public:
		using BaseSceneNode::BaseSceneNode;
		SceneNode() = default;
		SceneNode(const T*& object) {m_pSceneObject = object;}
		SceneNode(const T*&& object) { m_pSceneObject = std::move(object); }
		void AddSceneObjectRef(const T*& object) { m_pSceneObject = object; }
		void AddSceneObjectRef(const T*&& object) { m_pSceneObject = std::move(object); }
	};

	typedef BaseSceneNode SceneEmptyNode;
	class SceneGeometryNode :public SceneNode<SceneObjectGeometry>
	{
	protected:
		bool m_bVisible;
		bool m_bShadow;
		bool m_bMotionBlur;
		std::vector<SceneObjectMaterial*> m_Materials;
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
	};
}