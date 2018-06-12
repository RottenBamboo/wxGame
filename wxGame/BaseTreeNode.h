#pragma once
#include "stdafx.h"
#include <list>

namespace wxGame {

	class TreeNode
	{
	public:
		virtual void AppendChild(TreeNode&& treeNode)
		{
			treeNode.m_Parent = this;
			m_Children.push_back(treeNode);
		}
	protected:
		virtual void dump(std::ostream& out) const {};
		TreeNode* m_Parent;
		std::list<TreeNode> m_Children;
	private:
	};
}