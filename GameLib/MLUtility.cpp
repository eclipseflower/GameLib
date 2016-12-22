#include "MLUtility.h"

bool Float_Equals(float lhs, float rhs) {
	return fabs(lhs - rhs) < EPSILON ? true : false;
}

float Vec3_Length(const MLVector3 *pV) {
	return sqrtf(pV->x * pV->x + pV->y * pV->y + pV->z * pV->z);
}

MLVector3 *Vec3_Normalize(MLVector3 *pOut, const MLVector3 *pV) {
	*pOut = *pV / Vec3_Length(pV);
	return pOut;
}

float Vec3_Dot(const MLVector3 *pV1, const MLVector3 *pV2) {
	return pV1->x * pV2->x + pV1->y * pV2->y + pV1->z + pV2->z;
}

MLVector3 *Vec3_Cross(MLVector3 *pOut, const MLVector3 *pV1, const MLVector3 *pV2) {
	*pOut = MLVector3(
		pV1->y * pV2->z - pV1->z * pV2->y,
		pV1->z * pV2->x - pV1->x * pV2->z,
		pV1->x * pV2->y - pV1->y * pV2->x
	);
	return pOut;
}

MLMatrix4 *Matrix_Translation(MLMatrix4 *pOut, float x, float y, float z) {
	*pOut = MLMatrix4(
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		x, y, z, 1
	);
	return pOut;
}

MLMatrix4 *Matrix_RotationX(MLMatrix4 *pOut, float angle) {
	*pOut = MLMatrix4(
		1, 0, 0, 0,
		0, cosf(angle), sinf(angle), 0,
		0, -sinf(angle), cos(angle), 0,
		0, 0, 0, 1
	);
	return pOut;
}

MLMatrix4 *Matrix_RotationY(MLMatrix4 *pOut, float angle) {
	*pOut = MLMatrix4(
		cosf(angle), 0, -sinf(angle), 0,
		0, 1, 0, 0,
		sinf(angle), 0, cosf(angle), 0,
		0, 0, 0, 1
	);
	return pOut;
}

MLMatrix4 *Matrix_RotationZ(MLMatrix4 *pOut, float angle) {
	*pOut = MLMatrix4(
		cosf(angle), sinf(angle), 0, 0,
		-sinf(angle), cosf(angle), 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	);
	return pOut;
}

MLMatrix4 *Matrix_Scaling(MLMatrix4 *pOut, float sx, float sy, float sz) {
	*pOut = MLMatrix4(
		sx, 0, 0, 0,
		0, sy, 0, 0,
		0, 0, sz, 0,
		0, 0, 0, 1
	);
	return pOut;
}

float Plane_DotCoord(const MLPlane *pP, const MLVector3 *pV) {
	return Vec3_Dot(&MLVector3(pP->a, pP->b, pP->c), pV) + pP->d;
}