// Fix Pipeline implementation, just a demo
// Reference: https://github.com/skywind3000/mini3d
#include <Windows.h>
#include <cstdio>
#include "Math/MLUtility.h"
#include <assert.h>
#pragma warning(disable:4996)

const int Width = 800;
const int Height = 600;
const float PI = 3.1415927f;

const int TRANSFORM_WORLD = 1;
const int TRANSFORM_VIEW = 2;
const int TRANSFORM_PROJECTION = 4;

const int FILL_WIREFRAME = 1;
const int FILL_COLOR = 2;

bool OpenConsoleDebug() {
	static bool open = false;
	if (!open) {
		if(open = AllocConsole())
			freopen("CONOUT$", "w", stdout);
	}
	return open;
}

struct FPVertex {
	// position
	float _x, _y, _z, _w;
	// normal
	float _nx, _ny, _nz;
	// color
	float _r, _g, _b;
	// constructor
	FPVertex() {}
	FPVertex(float x, float y, float z) {
		_x = x; _y = y; _z = z; _w = 1.0f;
	}
	FPVertex(float x, float y, float z, float r, float g, float b) {
		_x = x; _y = y; _z = z; _w = 1.0f;
		_r = r; _g = g; _b = b;
	}
};

// create device
struct Device {
	// window handle
	HWND _hwnd;
	// back buffer dc
	HDC _drawdc;
	// back buffer
	unsigned int *_backbuf;
	// depth buffer
	float **_zbuf;
	// width and height
	int _width, _height;
	// vertex buffer input
	FPVertex *_vb;
	// index buffer input
	int *_ib;
	// world matrix
	MLMatrix4 _world;
	// view matrix
	MLMatrix4 _view;
	// projection matrix
	MLMatrix4 _proj;
	// render state
	int _rstate;

	Device() {}

	Device(HWND hwnd, int width, int height) {
		_width = width;
		_height = height;
		_zbuf = new float *[_width];
		for (int i = 0; i < _width; i++) {
			_zbuf[i] = new float[_height];
		}
		_hwnd = hwnd;
		HDC hdc = GetDC(hwnd);
		_drawdc = CreateCompatibleDC(hdc);
		ReleaseDC(hwnd, hdc);
		BITMAPINFO bi = { { sizeof(BITMAPINFOHEADER), Width, -Height, 1, 32, BI_RGB,
			Width * Height * 4, 0, 0, 0, 0 } };
		HBITMAP hb = CreateDIBSection(_drawdc, &bi, DIB_RGB_COLORS, (void **)&_backbuf, 0, 0);
		SelectObject(_drawdc, hb);
	}

	void SetTransform(int state, const MLMatrix4 *m) {
		switch (state) {
		case TRANSFORM_WORLD:
			_world = *m;
			break;
		case TRANSFORM_VIEW:
			_view = *m;
			break;
		case TRANSFORM_PROJECTION:
			_proj = *m;
			break;
		}
	}
	
	void SetRenderState(int value) {
		_rstate = value;
	}

	void Clear(unsigned int color, float z) {
		for (int i = 0; i < _width; i++) {
			for (int j = 0; j < _height; j++) {
				_backbuf[j * _width + i] = color;
				_zbuf[i][j] = z;
			}
		}
	}

	void SetStreamSource(FPVertex *vb) {
		_vb = vb;
	}

	void SetIndices(int *ib) {
		_ib = ib;
	}

	// clip
	// after projection(in CVV)
	bool CheckCVV(const MLVector4 *v) {
		if (v->x < -v->w || v->x > v->w)
			return false;
		if (v->y < -v->w || v->y > v->w)
			return false;
		if (v->z < 0.0f || v->z > v->w)
			return false;
		return true;
	}

	// backface culling
	// after projection division
	bool Backface_Culling(const MLVector4 *p1, const MLVector4 *p2, const MLVector4 *p3) {
		// wireframe mode don't need backface culling
		if (_rstate == FILL_WIREFRAME)
			return true;
		// BE CARE OF FLOATING POINT ERROR!!!
		return (p1->y - p3->y) * (p2->x - p3->x) + (p2->y - p3->y) * (p3->x - p1->x) > EPSILON;
	}

