/****************************************************
* Fix Pipeline implementation, just a demo
* Reference: 
* https://github.com/skywind3000/mini3d
* https://github.com/zhangbaochong/Tiny3D
*/ 

#include <Windows.h>
#include <cstdio>
#include "Math/MLUtility.h"
#include <assert.h>
#include <gdiplus.h>
#pragma warning(disable:4996)

const int Width = 800;
const int Height = 600;
const float PI = 3.1415927f;

enum TRANSFORMTYPE
{
	TRANSFORM_WORLD = 1,
	TRANSFORM_VIEW = 2,
	TRANSFORM_PROJECTION = 4,
};

enum FILLTYPE {
	FILL_WIREFRAME = 1,
	FILL_COLOR = 2,
	FILL_TEXTURE = 4,
};

enum LIGHTTYPE {
	LIGHT_POINT = 1,
	LIGHT_SPOT = 2,
	LIGHT_DIRECTIONAL = 4,
};

enum SHADETYPE {
	SHADE_GOURAUD = 1,
	SHADE_PHONG = 2,
};

bool OpenConsoleDebug() {
	static bool open = false;
	if (!open) {
		if(open = AllocConsole())
			freopen("CONOUT$", "w", stdout);
	}
	return open;
}

struct Color {
	float _r, _g, _b;
	Color() {}
	Color(float r, float g, float b) {
		_r = r; _g = g; _b = b;
	}
	Color operator * (const Color &rhs) {
		Color c;
		c._r = this->_r * rhs._r;
		c._g = this->_g * rhs._g;
		c._b = this->_b * rhs._b;
		return c;
	}
	Color operator * (float rhs) {
		Color c;
		c._r = this->_r * rhs;
		c._g = this->_g * rhs;
		c._b = this->_b * rhs;
		return c;
	}
	Color operator + (const Color &rhs) {
		Color c;
		c._r = this->_r + rhs._r;
		c._g = this->_g + rhs._g;
		c._b = this->_b + rhs._b;
		return c;
	}
	unsigned int ToUINT() {
		int r = (int)(_r * 255.0f);
		int g = (int)(_g * 255.0f);
		int b = (int)(_b * 255.0f);
		r = max(0, min(r, 255));
		g = max(0, min(g, 255));
		b = max(0, min(b, 255));
		unsigned int color = (r << 16) | (g << 8) | b;
		assert(r >= 0 && g >= 0 && b >= 0);
		return color;
	}
};

struct FPVertex {
	// position
	float _x, _y, _z, _w;
	// color
	float _r, _g, _b;
	// normal
	float _nx, _ny, _nz;
	// texture
	float _u, _v;
	// light color
	Color _lightcolor;
	// position in view
	MLVector3 _vpos;
	// constructor
	FPVertex() {}
	// XYZ
	FPVertex(float x, float y, float z) {
		_x = x; _y = y; _z = z; _w = 1.0f;
	}
	// XYZ | COLOR
	FPVertex(float x, float y, float z, float r, float g, float b) {
		_x = x; _y = y; _z = z; _w = 1.0f;
		_r = r; _g = g; _b = b;
	}
	// XYZ | COLOR | NORMAL
	FPVertex(float x, float y, float z, float r, float g, float b, float nx, float ny, float nz) {
		_x = x; _y = y; _z = z; _w = 1.0f;
		_r = r; _g = g; _b = b;
		_nx = nx; _ny = ny; _nz = nz;
	}
	// XYZ | NORMAL | TEX
	FPVertex(float x, float y, float z, float nx, float ny, float nz, float u, float v) {
		_x = x; _y = y; _z = z; _w = 1.0f;
		_nx = nx; _ny = ny; _nz = nz;
		_u = u; _v = v;
	}
	// XYZ | COLOR | NORMAL | TEX
	FPVertex(float x, float y, float z, float r, float g, float b, float nx, float ny, float nz, 
		float u, float v) {
		_x = x; _y = y; _z = z; _w = 1.0f;
		_r = r; _g = g; _b = b;
		_nx = nx; _ny = ny; _nz = nz;
		_u = u; _v = v;
	}
};

