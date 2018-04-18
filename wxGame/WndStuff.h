#pragma once
#include "stdafx.h"
#include "GraphicD3D12.h"

class WndStuff
{

public:
	WndStuff();
	~WndStuff();
public:
	static int WndStart(GraphicD3D12* graphD3D, HINSTANCE hInstance, int nCmdShow);
	static HWND GetHwnd() { return m_hwnd; }
	static HWND m_hwnd;
protected:
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
private:
	static bool m_fullscreenMode;
	static const UINT m_windowStyle = WS_OVERLAPPEDWINDOW;
	static RECT m_windowRect;

};