	void BresenhamDrawLine(const MLVector4 *p1, const MLVector4 *p2) {
		int x1 = (int)p1->x, y1 = (int)p1->y, x2 = (int)p2->x, y2 = (int)p2->y;
		int dx = x2 - x1, dy = y2 - y1;
		int xstep = 1, ystep = 1;
		if (x1 > x2) {
			dx = -dx;
			xstep = -1;
		}
		if (y1 > y2) {
			dy = -dy;
			ystep = -1;
		}
		// if line is a point
		if (dx == 0 && dy == 0) {
			SetBackBuffer(x1, y1, 0x00000000);
			return;
		}
		// if line slope infinity
		if (dx == 0) {
			for (int y = y1; y != y2; y += ystep)
				SetBackBuffer(x1, y, 0x00000000);
			return;
		}
		// if line slope 0
		if (dy == 0) {
			for (int x = x1; x != x2; x += xstep)
				SetBackBuffer(x, y1, 0x00000000);
			return;
		}
		int dx2 = 2 * dx, dy2 = 2 * dy;
		// set x unit, step y
		if (dx > dy) {
			int error = dx - dy2;
			for (int x = x1, y = y1; x != x2; x += xstep) {
				SetBackBuffer(x, y, 0x00000000);
				if (error < 0) {
					error += dx2;
					y += ystep;
				}
				error -= dy2;
			}
		}
		// set y unit, step x
		else {
			int error = dy - dx2;
			for (int y = y1, x = x1; y != y2; y += ystep) {
				SetBackBuffer(x, y, 0x00000000);
				if (error < 0) {
					error += dy2;
					x += xstep;
				}
				error -= dx2;
			}
		}
	}

	void DrawScanLine(const FPVertex *left, const FPVertex *right) {
		int start = (int)roundf(left->_x);
		int end = (int)roundf(right->_x);
		int len = end - start + 1;
		FPVertex *step = new FPVertex;
		VertexDivision(step, left, right, right->_x - left->_x);
		FPVertex v = *left;
		for (int i = 0; i < len; i++) {
			float z = 1.0f / v._w;
			int x = (int)roundf(v._x);
			int y = (int)roundf(v._y);
			if (v._z < _zbuf[x][y]) {
				_zbuf[x][y] = v._z;
				int r = (int)(v._r * z * 255.0f);
				int g = (int)(v._g * z * 255.0f);
				int b = (int)(v._b * z * 255.0f);
				unsigned int color = (r << 16) | (g << 8) | b;
				SetBackBuffer(x, y, color);
			}
			VertexAdd(&v, step);
		}
	}

	void VertexInterpolation(FPVertex *vOut, const FPVertex *v1, const FPVertex *v2, float factor) {
		vOut->_x = LinearInterpolation(v1->_x, v2->_x, factor);
		vOut->_y = LinearInterpolation(v1->_y, v2->_y, factor);
		vOut->_z = LinearInterpolation(v1->_z, v2->_z, factor);
		vOut->_w = LinearInterpolation(v1->_w, v2->_w, factor);
		vOut->_r = LinearInterpolation(v1->_r, v2->_r, factor);
		vOut->_g = LinearInterpolation(v1->_g, v2->_g, factor);
		vOut->_b = LinearInterpolation(v1->_b, v2->_b, factor);
	}

	void VertexDivision(FPVertex *vOut, const FPVertex *v1, const FPVertex *v2, float factor) {
		float oneoverfactor = Float_Equals(factor, 0.0f) ? 0.0f : 1.0f / factor;
		vOut->_x = (v2->_x - v1->_x) * oneoverfactor;
		vOut->_y = (v2->_y - v1->_y) * oneoverfactor;
		vOut->_z = (v2->_z - v1->_z) * oneoverfactor;
		vOut->_w = (v2->_w - v1->_w) * oneoverfactor;
		vOut->_r = (v2->_r - v1->_r) * oneoverfactor;
		vOut->_g = (v2->_g - v1->_g) * oneoverfactor;
		vOut->_b = (v2->_b - v1->_b) * oneoverfactor;
	}

