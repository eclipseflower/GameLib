#include "D3D/D3DUtility.h"

IDirect3DDevice9 *Device = 0;
IDirect3DVertexBuffer9 *VB = 0;
IDirect3DIndexBuffer9 *IB = 0;
IDirect3DVertexBuffer9 *Triangle = 0;
ID3DXMesh *Objects[5] = { 0, 0, 0, 0, 0 };
ID3DXMesh *Teapot = 0;

D3DXMATRIX ObjWorldMatrices[5];
D3DXMATRIX WorldMatrix;

const int Width = 800;
const int Height = 600;

struct Vertex {
	Vertex() {}
	Vertex(float x, float y, float z) {
		_x = x; _y = y; _z = z;
	}
	float _x, _y, _z;
	static const DWORD FVF = D3DFVF_XYZ;
};

struct ColorVertex {
	ColorVertex() {}
	ColorVertex(float x, float y, float z, D3DCOLOR c) {
		_x = x; _y = y; _z = z; _color = c;
	}
	float _x, _y, _z;
	D3DCOLOR _color;
	static const DWORD FVF = D3DFVF_XYZ | D3DFVF_DIFFUSE;
};

bool Cube_Setup() {
	Device->CreateVertexBuffer(8 * sizeof(Vertex), D3DUSAGE_WRITEONLY, Vertex::FVF, D3DPOOL_MANAGED,
		&VB, 0);
	Device->CreateIndexBuffer(36 * sizeof(WORD), D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_MANAGED,
		&IB, 0);

	Vertex *vertices;
	VB->Lock(0, 0, (void **)&vertices, 0);
	vertices[0] = Vertex(-1.0f, -1.0f, -1.0f);
	vertices[1] = Vertex(-1.0f, 1.0f, -1.0f);
	vertices[2] = Vertex(1.0f, 1.0f, -1.0f);
	vertices[3] = Vertex(1.0f, -1.0f, -1.0f);
	vertices[4] = Vertex(-1.0f, -1.0f, 1.0f);
	vertices[5] = Vertex(-1.0f, 1.0f, 1.0f);
	vertices[6] = Vertex(1.0f, 1.0f, 1.0f);
	vertices[7] = Vertex(1.0f, -1.0f, 1.0f);
	VB->Unlock();

	WORD *indices;
	IB->Lock(0, 0, (void **)&indices, 0);
	indices[0] = 0; indices[1] = 1; indices[2] = 2;
	indices[3] = 0; indices[4] = 2; indices[5] = 3;
	indices[6] = 4; indices[7] = 6; indices[8] = 5;
	indices[9] = 4; indices[10] = 7; indices[11] = 6;
	indices[12] = 4; indices[13] = 5; indices[14] = 1;
	indices[15] = 4; indices[16] = 1; indices[17] = 0;
	indices[18] = 3; indices[19] = 2; indices[20] = 6;
	indices[21] = 3; indices[22] = 6; indices[23] = 7;
	indices[24] = 1; indices[25] = 5; indices[26] = 6;
	indices[27] = 1; indices[28] = 6; indices[29] = 2;
	indices[30] = 4; indices[31] = 0; indices[32] = 3;
	indices[33] = 4; indices[34] = 3; indices[35] = 7;
	IB->Unlock();

	D3DXVECTOR3 position(0.0f, 0.0f, -5.0f);
	D3DXVECTOR3 target(0.0f, 0.0f, 0.0f);
	D3DXVECTOR3 up(0.0f, 1.0f, 0.0f);
	D3DXMATRIX V;
	D3DXMatrixLookAtLH(&V, &position, &target, &up);
	Device->SetTransform(D3DTS_VIEW, &V);

	D3DXMATRIX proj;
	D3DXMatrixPerspectiveFovLH(&proj, D3DX_PI * 0.5f, (float)Width / (float)Height, 1.0f, 1000.0f);
	Device->SetTransform(D3DTS_PROJECTION, &proj);

	Device->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
	return true;
}

