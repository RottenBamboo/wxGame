#pragma once
#include <string>
namespace wxGame
{
	class SceneObjectVertex
	{
	protected:
		const std::string m_strAttribute;
		const int m_nMorphTargetIndex;
		//const VertexDataType m_DataType;
	public:
		SceneObjectVertex();
		~SceneObjectVertex();
	};
}
