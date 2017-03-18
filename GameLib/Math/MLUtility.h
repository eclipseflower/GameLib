#pragma once
#include <math.h>
#include "MLVector.h"
#include "MLMatrix.h"
#include "MLPlane.h"
const float EPSILON = 0.001f;

bool Float_Equals(float lhs, float rhs);

// linear interploation
float LinearInterpolation(float x1, float x2, float factor);

float Vec3_Length(const MLVector3 *pV);

MLVector3 *Vec3_Normalize(MLVector3 *pOut, const MLVector3 *pV);

float Vec3_Dot(const MLVector3 *pV1, const MLVector3 *pV2);

float Vec4_Dot(const MLVector4 *pV1, const MLVector4 *pV2);

MLVector3 *Vec3_Cross(MLVector3 *pOut, const MLVector3 *pV1, const MLVector3 *pV2);

MLVector4 *Vec4_Transform(MLVector4 *pOut, const MLVector4 *pV, const MLMatrix4 *pM);

MLMatrix4 *Matrix_Translation(MLMatrix4 *pOut, float x, float y, float z);

MLMatrix4 *Matrix_RotationX(MLMatrix4 *pOut, float angle);

MLMatrix4 *Matrix_RotationY(MLMatrix4 *pOut, float angle);

MLMatrix4 *Matrix_RotationZ(MLMatrix4 *pOut, float angle);

MLMatrix4 *Matrix_Scaling(MLMatrix4 *pOut, float sx, float sy, float sz);

MLMatrix4 *Matrix_Transpose(MLMatrix4 *pOut, const MLMatrix4 *pM);

MLMatrix4 *Matrix_Inverse(MLMatrix4 *pOut, const MLMatrix4 *pM);

// calculate 3x3 cofactor
float Cofactor3x3(const MLMatrix4 *pM, int row, int col);

// param: plane and point
// out: np + d
float Plane_DotCoord(const MLPlane *pP, const MLVector3 *pV);

// view matrix
MLMatrix4 *Matrix_LookAt(MLMatrix4 *pOut, const MLVector3 *pEye, const MLVector3 *pAt,
	const MLVector3 *pUp);

// projection matrix
MLMatrix4 *Matrix_PerspectiveFov(MLMatrix4 *pOut, float fovY, float Aspect, float zn, float zf);

// viewport matrix
MLMatrix4 *Matrix_Viewport(MLMatrix4 *pOut, float screenx, float screeny, int width, int height,
	float minZ = 0.0f, float maxZ = 1.0f);