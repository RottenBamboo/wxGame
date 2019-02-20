#pragma once
#include "stdafx.h"
#include "util.h"
#include "wxGraphicD3D12.h"

_Use_decl_annotations_
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
	wxGraphicD3D12 sample(2560, 1440, L"wxGame");
	return Win32Application::Run(&sample, hInstance, nCmdShow);
}