	void VertexAdd(FPVertex *vOut, FPVertex *step) {
		vOut->_x += step->_x;
		vOut->_y += step->_y;
		vOut->_z += step->_z;
		vOut->_w += step->_w;
		vOut->_r += step->_r;
		vOut->_g += step->_g;
		vOut->_b += step->_b;
	}

	// v1, v2 are in top and v1.x < v2.x
	void FillTopPrimitive(const FPVertex *v1, const FPVertex *v2, const FPVertex *v3) {
		int start = (int)roundf(v1->_y);
		int end = (int)roundf(v3->_y);
		int len = end - start + 1;
		FPVertex *stepLeft = new FPVertex;
		FPVertex *stepRight = new FPVertex;
		VertexDivision(stepLeft, v1, v3, v3->_y - v1->_y);
		VertexDivision(stepRight, v2, v3, v3->_y - v2->_y);
		FPVertex scanLeft = *v1;
		FPVertex scanRight = *v2;
		for (int i = 0; i < len; i++) {
			DrawScanLine(&scanLeft, &scanRight);
			VertexAdd(&scanLeft, stepLeft);
			VertexAdd(&scanRight, stepRight);
		}
	}

	// v2, v3 are in down and v2.x < v3.x
	void FillDownPrimitive(const FPVertex *v1, const FPVertex *v2, const FPVertex *v3) {
		int start = (int)roundf(v1->_y);
		int end = (int)roundf(v2->_y);
		int len = end - start + 1;
		FPVertex *stepLeft = new FPVertex;
		FPVertex *stepRight = new FPVertex;
		VertexDivision(stepLeft, v1, v2, v2->_y - v1->_y);
		VertexDivision(stepRight, v1, v3, v3->_y - v1->_y);
		FPVertex scanLeft = *v1;
		FPVertex scanRight = *v1;
		for (int i = 0; i < len; i++) {
			DrawScanLine(&scanLeft, &scanRight);
			VertexAdd(&scanLeft, stepLeft);
			VertexAdd(&scanRight, stepRight);
		}
	}

	void FillOnePrimitive(const FPVertex *v1, const FPVertex *v2, const FPVertex *v3) {
		if (Float_Equals(v1->_y, v2->_y)) {
			if (Float_Equals(v1->_x, v2->_x))
				return;
			else if (v1->_x < v2->_x)
				FillTopPrimitive(v1, v2, v3);
			else
				FillTopPrimitive(v2, v1, v3);
		}
		else if (Float_Equals(v2->_y, v3->_y)) {
			if (Float_Equals(v2->_x, v3->_x))
				return;
			else if (v2->_x < v3->_x)
				FillDownPrimitive(v1, v2, v3);
			else
				FillDownPrimitive(v1, v3, v2);
		}
		else {
			// interpolation
			float factor = (v2->_y - v1->_y) / (v3->_y - v1->_y);
			FPVertex *v = new FPVertex;
			VertexInterpolation(v, v1, v3, factor);
			if (Float_Equals(v->_x, v2->_x))
				return;
			else if (v->_x < v2->_x) {
				FillDownPrimitive(v1, v, v2);
				FillTopPrimitive(v, v2, v3);
			}
			else {
				FillDownPrimitive(v1, v2, v);
				FillTopPrimitive(v2, v, v3);
			}
		}
	}

