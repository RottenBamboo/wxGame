#include "stdafx.h"
#include "WndStuff.h"

HWND WndStuff::m_hwnd = nullptr;
WndStuff::WndStuff()
{
}

WndStuff::~WndStuff()
{
}

int WndStuff::WndStart(GraphicD3D12* graphD3D, HINSTANCE hInstance, int nCmdShow)
{
	// Register class
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, IDI_APPLICATION);
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = nullptr;
	wcex.lpszClassName = L"wxGame";
	wcex.hIconSm = LoadIcon(hInstance, IDI_APPLICATION);
	assert(0 != RegisterClassEx(&wcex));
	
	
	RECT windowRect = { 0, 0, static_cast<long>(GetSystemMetrics(SM_CXSCREEN)), static_cast<long>(GetSystemMetrics(SM_CYSCREEN)) };
	bool suced = AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

	// Create the window and assign return value to a handle.
	m_hwnd = CreateWindow(
		wcex.lpszClassName,
		L"Game",
		m_windowStyle,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		windowRect.right - windowRect.left,
		windowRect.bottom - windowRect.top,
		nullptr,		// We have no parent window.
		nullptr,		// We aren't using menus.
		hInstance,
		graphD3D);

	// Initialize the D3D stuff.
	graphD3D->Initialize();

	ShowWindow(m_hwnd, SW_SHOWDEFAULT);

	MSG msg = {};
	do
	{
		while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		if (msg.message == WM_QUIT)
			break;
	}
	while (msg.message != WM_QUIT);	// Returns false to quit loop

	return 0;
}


LRESULT CALLBACK WndStuff::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	// find the pointer of wxGraphicD3D12 class object in the hWnd, reinterpret and assign it to a base class pointer.
	GraphicD3D12* gD3D12 = reinterpret_cast<GraphicD3D12*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
	switch (message)
	{
	case WM_CREATE:
	{
		// save the GraphicD3D12 creation structure
		LPCREATESTRUCT pGraphicCreateStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);
		SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<__int64>(pGraphicCreateStruct->lpCreateParams));
	}
	case WM_PAINT:
		if (gD3D12)
		{
			gD3D12->OnUpdate();
			gD3D12->OnRender();
			return 0;
		}

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}

	// Handle any messages the switch statement didn't.
	return DefWindowProc(hWnd, message, wParam, lParam);
}