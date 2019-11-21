#pragma once
#include "WinUser.h"
#include "wxGraphicD3D12.h"
#include <functional>

class DeviceInput
{
public:
	enum InputKeyEnum{
	};
	std::function<void(int, float)> InputFunction;

	DeviceInput()
	{
		Init();
	}

	void Init()
	{

	}

};