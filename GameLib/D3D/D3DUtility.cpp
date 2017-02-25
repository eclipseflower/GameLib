#include "D3DUtility.h"

// Create the main application window
HWND D3DLib::InitWindow(HINSTANCE hInstance, int width, int height, LPCWSTR className) {
	WNDCLASS wc;
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = D3DLib::WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(0, IDI_APPLICATION);
	wc.hCursor = LoadCursor(0, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.lpszMenuName = 0;
	wc.lpszClassName = className;

	if (!RegisterClass(&wc)) {
		MessageBox(0, L"RegisterClass() - FAILED", 0, 0);
		return 0;
	}

	HWND hwnd = 0;
	hwnd = CreateWindow(wc.lpszClassName, wc.lpszClassName, WS_OVERLAPPEDWINDOW, 0, 0, width, height, 
		0, 0, hInstance, 0);

	return hwnd;
}

// Init D3D
bool D3DLib::InitD3D(HINSTANCE hInstance, HWND hwnd, int width, int height, bool windowed, 
	D3DDEVTYPE deviceType, IDirect3DDevice9 **device) {
	if (!hwnd) {
		MessageBox(0, L"CreateWindow() - FAILED", 0, 0);
		return false;
	}

	ShowWindow(hwnd, SW_SHOW);
	UpdateWindow(hwnd);

	// Step 1: Create the IDirect3D9 object
	IDirect3D9 *d3d9 = 0;
	d3d9 = Direct3DCreate9(D3D_SDK_VERSION);

	if (!d3d9) {
		MessageBox(0, L"Direct3DCreate9() - FAILED", 0, 0);
		return false;
	}

	// Step 2: Check for hardware vp
	D3DCAPS9 caps;
	d3d9->GetDeviceCaps(D3DADAPTER_DEFAULT, deviceType, &caps);

	int vp = 0;
	if (caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT) {
		vp = D3DCREATE_HARDWARE_VERTEXPROCESSING;
	}
	else {
		vp = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
	}

	// Step 3: Fill out the D3DPRESENT_PARAMETERS structure
	D3DPRESENT_PARAMETERS d3dpp;
	d3dpp.BackBufferWidth = width;
	d3dpp.BackBufferHeight = height;
	d3dpp.BackBufferFormat = D3DFMT_A8R8G8B8;
	d3dpp.BackBufferCount = 1;
	d3dpp.MultiSampleType = D3DMULTISAMPLE_NONE;
	d3dpp.MultiSampleQuality = 0;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.hDeviceWindow = hwnd;
	d3dpp.Windowed = windowed;
	d3dpp.EnableAutoDepthStencil = true;
	d3dpp.AutoDepthStencilFormat = D3DFMT_D24S8;
	d3dpp.Flags = 0;
	d3dpp.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
	d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

	// Step 4: Create the device
	HRESULT hr = d3d9->CreateDevice(D3DADAPTER_DEFAULT, deviceType, hwnd, vp, &d3dpp, device);
	if (FAILED(hr)) {
		d3d9->Release();
		MessageBox(0, L"CreateDevice() - FAILED", 0, 0);
		return false;
	}
	d3d9->Release();
	return true;
}

int D3DLib::EnterMsgLoop(bool(*ptr_display)(float timeDelta)) {
	MSG msg;
	ZeroMemory(&msg, sizeof(MSG));

	static float lastTime = (float)timeGetTime();

	while (msg.message != WM_QUIT) {
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else {
			float currTime = (float)timeGetTime();
			float timeDelta = (currTime - lastTime) * 0.001f;

			ptr_display(timeDelta);

			lastTime = currTime;
		}
	}

	return msg.wParam;
}

LRESULT CALLBACK D3DLib::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE)
			DestroyWindow(hwnd);
		break;
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

// Init Material
D3DMATERIAL9 D3DLib::InitMaterial(D3DXCOLOR a, D3DXCOLOR d, D3DXCOLOR s, D3DXCOLOR e, float p) {
	D3DMATERIAL9 mtrl;
	mtrl.Ambient = a;
	mtrl.Diffuse = d;
	mtrl.Specular = s;
	mtrl.Emissive = e;
	mtrl.Power = p;
	return mtrl;
}