bool Cube_Display(float timeDelta)
{
	if (Device) // Only use Device methods if we have a valid device.
	{
		D3DXMATRIX Rx, Ry;
		static float x = D3DX_PI * 0.25f;
		D3DXMatrixRotationX(&Rx, x);
		static float y = 0.0f;
		D3DXMatrixRotationY(&Ry, y);
		y += timeDelta;

		if (y >= D3DX_PI * 2.0f)
			y = 0.0f;

		D3DMATRIX p = Rx * Ry;
		Device->SetTransform(D3DTS_WORLD, &p);

		// Instruct the device to set each pixel on the back buffer black -
		// D3DCLEAR_TARGET: 0x00000000 (black) - and to set each pixel on
		// the depth buffer to a value of 1.0 - D3DCLEAR_ZBUFFER: 1.0f.
		Device->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0xffffffff, 1.0f, 0);

		Device->BeginScene();
		Device->SetStreamSource(0, VB, 0, sizeof(Vertex));
		Device->SetIndices(IB);
		Device->SetFVF(Vertex::FVF);
		Device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, 8, 0, 12);
		Device->EndScene();

		// Swap the back and front buffers.
		Device->Present(0, 0, 0, 0);
	}
	return true;
}

bool D3DXCreate_Setup() {
	D3DXCreateTeapot(Device, &Objects[0], 0);
	D3DXCreateBox(Device, 2.0f, 2.0f, 2.0f, &Objects[1], 0);
	D3DXCreateCylinder(Device, 1.0f, 1.0f, 3.0f, 10, 10, &Objects[2], 0);
	D3DXCreateTorus(Device, 1.0f, 3.0f, 10, 10, &Objects[3], 0);
	D3DXCreateSphere(Device, 1.0f, 10, 10, &Objects[4], 0);

	D3DXMatrixTranslation(&ObjWorldMatrices[0], 0.0f, 0.0f, 0.0f);
	D3DXMatrixTranslation(&ObjWorldMatrices[1], -5.0f, 0.0f, 5.0f);
	D3DXMatrixTranslation(&ObjWorldMatrices[2], 5.0f, 0.0f, 5.0f);
	D3DXMatrixTranslation(&ObjWorldMatrices[3], -5.0f, 0.0f, -5.0f);
	D3DXMatrixTranslation(&ObjWorldMatrices[4], 5.0f, 0.0f, -5.0f);

	D3DXMATRIX proj;
	D3DXMatrixPerspectiveFovLH(&proj, D3DX_PI * 0.5f, (float)Width / (float)Height, 1.0f, 1000.0f);
	Device->SetTransform(D3DTS_PROJECTION, &proj);

	Device->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
	return true;
}

bool D3DXCreate_Display(float timeDelta) {
	static float angle = (3.0f * D3DX_PI) / 2.0f;
	static float cameraHeight = 0.0f;
	static float cameraHeightDirection = 5.0f;

	D3DXVECTOR3 position(cosf(angle) * 10.0f, cameraHeight, sinf(angle) * 10.0f);
	D3DXVECTOR3 target(0.0f, 0.0f, 0.0f);
	D3DXVECTOR3 up(0.0f, 1.0f, 0.0f);
	D3DXMATRIX V;
	D3DXMatrixLookAtLH(&V, &position, &target, &up);
	Device->SetTransform(D3DTS_VIEW, &V);

	angle += timeDelta;
	if (angle > D3DX_PI * 2.0f)
		angle = 0.0f;
	cameraHeight += cameraHeightDirection * timeDelta;
	if (cameraHeight >= 10.0f || cameraHeight <= -10.0f)
		cameraHeightDirection = -cameraHeightDirection;

	Device->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0xffffffff, 1.0f, 0);
	Device->BeginScene();
	for (int i = 0; i < 5; i++) {
		Device->SetTransform(D3DTS_WORLD, &ObjWorldMatrices[i]);
		Objects[i]->DrawSubset(0);
	}
	Device->EndScene();
	Device->Present(0, 0, 0, 0);
	return true;
}

bool Teapot_Setup() {
	D3DXCreateTeapot(Device, &Teapot, 0);
	D3DXVECTOR3 position(0.0f, 0.0f, -3.0f);
	D3DXVECTOR3 target(0.0f, 0.0f, 0.0f);
	D3DXVECTOR3 up(0.0f, 1.0f, 0.0f);
	D3DXMATRIX V;
	D3DXMatrixLookAtLH(&V, &position, &target, &up);
	Device->SetTransform(D3DTS_VIEW, &V);

	D3DXMATRIX proj;
	D3DXMatrixPerspectiveFovLH(&proj, D3DX_PI * 0.5f, (float)Width / (float)Height, 1.0f, 1000.0f);
	Device->SetTransform(D3DTS_PROJECTION, &proj);

	Device->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
	return true;
}

