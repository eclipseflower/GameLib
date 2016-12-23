#pragma once
#include <math.h>
#include "MLVector.h"
#include "MLMatrix.h"
#include "MLPlane.h"
const float EPSILON = 0.001f;

bool Float_Equals(float lhs, float rhs);

float Vec3_Length(const MLVector3 *pV);

MLVector3 *Vec3_Normalize(MLVector3 *pOut, const MLVector3 *pV);

float Vec3_Dot(const MLVector3 *pV1, const MLVector3 *pV2);

MLVector3 *Vec3_Cross(MLVector3 *pOut, const MLVector3 *pV1, const MLVector3 *pV2);

MLMatrix4 *Matrix_Translation(MLMatrix4 *pOut, float x, float y, float z);

MLMatrix4 *Matrix_RotationX(MLMatrix4 *pOut, float angle);

MLMatrix4 *Matrix_RotationY(MLMatrix4 *pOut, float angle);

MLMatrix4 *Matrix_RotationZ(MLMatrix4 *pOut, float angle);

MLMatrix4 *Matrix_Scaling(MLMatrix4 *pOut, float sx, float sy, float sz);

// param: plane and point
float Plane_DotCoord(const MLPlane *pP, const MLVector3 *pV);