#include "D3D/D3DUtility.h"

IDirect3DDevice9* Device = 0;

bool Display(float timeDelta)
{
	if (Device) // Only use Device methods if we have a valid device.
	{
		// Instruct the device to set each pixel on the back buffer black -
		// D3DCLEAR_TARGET: 0x00000000 (black) - and to set each pixel on
		// the depth buffer to a value of 1.0 - D3DCLEAR_ZBUFFER: 1.0f.
		Device->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0x00000000, 1.0f, 0);

		// Swap the back and front buffers.
		Device->Present(0, 0, 0, 0);
	}
	return true;
}

//
// WinMain
//
int WINAPI WinMain(HINSTANCE hinstance,
	HINSTANCE prevInstance,
	PSTR cmdLine,
	int showCmd)
{
	HWND hwnd = D3DLib::InitWindow(hinstance, 640, 480, L"Demo");
	if (!D3DLib::InitD3D(hinstance, hwnd,
		640, 480, true, D3DDEVTYPE_HAL, &Device))
	{
		::MessageBox(0, L"InitD3D() - FAILED", 0, 0);
		return 0;
	}

	D3DLib::EnterMsgLoop(Display);

	Device->Release();

	return 0;
}