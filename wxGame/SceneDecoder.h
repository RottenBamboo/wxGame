#pragma once
#include<memory>
#include <string>
#include "Scene.h"

namespace wxGame {
	class SceneDecoder
	{
	public:
		virtual Scene* Decoder(const  std::string& buff) = 0;
	};
}