// Fix Pipeline implementation, just a demo
// Reference: https://github.com/skywind3000/mini3d
#include <Windows.h>
#include "Math/MLUtility.h"

const int Width = 800;
const int Height = 600;
const float PI = 3.1415927f;

const int TRANSFORM_VIEW = 1;
const int TRANSFORM_PROJECTION = 2;

const int FILL_WIREFRAME = 1;

struct Device {
	// view matrix
	MLMatrix4 _view;
	// projection matrix
	MLMatrix4 _proj;
	// render state
	int _rstate;

	void SetTransform(int state, const MLMatrix4 *m) {
		switch (state) {
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
};

struct Vertex {
	// position
	float _x, _y, _z;
	// normal
	float _nx, _ny, _nz;
	// color
	float _r, _g, _b;
	// constructor
	Vertex() {}
	Vertex(float x, float y, float z) {
		_x = x; _y = y; _z = z;
	}
};

void InitCube(Vertex *vb, int *ib) {
	vb[0] = Vertex(-1.0f, 1.0f, -1.0f);
	vb[1] = Vertex(1.0f, 1.0f, -1.0f);
	vb[2] = Vertex(1.0f, -1.0f, -1.0f);
	vb[3] = Vertex(-1.0f, -1.0f, -1.0f);
	vb[4] = Vertex(-1.0f, 1.0f, 1.0f);
	vb[5] = Vertex(1.0f, 1.0f, 1.0f);
	vb[6] = Vertex(1.0f, -1.0f, 1.0f);
	vb[7] = Vertex(-1.0f, -1.0f, 1.0f);

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

int FixPipeline(HINSTANCE hinstance, HINSTANCE prevInstance, PSTR cmdLine, int showCmd) {
	// create device
	Device device;
	// create vertex buffer
	Vertex *vb = new Vertex[8];
	// create index buffer
	int *ib = new int[36];
	// fill vertex buffer and index buffer
	InitCube(vb, ib);
	// set view matrix
	MLVector3 pos(0.0f, 0.0f, -5.0f);
	MLVector3 target(0.0f, 0.0f, 0.0f);
	MLVector3 up(0.0f, 1.0f, 0.0f);
	MLMatrix4 V;
	Matrix_LookAt(&V, &pos, &target, &up);
	device.SetTransform(TRANSFORM_VIEW, &V);
	// set projection matrix
	MLMatrix4 proj;
	Matrix_PerspectiveFov(&proj, PI * 0.5f, (float)Width / (float)Height, 1.0f, 1000.0f);
	device.SetTransform(TRANSFORM_PROJECTION, &proj);
	// set render state
	device.SetRenderState(FILL_WIREFRAME);
	return 0;
}