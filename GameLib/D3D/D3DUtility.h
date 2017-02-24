#pragma once
#include <d3dx9.h>

namespace D3DLib {
	HWND InitWindow(HINSTANCE hInstance, int width, int height, LPCWSTR className);
	bool InitD3D(HINSTANCE hInstance, HWND hwnd, int width, int height, bool windowed,
		D3DDEVTYPE deviceType, IDirect3DDevice9 **device);
	int EnterMsgLoop(bool(*ptr_display)(float timeDelta));
	LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

	const D3DXCOLOR WHITE(D3DCOLOR_XRGB(255, 255, 255));
	const D3DXCOLOR BLACK(D3DCOLOR_XRGB(0, 0, 0));
}