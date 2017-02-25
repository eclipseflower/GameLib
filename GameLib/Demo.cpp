#include "D3D/D3DUtility.h"

IDirect3DDevice9 *Device = 0;
IDirect3DVertexBuffer9 *VB = 0;
IDirect3DIndexBuffer9 *IB = 0;
IDirect3DVertexBuffer9 *Triangle = 0;
IDirect3DVertexBuffer9 *Pyramid = 0;
ID3DXMesh *Objects[5] = { 0, 0, 0, 0, 0 };
ID3DXMesh *Teapot = 0;
D3DMATERIAL9 Mtrl[5];

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

struct LightVertex {
	LightVertex() {}
	LightVertex(float x, float y, float z, float nx, float ny, float nz) {
		_x = x; _y = y; _z = z; _nx = nx; _ny = ny; _nz = nz;
	}
	float _x, _y, _z;
	float _nx, _ny, _nz;
	static const DWORD FVF = D3DFVF_XYZ | D3DFVF_NORMAL;
};

// chapter 3
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

// chapter 4
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

// chapter 5
bool Pyramid_Setup() {
	Device->SetRenderState(D3DRS_LIGHTING, true);
	Device->CreateVertexBuffer(12 * sizeof(LightVertex), D3DUSAGE_WRITEONLY, LightVertex::FVF, 
		D3DPOOL_MANAGED, &Pyramid, 0);
	LightVertex *v;
	Pyramid->Lock(0, 0, (void **)&v, 0);
	v[0] = LightVertex(-1.0f, 0.0f, -1.0f, 0.0f, 0.707f, -0.707f);
	v[1] = LightVertex(0.0f, 1.0f, 0.0f, 0.0f, 0.707f, -0.707f);
	v[2] = LightVertex(1.0f, 0.0f, -1.0f, 0.0f, 0.707f, -0.707f);
	v[3] = LightVertex(-1.0f, 0.0f, 1.0f, -0.707f, 0.707f, 0.0f);
	v[4] = LightVertex(0.0f, 1.0f, 0.0f, -0.707f, 0.707f, 0.0f);
	v[5] = LightVertex(-1.0f, 0.0f, -1.0f, -0.707f, 0.707f, 0.0f);
	v[6] = LightVertex(1.0f, 0.0f, -1.0f, 0.707f, 0.707f, 0.0f);
	v[7] = LightVertex(0.0f, 1.0f, 0.0f, 0.707f, 0.707f, 0.0f);
	v[8] = LightVertex(1.0f, 0.0f, 1.0f, 0.707f, 0.707f, 0.0f);
	v[9] = LightVertex(1.0f, 0.0f, 1.0f, 0.0f, 0.707f, 0.707f);
	v[10] = LightVertex(0.0f, 1.0f, 0.0f, 0.0f, 0.707f, 0.707f);
	v[11] = LightVertex(-1.0f, 0.0f, 1.0f, 0.0f, 0.707f, 0.707f);
	Pyramid->Unlock();

	D3DMATERIAL9 mtrl;
	mtrl.Ambient = D3DLib::WHITE;
	mtrl.Diffuse = D3DLib::WHITE;
	mtrl.Specular = D3DLib::WHITE;
	mtrl.Emissive = D3DLib::BLACK;
	mtrl.Power = 5.0f;
	Device->SetMaterial(&mtrl);

	D3DLIGHT9 dir;
	ZeroMemory(&dir, sizeof(dir));
	dir.Type = D3DLIGHT_DIRECTIONAL;
	dir.Diffuse = D3DLib::WHITE;
	dir.Specular = D3DLib::WHITE * 0.3f;
	dir.Ambient = D3DLib::WHITE * 0.6f;
	dir.Direction = D3DXVECTOR3(1.0f, 0.0f, 0.0f);
	Device->SetLight(0, &dir);
	Device->LightEnable(0, true);
	Device->SetRenderState(D3DRS_NORMALIZENORMALS, true);
	Device->SetRenderState(D3DRS_SPECULARENABLE, true);

	D3DXVECTOR3 pos(0.0f, 1.0f, -3.0f);
	D3DXVECTOR3 target(0.0f, 0.0f, 0.0f);
	D3DXVECTOR3 up(0.0f, 1.0f, 0.0f);
	D3DXMATRIX V;
	D3DXMatrixLookAtLH(&V, &pos, &target, &up);
	Device->SetTransform(D3DTS_VIEW, &V);

	D3DXMATRIX proj;
	D3DXMatrixPerspectiveFovLH(&proj, D3DX_PI * 0.5f, (float)Width / (float)Height, 1.0f, 1000.0f);
	Device->SetTransform(D3DTS_PROJECTION, &proj);
	return true;
}

bool Pyramid_Display(float timeDelta) {
	D3DXMATRIX Ry;
	static float y = 0.0f;
	D3DXMatrixRotationY(&Ry, y);
	y += timeDelta;
	if (y > D3DX_PI * 2.0f)
		y = 0.0f;
	Device->SetTransform(D3DTS_WORLD, &Ry);

	Device->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0x00000000, 1.0f, 0);
	Device->BeginScene();
	Device->SetStreamSource(0, Pyramid, 0, sizeof(LightVertex));
	Device->SetFVF(LightVertex::FVF);
	Device->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 4);
	Device->EndScene();
	Device->Present(0, 0, 0, 0);
	return true;
}