bool Teapot_Display(float timeDelta) {
	D3DXMATRIX Ry;
	static float y = 0.0f;
	D3DXMatrixRotationY(&Ry, y);
	y += timeDelta;
	if (y >= D3DX_PI * 2.0f)
		y = 0.0f;
	Device->SetTransform(D3DTS_WORLD, &Ry);

	Device->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0xffffffff, 1.0f, 0);
	Device->BeginScene();
	Teapot->DrawSubset(0);
	Device->EndScene();
	Device->Present(0, 0, 0, 0);
	return true;
}

bool Triangle_Setup() {
	Device->CreateVertexBuffer(3 * sizeof(Vertex), D3DUSAGE_WRITEONLY, Vertex::FVF, D3DPOOL_MANAGED,
		&Triangle, 0);
	Vertex *vertices;
	Triangle->Lock(0, 0, (void **)&vertices, 0);
	vertices[0] = Vertex(-1.0f, 0.0f, 2.0f);
	vertices[1] = Vertex(0.0f, 1.0f, 2.0f);
	vertices[2] = Vertex(1.0f, 0.0f, 2.0f);
	Triangle->Unlock();

	D3DXMATRIX proj;
	D3DXMatrixPerspectiveFovLH(&proj, D3DX_PI * 0.5f, (float)Width / (float)Height, 1.0f, 1000.0f);
	Device->SetTransform(D3DTS_PROJECTION, &proj);

	Device->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
	return true;
}

bool Triangle_Display(float timeDelta) {
	Device->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0xffffffff, 1.0f, 0);
	Device->BeginScene();
	Device->SetStreamSource(0, Triangle, 0, sizeof(Vertex));
	Device->SetFVF(Vertex::FVF);
	Device->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 1);
	Device->EndScene();
	Device->Present(0, 0, 0, 0);
	return true;
}

bool Color_Setup() {
	Device->CreateVertexBuffer(3 * sizeof(ColorVertex), D3DUSAGE_WRITEONLY, ColorVertex::FVF,
		D3DPOOL_MANAGED, &Triangle, 0);
	ColorVertex *v;
	Triangle->Lock(0, 0, (void **)&v, 0);
	v[0] = ColorVertex(-1.0f, 0.0f, 2.0f, D3DCOLOR_XRGB(255, 0, 0));
	v[1] = ColorVertex(0.0f, 1.0f, 2.0f, D3DCOLOR_XRGB(0, 255, 0));
	v[2] = ColorVertex(1.0f, 0.0f, 2.0f, D3DCOLOR_XRGB(0, 0, 255));
	Triangle->Unlock();

	D3DXMATRIX proj;
	D3DXMatrixPerspectiveFovLH(&proj, D3DX_PI * 0.5f, (float)Width / (float)Height, 1.0f, 1000.0f);
	Device->SetTransform(D3DTS_PROJECTION, &proj);

	Device->SetRenderState(D3DRS_LIGHTING, false);
	return true;
}

bool Color_Display(float timeDelta) {
	Device->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0xffffffff, 1.0f, 0);
	Device->BeginScene();
	Device->SetFVF(ColorVertex::FVF);
	Device->SetStreamSource(0, Triangle, 0, sizeof(ColorVertex));
	D3DXMatrixTranslation(&WorldMatrix, -1.25f, 0.0f, 0.0f);
	Device->SetTransform(D3DTS_WORLD, &WorldMatrix);
	Device->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_FLAT);
	Device->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 1);
	D3DXMatrixTranslation(&WorldMatrix, 1.25f, 0.0f, 0.0f);
	Device->SetTransform(D3DTS_WORLD, &WorldMatrix);
	Device->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD);
	Device->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 1);
	Device->EndScene();
	Device->Present(0, 0, 0, 0);
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
	HWND hwnd = D3DLib::InitWindow(hinstance, Width, Height, L"Demo");
	if (!D3DLib::InitD3D(hinstance, hwnd,
		Width, Height, true, D3DDEVTYPE_HAL, &Device))
	{
		::MessageBox(0, L"InitD3D() - FAILED", 0, 0);
		return 0;
	}

	Color_Setup();
	D3DLib::EnterMsgLoop(Color_Display);

	Device->Release();

	return 0;
}