#pragma once
#include "SceneManager.h"

namespace wxGame
{
	std::vector<SceneManager*> g_vecpSceneManager;
	template<typename T> class ParseText
	{
		T* m_data;
		size_t size;
		ParseText() :size = 0, m_data = nullptr; = default;

		ParseText(T* data)
		{
			m_data = data;
		}

		~ParseText()
		{
			if (m_data)
			{
				m_data = nullptr;
			}
		};
	};
}