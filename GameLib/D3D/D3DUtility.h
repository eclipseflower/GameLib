#pragma once
#include <d3dx9.h>

namespace D3DLib {
	HWND InitWindow(HINSTANCE hInstance, int width, int height, LPCWSTR className);
	bool InitD3D(HINSTANCE hInstance, HWND hwnd, int width, int height, bool windowed,
		D3DDEVTYPE deviceType, IDirect3DDevice9 **device);
	int EnterMsgLoop(bool(*ptr_display)(float timeDelta));
	LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	D3DMATERIAL9 InitMaterial(D3DXCOLOR a, D3DXCOLOR d, D3DXCOLOR s, D3DXCOLOR e, float p);

	const D3DXCOLOR WHITE(D3DCOLOR_XRGB(255, 255, 255));
	const D3DXCOLOR BLACK(D3DCOLOR_XRGB(0, 0, 0));
	const D3DXCOLOR RED(D3DCOLOR_XRGB(255, 0, 0));
	const D3DXCOLOR GREEN(D3DCOLOR_XRGB(0, 255, 0));
	const D3DXCOLOR BLUE(D3DCOLOR_XRGB(0, 0, 255));
	const D3DXCOLOR YELLOW(D3DCOLOR_XRGB(255, 255, 0));
	const D3DXCOLOR CYAN(D3DCOLOR_XRGB(0, 255, 255));
	const D3DXCOLOR MAGENTA(D3DCOLOR_XRGB(255, 0, 255));
}