bool Light_Setup(D3DLIGHTTYPE type) {
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

	Mtrl[0] = D3DLib::InitMaterial(D3DLib::RED, D3DLib::RED, D3DLib::RED, D3DLib::BLACK, 2.0f);
	Mtrl[1] = D3DLib::InitMaterial(D3DLib::BLUE, D3DLib::BLUE, D3DLib::BLUE, D3DLib::BLACK, 2.0f);
	Mtrl[2] = D3DLib::InitMaterial(D3DLib::GREEN, D3DLib::GREEN, D3DLib::GREEN, D3DLib::BLACK, 2.0f);
	Mtrl[3] = D3DLib::InitMaterial(D3DLib::YELLOW, D3DLib::YELLOW, D3DLib::YELLOW, D3DLib::BLACK, 2.0f);
	Mtrl[4] = D3DLib::InitMaterial(D3DLib::CYAN, D3DLib::CYAN, D3DLib::CYAN, D3DLib::CYAN, 2.0f);

	D3DLIGHT9 light;
	ZeroMemory(&light, sizeof(light));
	if (type == D3DLIGHT_DIRECTIONAL) {
		light.Type = D3DLIGHT_DIRECTIONAL;
		light.Ambient = D3DLib::WHITE * 0.6f;
		light.Diffuse = D3DLib::WHITE;
		light.Specular = D3DLib::WHITE * 0.6f;
		light.Direction = D3DXVECTOR3(1.0f, 0.0f, 0.25f);
	}
	else if (type == D3DLIGHT_POINT) {
		light.Type = D3DLIGHT_POINT;
		light.Ambient = D3DLib::WHITE * 0.6f;
		light.Diffuse = D3DLib::WHITE;
		light.Specular = D3DLib::WHITE * 0.6f;
		light.Position = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
		light.Range = 1000.0f;
		light.Falloff = 1.0f;
		light.Attenuation0 = 1.0f;
		light.Attenuation1 = 0.0f;
		light.Attenuation2 = 0.0f;
	}
	else if (type == D3DLIGHT_SPOT) {
		light.Type = D3DLIGHT_SPOT;
		light.Ambient = D3DLib::WHITE * 0.0f;
		light.Diffuse = D3DLib::WHITE;
		light.Specular = D3DLib::WHITE * 0.6f;
		light.Position = D3DXVECTOR3(0.0f, 0.0f, -5.0f);
		light.Direction = D3DXVECTOR3(0.0f, 0.0f, 1.0f);
		light.Range = 1000.0f;
		light.Falloff = 1.0f;
		light.Attenuation0 = 1.0f;
		light.Attenuation1 = 0.0f;
		light.Attenuation2 = 0.0f;
		light.Theta = 0.4f;
		light.Phi = 0.9f;
	}
	Device->SetLight(0, &light);
	Device->LightEnable(0, true);
	Device->SetRenderState(D3DRS_NORMALIZENORMALS, true);
	Device->SetRenderState(D3DRS_SPECULARENABLE, true);

	D3DXMATRIX proj;
	D3DXMatrixPerspectiveFovLH(&proj, D3DX_PI * 0.25f, (float)Width / (float)Height, 1.0f, 1000.0f);
	Device->SetTransform(D3DTS_PROJECTION, &proj);
	return true;
}

bool Light_Display(float timeDelta) {
	static float angle = (3.0f * D3DX_PI) / 2.0f;
	static float height = 5.0f;

	if (GetAsyncKeyState(VK_LEFT) & 0x8000f)
		angle -= 0.5f * timeDelta;

	if (GetAsyncKeyState(VK_RIGHT) & 0x8000f)
		angle += 0.5f * timeDelta;

	if (GetAsyncKeyState(VK_UP) & 0x8000f)
		height += 5.0f * timeDelta;

	if (GetAsyncKeyState(VK_DOWN) & 0x8000f)
		height -= 5.0f * timeDelta;

	D3DXVECTOR3 position(cosf(angle) * 7.0f, height, sinf(angle) * 7.0f);
	D3DXVECTOR3 target(0.0f, 0.0f, 0.0f);
	D3DXVECTOR3 up(0.0f, 1.0f, 0.0f);
	D3DXMATRIX V;
	D3DXMatrixLookAtLH(&V, &position, &target, &up);
	Device->SetTransform(D3DTS_VIEW, &V);

	Device->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0x00000000, 1.0f, 0);
	Device->BeginScene();

	for(int i = 0; i < 5; i++) {
		Device->SetMaterial(&Mtrl[i]);
		Device->SetTransform(D3DTS_WORLD, &ObjWorldMatrices[i]);
		Objects[i]->DrawSubset(0);
	}

	Device->EndScene();
	Device->Present(0, 0, 0, 0);
	return true;
}

//
// WinMain
//
/*
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

	Light_Setup(D3DLIGHT_SPOT);
	D3DLib::EnterMsgLoop(Light_Display);

	Device->Release();

	return 0;
}
*/