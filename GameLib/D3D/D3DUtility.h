#pragma once
#include <d3dx9.h>

namespace D3DLib {
	bool InitWindow(HINSTANCE hInstance, int width, int height, LPCWSTR className);
	bool InitD3D(HINSTANCE hInstance, int width, int height, bool windowed, D3DDEVTYPE deviceType,
		IDirect3DDevice9 **device);
	int EnterMsgLoop(bool(*ptr_display)(float timeDelta));
	LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
}