// wxGame.cpp: 定义控制台应用程序的入口点。
//
#pragma once
#include "stdafx.h"
#include "wxGraphicD3D12.h"

int main()
{
	wxGraphicD3D12 gameSample;
	HINSTANCE gameHandle;
	gameHandle = GetModuleHandle(nullptr);
	WndStuff::WndStart(&gameSample, gameHandle, 0);
    return 0;
}