	void DrawOnePrimitive(const FPVertex *v1, const FPVertex *v2, const FPVertex *v3) {
		MLVector4 p1, p2, p3;
		// transform to projection for cliping
		MLMatrix4 tran = _world * _view * _proj;
		Vec4_Transform(&p1, &MLVector4(v1->_x, v1->_y, v1->_z, v1->_w), &tran);
		Vec4_Transform(&p2, &MLVector4(v2->_x, v2->_y, v2->_z, v2->_w), &tran);
		Vec4_Transform(&p3, &MLVector4(v3->_x, v3->_y, v3->_z, v3->_w), &tran);
		if (!CheckCVV(&p1) || !CheckCVV(&p2) || !CheckCVV(&p3))
			return;
		// third projection division and viewport transformation for rasterization
		// remember to store real z first before division
		float z1 = p1.w;
		float z2 = p2.w;
		float z3 = p3.w;
		p1 /= p1.w; p2 /= p2.w; p3 /= p3.w;
		if (!Backface_Culling(&p1, &p2, &p3))
			return;
		MLMatrix4 _viewport;
		Matrix_Viewport(&_viewport, 0.0f, 0.0f, _width, _height);
		Vec4_Transform(&p1, &p1, &_viewport);
		Vec4_Transform(&p2, &p2, &_viewport);
		Vec4_Transform(&p3, &p3, &_viewport);

		if (_rstate == FILL_WIREFRAME) {
			// draw line
			BresenhamDrawLine(&p1, &p2);
			BresenhamDrawLine(&p2, &p3);
			BresenhamDrawLine(&p3, &p1);
			return;
		}
		if (_rstate == FILL_COLOR) {
			// fill primitive
			if (Float_Equals(p1.y, p2.y) && Float_Equals(p2.y, p3.y))
				return;
			if (Float_Equals(p1.x, p2.x) && Float_Equals(p2.x, p3.x))
				return;
			FPVertex r1(p1.x, p1.y, p1.z, v1->_r / z1, v1->_g / z1, v1->_b / z1);
			FPVertex r2(p2.x, p2.y, p2.z, v2->_r / z2, v2->_g / z2, v2->_b / z2);
			FPVertex r3(p3.x, p3.y, p3.z, v3->_r / z3, v3->_g / z3, v3->_b / z3);
			// remember to store real z
			r1._w = 1.0f / z1;
			r2._w = 1.0f / z2;
			r3._w = 1.0f / z3;
			// sort by y
			if (p1.y < p2.y) {
				if (p2.y < p3.y) {
					// p1 p2 p3
					FillOnePrimitive(&r1, &r2, &r3);
				}
				else if (p1.y < p3.y) {
					// p1 p3 p2
					FillOnePrimitive(&r1, &r3, &r2);
				}
				else {
					// p3 p1 p2
					FillOnePrimitive(&r3, &r1, &r2);
				}
			}
			else {
				if (p2.y > p3.y) {
					// p3 p2 p1
					FillOnePrimitive(&r3, &r2, &r1);
				}
				else if(p1.y > p3.y) {
					// p2 p3 p1
					FillOnePrimitive(&r2, &r3, &r1);
				}
				else {
					// p2 p1 p3
					FillOnePrimitive(&r2, &r1, &r3);
				}
			}
			return;
		}
	}

	void DrawIndexedPrimitive(int NumVertices, int TriCount) {
		// ready to draw
		for (int i = 0; i < TriCount; i++) {
			DrawOnePrimitive(&_vb[_ib[i * 3]], &_vb[_ib[i * 3 + 1]],
				&_vb[_ib[i * 3 + 2]]);
		}
	}

	void SetBackBuffer(int x, int y, unsigned int color) {
		if (x >= 0 && x < _width && y >= 0 && y < _height)
			_backbuf[y * _width + x] = color;
	}
	
	void Present() {
		HDC hDC = GetDC(_hwnd);
		BitBlt(hDC, 0, 0, _width, _height, _drawdc, 0, 0, SRCCOPY);
		ReleaseDC(_hwnd, hDC);
	}

};

Device *device;
// vertex buffer
FPVertex *vb;
// index buffer
int *ib;

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
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

