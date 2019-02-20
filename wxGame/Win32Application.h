
#pragma once

#include "GraphicD3D12.h"

class GraphicD3D12;

class Win32Application
{
public:
	static int Run(GraphicD3D12* pSample, HINSTANCE hInstance, int nCmdShow);
	static HWND GetHwnd() { return m_hwnd; }

protected:
	static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

private:
	static HWND m_hwnd;
};