struct Material {
	Color Diffuse;
	Color Ambient;
	Color Specular;
	Color Emissive;
	float Power;
};

struct Light {
	LIGHTTYPE Type;
	Color Diffiuse;
	Color Ambient;
	Color Specular;
	MLVector3 Position;
	MLVector3 Direction;
	float Range;
	float Falloff;
	float Attenuation0;
	float Attenuation1;
	float Attenuation2;
	float Theta;
	float Phi;
};

struct Texture {
	unsigned int _width, _height;
	Color **_pixelbuf;
	Texture(unsigned int width, unsigned int height) {
		_width = width; _height = height;
		_pixelbuf = new Color *[_width];
		for (unsigned int i = 0; i < _width; i++)
			_pixelbuf[i] = new Color[_height];
	}
	Texture(const Texture &tex) {
		_width = tex._width;
		_height = tex._height;
		_pixelbuf = new Color *[_width];
		for (unsigned int i = 0; i < _width; i++) {
			_pixelbuf[i] = new Color[_height];
			for (unsigned int j = 0; j < _height; j++)
				_pixelbuf[i][j] = tex._pixelbuf[i][j];
		}
	}
};

void CreateTextureFromFile(LPCWSTR filename, Texture *&tex) {
	Gdiplus::GdiplusStartupInput gdiplusstartupinput;
	ULONG_PTR gdiplustoken;
	GdiplusStartup(&gdiplustoken, &gdiplusstartupinput, nullptr);

	Gdiplus::Bitmap* bmp = new Gdiplus::Bitmap(filename);
	if (!bmp) {
		delete bmp;
		Gdiplus::GdiplusShutdown(gdiplustoken);
	}
	else {
		unsigned int width = bmp->GetWidth();
		unsigned int height = bmp->GetHeight();
		tex = new Texture(width, height);

		Gdiplus::Color color;
		for (unsigned int i = 0; i < width; i++) {
			for (unsigned int j = 0; j < height; j++) {
				bmp->GetPixel(i, j, &color);
				tex->_pixelbuf[i][height - 1 - j] = Color(color.GetRed() / 255.0f, 
					color.GetGreen() / 255.0f, color.GetBlue() / 255.0f);
			}
		}
		delete bmp;
		Gdiplus::GdiplusShutdown(gdiplustoken);
	}
}

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
	FILLTYPE _rstate;
	// shade mode
	SHADETYPE _shade;
	// material
	Material *_mtrl;
	// light
	Light *_light;
	// light status
	bool _lightenable;
	// texture
	Texture *_tex;

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

	void SetTransform(TRANSFORMTYPE type, const MLMatrix4 *m) {
		switch (type) {
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
	
	void SetRenderState(FILLTYPE value) {
		_rstate = value;
	}

	void SetShadeMode(SHADETYPE value) {
		_shade = value;
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

	void SetMaterial(Material *mtrl) {
		_mtrl = new Material(*mtrl);
	}

	void SetLight(Light *light) {
		_light = new Light(*light);
	}

	void SetTexture(Texture *tex) {
		_tex = new Texture(*tex);
	}

	void LightEnable(bool value) {
		_lightenable = value;
	}

	// return light direction normalized vector in view
	MLVector3 GetLightDirection(const MLVector4 *pV) {
		MLVector3 res;
		MLVector4 tran;
		switch (_light->Type) {
		case LIGHT_DIRECTIONAL:
			Vec4_Transform(&tran, &MLVector4(_light->Direction.x, _light->Direction.y,
				_light->Direction.z, 0.0f), &_view);
			Vec3_Normalize(&res, &MLVector3(tran.x, tran.y, tran.z));
			res = -res;
			break;
		case LIGHT_POINT:
		case LIGHT_SPOT:
			Vec4_Transform(&tran, &MLVector4(_light->Position.x, _light->Position.y,
				_light->Position.z, 0.0f), &_view);
			Vec3_Normalize(&res, &MLVector3(tran.x - pV->x, tran.y - pV->y, tran.z - pV->z));
			break;
		}
		return res;
	}

	// parameter: transformed normal and transformed vertex
	Color GetDiffuseColor(const MLVector4 *pN, const MLVector4 *pV) {
		MLVector3 normal;
		Vec3_Normalize(&normal, &MLVector3(pN->x, pN->y, pN->z));
		MLVector3 lightdir = GetLightDirection(pV);
		float cosine = max(0.0f, Vec3_Dot(&normal, &lightdir));
		Color diffuse = _mtrl->Diffuse * _light->Diffiuse * cosine;
		return diffuse;
	}

	// parameter: transformed normal and transformed vertex
	Color GetSpecularColor(const MLVector4 *pN, const MLVector4 *pV) {
		Color specular(0.0f, 0.0f, 0.0f);
		MLVector3 normal;
		Vec3_Normalize(&normal, &MLVector3(pN->x, pN->y, pN->z));
		MLVector3 lightdir = GetLightDirection(pV);
		if (Vec3_Dot(&normal, &lightdir) <= 0)
			return specular;
		MLVector3 view;
		Vec3_Normalize(&view, &MLVector3(pV->x, pV->y, pV->z));
		view = -view;
		MLVector3 half;
		Vec3_Normalize(&half, &(view + lightdir));
		float cosine = max(0.0f, Vec3_Dot(&normal, &half));
		specular = _mtrl->Specular * _light->Specular * powf(cosine, _mtrl->Power);
		return specular;
	}

	// get the final light color(emissive + amibent + diffuse + specular)
	// parameter: transformed normal and transformed vertex
	Color GetLightColor(const MLVector4 *pN, const MLVector4 *pV) {
		Color emissive = _mtrl->Emissive;
		Color amibent = _mtrl->Ambient * _light->Ambient;
		Color diffuse, specular, finalcolor;
		float attenuation = GetLightAttenuation(pV);
		float spotfactor = GetSpotFactor(pV);
		if (Float_Equals(attenuation, 0.0f) || Float_Equals(spotfactor, 0.0f)) {
			finalcolor = emissive + amibent;
		}
		else {
			diffuse = GetDiffuseColor(pN, pV);
			specular = GetSpecularColor(pN, pV);
			float factor = attenuation * spotfactor;
			finalcolor = emissive + amibent + (diffuse + specular) * factor;
		}
		return finalcolor;
	}

	float GetLightAttenuation(const MLVector4 *pV) {
		if (_light->Type == LIGHT_POINT || _light->Type == LIGHT_SPOT) {
			MLVector4 tran;
			Vec4_Transform(&tran, &MLVector4(_light->Position.x, _light->Position.y,
				_light->Position.z, 0.0f), &_view);
			float dis = Vec3_Length(&MLVector3(tran.x - pV->x, tran.y - pV->y, tran.z - pV->z));
			if (dis > _light->Range)
				return 0.0f;
			float attenuation = 1.0f / (_light->Attenuation0 + _light->Attenuation1 * dis +
				_light->Attenuation2 * dis * dis);
			return attenuation;
		}
		return 1.0f;
	}

	float GetSpotFactor(const MLVector4 *pV) {
		if (_light->Type == LIGHT_SPOT) {
			MLVector4 tran1, tran2;
			Vec4_Transform(&tran1, &MLVector4(_light->Direction.x, _light->Direction.y,
				_light->Direction.z, 0.0f), &_view);
			Vec4_Transform(&tran2, &MLVector4(_light->Position.x, _light->Position.y,
				_light->Position.z, 0.0f), &_view);
			MLVector3 dir1, dir2;
			Vec3_Normalize(&dir1, &MLVector3(tran1.x, tran1.y, tran1.z));
			Vec3_Normalize(&dir2, &MLVector3(pV->x - tran2.x, pV->y - tran2.y, pV->z - tran2.z));
			float cosine = Vec3_Dot(&dir1, &dir2);
			float costheta = cosf(_light->Theta * 0.5f);
			float cosphi = cosf(_light->Phi * 0.5f);
			if (cosine > costheta)
				return 1.0f;
			else if (cosine <= cosphi)
				return 0.0f;
			else {
				float base = (cosine - cosphi) / (costheta - cosphi);
				return powf(base, _light->Falloff);
			}
		}
		return 1.0f;
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

	void DrawScanLine(const FPVertex *left, const FPVertex *right , int yIndex) {
		int start = (int)ceilf(left->_x);
		int end = (int)ceilf(right->_x);
		FPVertex *step = new FPVertex;
		VertexDivision(step, left, right, right->_x - left->_x);
		FPVertex v = *left;
		for (int xIndex = start; xIndex < end; xIndex++) {
			assert(xIndex >= 0 && xIndex < _width);
			float z = 1.0f / v._w;
			if (v._z < _zbuf[xIndex][yIndex]) {
				_zbuf[xIndex][yIndex] = v._z;
				Color finalcolor;
				Color vertexcolor;
				if(_rstate == FILL_COLOR)
					vertexcolor = Color(v._r, v._g, v._b) * z;
				else if (_rstate == FILL_TEXTURE) {
					int x = (_tex->_width - 1) * v._u * z;
					int y = (_tex->_height - 1) * v._v * z;
					vertexcolor = _tex->_pixelbuf[x][y];
				}
				if (_lightenable) {
					Color lightcolor;
					if(_shade == SHADE_GOURAUD)
						lightcolor = v._lightcolor * z;
					else if (_shade == SHADE_PHONG) {
						MLVector4 fragN(v._nx * z, v._ny * z, v._nz * z, 0.0f);
						MLVector4 fragV(v._vpos.x * z, v._vpos.y * z, v._vpos.z * z, 1.0f);
						lightcolor = GetLightColor(&fragN, &fragV);
					}
					finalcolor = vertexcolor * lightcolor;
				}
				else
					finalcolor = vertexcolor;
				unsigned int color = finalcolor.ToUINT();
				SetBackBuffer(xIndex, yIndex, color);
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
		vOut->_nx = LinearInterpolation(v1->_nx, v2->_nx, factor);
		vOut->_ny = LinearInterpolation(v1->_ny, v2->_ny, factor);
		vOut->_nz = LinearInterpolation(v1->_nz, v2->_nz, factor);
		vOut->_u = LinearInterpolation(v1->_u, v2->_u, factor);
		vOut->_v = LinearInterpolation(v1->_v, v2->_v, factor);
		vOut->_lightcolor._r = LinearInterpolation(v1->_lightcolor._r, v2->_lightcolor._r, factor);
		vOut->_lightcolor._g = LinearInterpolation(v1->_lightcolor._g, v2->_lightcolor._g, factor);
		vOut->_lightcolor._b = LinearInterpolation(v1->_lightcolor._b, v2->_lightcolor._b, factor);
		vOut->_vpos.x = LinearInterpolation(v1->_vpos.x, v2->_vpos.x, factor);
		vOut->_vpos.y = LinearInterpolation(v1->_vpos.y, v2->_vpos.y, factor);
		vOut->_vpos.z = LinearInterpolation(v1->_vpos.z, v2->_vpos.z, factor);
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
		vOut->_nx = (v2->_nx - v1->_nx) * oneoverfactor;
		vOut->_ny = (v2->_ny - v1->_ny) * oneoverfactor;
		vOut->_nz = (v2->_nz - v1->_nz) * oneoverfactor;
		vOut->_u = (v2->_u - v1->_u) * oneoverfactor;
		vOut->_v = (v2->_v - v1->_v) * oneoverfactor;
		vOut->_lightcolor._r = (v2->_lightcolor._r - v1->_lightcolor._r) * oneoverfactor;
		vOut->_lightcolor._g = (v2->_lightcolor._g - v1->_lightcolor._g) * oneoverfactor;
		vOut->_lightcolor._b = (v2->_lightcolor._b - v1->_lightcolor._b) * oneoverfactor;
		vOut->_vpos.x = (v2->_vpos.x - v1->_vpos.x) * oneoverfactor;
		vOut->_vpos.y = (v2->_vpos.y - v1->_vpos.y) * oneoverfactor;
		vOut->_vpos.z = (v2->_vpos.z - v1->_vpos.z) * oneoverfactor;
	}

	void VertexAdd(FPVertex *vOut, FPVertex *step) {
		vOut->_x += step->_x;
		vOut->_y += step->_y;
		vOut->_z += step->_z;
		vOut->_w += step->_w;
		vOut->_r += step->_r;
		vOut->_g += step->_g;
		vOut->_b += step->_b;
		vOut->_u += step->_u;
		vOut->_v += step->_v;
		vOut->_nx += step->_nx;
		vOut->_ny += step->_ny;
		vOut->_nz += step->_nz;
		vOut->_lightcolor._r += step->_lightcolor._r;
		vOut->_lightcolor._g += step->_lightcolor._g;
		vOut->_lightcolor._b += step->_lightcolor._b;
		vOut->_vpos.x += step->_vpos.x;
		vOut->_vpos.y += step->_vpos.y;
		vOut->_vpos.z += step->_vpos.z;
	}

	/**********************************************************************************
		Here, In FillTopPrimitive and FillDownPrimitive function, we didn't use vertex_add to 
		interpolation. Why? Because the floating point add error!
		In my debug history, I found that the error could cause value become 0.999 while the real
		value is 1.001. Thus, when we use ceilf function, the integer value will be 1 NOT 2! So it
		will cause a white plot in some circumstance.
		LOOK OUT FOR FLOATING POINT ADD ERROR!
	**/

	// v1, v2 are in top and v1.x < v2.x
	void FillTopPrimitive(const FPVertex *v1, const FPVertex *v2, const FPVertex *v3) {
		int start = (int)ceilf(v1->_y);
		int end = (int)ceilf(v3->_y);
		FPVertex *stepLeft = new FPVertex;
		FPVertex *stepRight = new FPVertex;
		VertexDivision(stepLeft, v1, v3, v3->_y - v1->_y);
		VertexDivision(stepRight, v2, v3, v3->_y - v2->_y);
		FPVertex *scanLeft = new FPVertex;
		FPVertex *scanRight = new FPVertex;
		//FPVertex *tscanLeft = new FPVertex;
		//FPVertex *tscanRight = new FPVertex;
		//VertexInterpolation(tscanLeft, v1, v3, (start - v1->_y) / (v3->_y - v1->_y));
		//VertexInterpolation(tscanRight, v2, v3, (start - v2->_y) / (v3->_y - v2->_y));
		for (int yIndex = start; yIndex < end; yIndex++) {
			VertexInterpolation(scanLeft, v1, v3, (yIndex - v1->_y) / (v3->_y - v1->_y));
			VertexInterpolation(scanRight, v2, v3, (yIndex - v2->_y) / (v3->_y - v2->_y));
			DrawScanLine(scanLeft, scanRight, yIndex);
			//VertexAdd(tscanLeft, stepLeft);
			//VertexAdd(tscanRight, stepRight);
		}
	}

	// v2, v3 are in down and v2.x < v3.x
	void FillDownPrimitive(const FPVertex *v1, const FPVertex *v2, const FPVertex *v3) {
		int start = (int)ceilf(v1->_y);
		int end = (int)ceilf(v2->_y);
		FPVertex *stepLeft = new FPVertex;
		FPVertex *stepRight = new FPVertex;
		VertexDivision(stepLeft, v1, v2, v2->_y - v1->_y);
		VertexDivision(stepRight, v1, v3, v3->_y - v1->_y);
		FPVertex *scanLeft = new FPVertex;
		FPVertex *scanRight = new FPVertex;
		//FPVertex *tscanLeft = new FPVertex;
		//FPVertex *tscanRight = new FPVertex;
		//VertexInterpolation(tscanLeft, v1, v2, (start - v1->_y) / (v2->_y - v1->_y));
		//VertexInterpolation(tscanRight, v1, v3, (start - v1->_y) / (v3->_y - v1->_y));
		for (int yIndex = start; yIndex < end; yIndex++) {
			VertexInterpolation(scanLeft, v1, v2, (yIndex - v1->_y) / (v2->_y - v1->_y));
			VertexInterpolation(scanRight, v1, v3, (yIndex - v1->_y) / (v3->_y - v1->_y));
			DrawScanLine(scanLeft, scanRight, yIndex);
			//VertexAdd(tscanLeft, stepLeft);
			//VertexAdd(tscanRight, stepRight);
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
		// if enable light, calculate vertex light color in view as view vector can be easy
		Color lightcolor1, lightcolor2, lightcolor3;
		MLVector4 vp1, vp2, vp3;
		MLVector4 n1, n2, n3;
		if (_lightenable) {
			MLMatrix4 tran = _world * _view;
			Vec4_Transform(&vp1, &MLVector4(v1->_x, v1->_y, v1->_z, v1->_w), &tran);
			Vec4_Transform(&vp2, &MLVector4(v2->_x, v2->_y, v2->_z, v2->_w), &tran);
			Vec4_Transform(&vp3, &MLVector4(v3->_x, v3->_y, v3->_z, v3->_w), &tran);
			// normal transformation
			MLMatrix4 ttran, ntran;
			Matrix_Transpose(&ttran, &tran);
			Matrix_Inverse(&ntran, &ttran);
			Vec4_Transform(&n1, &MLVector4(v1->_nx, v1->_ny, v1->_nz, 0.0f), &ntran);
			Vec4_Transform(&n2, &MLVector4(v2->_nx, v2->_ny, v2->_nz, 0.0f), &ntran);
			Vec4_Transform(&n3, &MLVector4(v3->_nx, v3->_ny, v3->_nz, 0.0f), &ntran);
			// calculate lighting
			if (_shade == SHADE_GOURAUD) {
				lightcolor1 = GetLightColor(&n1, &vp1);
				lightcolor2 = GetLightColor(&n2, &vp2);
				lightcolor3 = GetLightColor(&n3, &vp3);
			}
		}
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
		if (_rstate == FILL_COLOR || _rstate == FILL_TEXTURE) {
			// fill primitive
			if (Float_Equals(p1.y, p2.y) && Float_Equals(p2.y, p3.y))
				return;
			if (Float_Equals(p1.x, p2.x) && Float_Equals(p2.x, p3.x))
				return;
			FPVertex r1(p1.x, p1.y, p1.z, v1->_r / z1, v1->_g / z1, v1->_b / z1, n1.x / z1, n1.y / z1,
				n1.z / z1, v1->_u / z1, v1->_v / z1);
			FPVertex r2(p2.x, p2.y, p2.z, v2->_r / z2, v2->_g / z2, v2->_b / z2, n2.x / z2, n2.y / z2,
				n2.z / z2, v2->_u / z2, v2->_v / z2);
			FPVertex r3(p3.x, p3.y, p3.z, v3->_r / z3, v3->_g / z3, v3->_b / z3, n3.x / z3, n3.y / z3,
				n3.z / z3, v3->_u / z3, v3->_v / z3);
			// remember to store real z
			r1._w = 1.0f / z1;
			r2._w = 1.0f / z2;
			r3._w = 1.0f / z3;
			// remember to store light color / z or view xyz / z if light enable
			if (_lightenable) {
				if (_shade == SHADE_GOURAUD) {
					r1._lightcolor = lightcolor1 * r1._w;
					r2._lightcolor = lightcolor2 * r2._w;
					r3._lightcolor = lightcolor3 * r3._w;
				}
				else if (_shade == SHADE_PHONG) {
					r1._vpos = MLVector3(vp1.x, vp1.y, vp1.z) * r1._w;
					r2._vpos = MLVector3(vp2.x, vp2.y, vp2.z) * r2._w;
					r3._vpos = MLVector3(vp3.x, vp3.y, vp3.z) * r3._w;
				}
			}
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

	void DrawPrimitive(int startIndex, int TriCount) {
		// ready to draw
		for (int i = 0; i < TriCount; i++) {
			DrawOnePrimitive(&_vb[startIndex + i * 3], &_vb[startIndex + i * 3 + 1],
				&_vb[startIndex + i * 3 + 2]);
		}
	}

	void DrawIndexedPrimitive(int startIndex, int TriCount) {
		// ready to draw
		for (int i = 0; i < TriCount; i++) {
			DrawOnePrimitive(&_vb[_ib[startIndex + i * 3]], &_vb[_ib[startIndex + i * 3 + 1]],
				&_vb[_ib[startIndex + i * 3 + 2]]);
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

void InitPyramid(FPVertex *vb) {
	vb[0] = FPVertex(-1.0f, 0.0f, -1.0f, 1.0f, 0.2f, 0.2f, 0.0f, 0.707f, -0.707f);
	vb[1] = FPVertex(0.0f, 1.0f, 0.0f, 0.2f, 1.0f, 0.2f, 0.0f, 0.707f, -0.707f);
	vb[2] = FPVertex(1.0f, 0.0f, -1.0f, 0.2f, 0.2f, 1.0f, 0.0f, 0.707f, -0.707f);
	vb[3] = FPVertex(-1.0f, 0.0f, 1.0f, 1.0f, 0.2f, 1.0f, -0.707f, 0.707f, 0.0f);
	vb[4] = FPVertex(0.0f, 1.0f, 0.0f, 0.2f, 1.0f, 0.2f, -0.707f, 0.707f, 0.0f);
	vb[5] = FPVertex(-1.0f, 0.0f, -1.0f, 1.0f, 0.2f, 0.2f, -0.707f, 0.707f, 0.0f);
	vb[6] = FPVertex(1.0f, 0.0f, -1.0f, 0.2f, 0.2f, 1.0f, 0.707f, 0.707f, 0.0f);
	vb[7] = FPVertex(0.0f, 1.0f, 0.0f, 0.2f, 1.0f, 0.2f, 0.707f, 0.707f, 0.0f);
	vb[8] = FPVertex(1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.2f, 0.707f, 0.707f, 0.0f);
	vb[9] = FPVertex(1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.2f, 0.0f, 0.707f, 0.707f);
	vb[10] = FPVertex(0.0f, 1.0f, 0.0f, 0.2f, 1.0f, 0.2f, 0.0f, 0.707f, 0.707f);
	vb[11] = FPVertex(-1.0f, 0.0f, 1.0f, 1.0f, 0.2f, 1.0f, 0.0f, 0.707f, 0.707f);
}

void InitTexCube(FPVertex *vb, int *ib) {
	vb[0] = FPVertex(-1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f);
	vb[1] = FPVertex(-1.0f, 1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f);
	vb[2] = FPVertex(1.0f, 1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f);
	vb[3] = FPVertex(1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f);

	vb[4] = FPVertex(-1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f);
	vb[5] = FPVertex(1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f);
	vb[6] = FPVertex(1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
	vb[7] = FPVertex(-1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f);

	vb[8] = FPVertex(-1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f);
	vb[9] = FPVertex(-1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f);
	vb[10] = FPVertex(1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f);
	vb[11] = FPVertex(1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f);

	vb[12] = FPVertex(-1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f);
	vb[13] = FPVertex(1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f);
	vb[14] = FPVertex(1.0f, -1.0f, 1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f);
	vb[15] = FPVertex(-1.0f, -1.0f, 1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f);

	vb[16] = FPVertex(-1.0f, -1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	vb[17] = FPVertex(-1.0f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
	vb[18] = FPVertex(-1.0f, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f);
	vb[19] = FPVertex(-1.0f, -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f);

	vb[20] = FPVertex(1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	vb[21] = FPVertex(1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
	vb[22] = FPVertex(1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f);
	vb[23] = FPVertex(1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f);

	ib[0] = 0; ib[1] = 1; ib[2] = 2;
	ib[3] = 0; ib[4] = 2; ib[5] = 3;
	ib[6] = 4; ib[7] = 5; ib[8] = 6;
	ib[9] = 4; ib[10] = 6; ib[11] = 7;
	ib[12] = 8; ib[13] = 9; ib[14] = 10;
	ib[15] = 8; ib[16] = 10; ib[17] = 11;
	ib[18] = 12; ib[19] = 13; ib[20] = 14;
	ib[21] = 12; ib[22] = 14; ib[23] = 15;
	ib[24] = 16; ib[25] = 17; ib[26] = 18;
	ib[27] = 16; ib[28] = 18; ib[29] = 19;
	ib[30] = 20; ib[31] = 21; ib[32] = 22;
	ib[33] = 20; ib[34] = 22; ib[35] = 23;
}

void InitMaterial() {
	Material mtrl;
	mtrl.Ambient = Color(1.0f, 1.0f, 1.0f);
	mtrl.Diffuse = Color(1.0f, 1.0f, 1.0f);
	mtrl.Specular = Color(1.0f, 1.0f, 1.0f);
	mtrl.Emissive = Color(0.0f, 0.0f, 0.0f);
	mtrl.Power = 5.0f;
	device->SetMaterial(&mtrl);
}

void InitLight() {
	Light light;
	light.Type = LIGHT_SPOT;
	light.Diffiuse = Color(1.0f, 1.0f, 1.0f);
	light.Specular = Color(0.3f, 0.3f, 0.3f);
	light.Ambient = Color(0.6f, 0.6f, 0.6f);
	light.Direction = MLVector3(0.0f, 0.0f, 1.0f);
	light.Position = MLVector3(-1.0f, 0.0f, 1.0f);
	light.Range = 1000.0f;
	light.Falloff = 1.0f;
	light.Attenuation0 = 1.0f;
	light.Attenuation1 = 0.0f;
	light.Attenuation2 = 0.0f;
	light.Theta = 0.4f;
	light.Phi = 0.9f;
	device->SetLight(&light);
	device->LightEnable(true);
}

void InitTexture() {
	Texture *tex;
	CreateTextureFromFile(L"crate.jpg", tex);
	device->SetTexture(tex);
}

bool Setup() {
	// create vertex buffer
	vb = new FPVertex[24];
	// create index buffer
	ib = new int[36];
	// fill vertex buffer and index buffer
	//InitCube(vb, ib);
	//InitPyramid(vb);
	InitTexCube(vb, ib);
	// init material
	InitMaterial();
	// init light
	InitLight();
	// init texture
	InitTexture();
	// set view matrix
	MLVector3 pos(0.0f, 1.0f, -4.0f);
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
	device->SetRenderState(FILL_TEXTURE);
	device->SetShadeMode(SHADE_GOURAUD);
	return true;
}

bool Display(float timeDelta) {
	MLMatrix4 Ry;
	//static float x = PI * 0.25f;
	//Matrix_RotationX(&Rx, x);
	static float y = 0.0f;
	Matrix_RotationY(&Ry, y);
	y += timeDelta;
	if (y >= PI * 2.0f)
		y = 0.0f;
	MLMatrix4 p = Ry;
	device->SetTransform(TRANSFORM_WORLD, &p);
	// clear back and depth buffer
	device->Clear(0x00000000, 1.0f);
	// draw
	device->SetStreamSource(vb);
	device->SetIndices(ib);
	device->DrawIndexedPrimitive(0, 12);
	//device->DrawPrimitive(0, 4);
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