HWND InitWindow(HINSTANCE hInstance, int width, int height, LPCWSTR className) {
	WNDCLASS wc;
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WndProc;
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

void InitCube(FPVertex *vb, int *ib) {
	vb[0] = FPVertex(-1.0f, 1.0f, -1.0f, 1.0f, 0.2f, 0.2f);
	vb[1] = FPVertex(1.0f, 1.0f, -1.0f, 0.2f, 1.0f, 0.2f);
	vb[2] = FPVertex(1.0f, -1.0f, -1.0f, 0.2f, 0.2f, 1.0f);
	vb[3] = FPVertex(-1.0f, -1.0f, -1.0f, 1.0f, 0.2f, 1.0f);
	vb[4] = FPVertex(-1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.2f);
	vb[5] = FPVertex(1.0f, 1.0f, 1.0f, 0.2f, 1.0f, 1.0f);
	vb[6] = FPVertex(1.0f, -1.0f, 1.0f, 1.0f, 0.3f, 0.3f);
	vb[7] = FPVertex(-1.0f, -1.0f, 1.0f, 0.2f, 1.0f, 0.3f);

	// front face
	ib[0] = 0; ib[1] = 1; ib[2] = 2;
	ib[3] = 0; ib[4] = 2; ib[5] = 3;
	// left face
	ib[6] = 4; ib[7] = 0; ib[8] = 3;
	ib[9] = 4; ib[10] = 3; ib[11] = 7;
	// up face
	ib[12] = 4; ib[13] = 5; ib[14] = 1;
	ib[15] = 4; ib[16] = 1; ib[17] = 0;
	// right face
	ib[18] = 1; ib[19] = 5; ib[20] = 6;
	ib[21] = 1; ib[22] = 6; ib[23] = 2;
	// back face
	ib[24] = 5; ib[25] = 4; ib[26] = 7;
	ib[27] = 5; ib[28] = 7; ib[29] = 6;
	// down face
	ib[30] = 6; ib[31] = 7; ib[32] = 3;
	ib[33] = 6; ib[34] = 3; ib[35] = 2;
}

bool Setup() {
	// create vertex buffer
	vb = new FPVertex[8];
	// create index buffer
	ib = new int[36];
	// fill vertex buffer and index buffer
	InitCube(vb, ib);
	// set view matrix
	MLVector3 pos(0.0f, 0.0f, -5.0f);
	MLVector3 target(0.0f, 0.0f, 0.0f);
	MLVector3 up(0.0f, 1.0f, 0.0f);
	MLMatrix4 V;
	Matrix_LookAt(&V, &pos, &target, &up);
	device->SetTransform(TRANSFORM_VIEW, &V);
	// set projection matrix
	MLMatrix4 proj;
	Matrix_PerspectiveFov(&proj, PI * 0.5f, (float)Width / (float)Height, 1.0f, 1000.0f);
	device->SetTransform(TRANSFORM_PROJECTION, &proj);
	// set render state
	device->SetRenderState(FILL_COLOR);
	return true;
}

bool Display(float timeDelta) {
	MLMatrix4 Rx, Ry;
	static float x = PI * 0.25f;
	Matrix_RotationX(&Rx, x);
	static float y = PI * 0.25f;
	Matrix_RotationY(&Ry, y);
	//y += timeDelta;
	//if (y >= PI * 2.0f)
	//	y = 0.0f;
	MLMatrix4 p = Rx * Ry;
	device->SetTransform(TRANSFORM_WORLD, &p);
	// clear back and depth buffer
	device->Clear(0xffffffff, 1.0f);
	// draw
	device->SetStreamSource(vb);
	device->SetIndices(ib);
	device->DrawIndexedPrimitive(8, 12);
	device->Present();
	return true;
}

int EnterMsgLoop(bool (*ptr_display)(float timeDelta)) {
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

int FixPipeline(HINSTANCE hinstance, HINSTANCE prevInstance, PSTR cmdLine, int showCmd) {
	HWND hwnd = InitWindow(hinstance, Width, Height, L"FixPipeline");
	device = new Device(hwnd, Width, Height);
	ShowWindow(hwnd, SW_SHOW);
	UpdateWindow(hwnd);
	Setup();
	EnterMsgLoop(Display);
	return 0;
}