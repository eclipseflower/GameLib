#include "MLUtility.h"

bool Float_Equals(float lhs, float rhs) {
	return fabs(lhs - rhs) < EPSILON ? true : false;
}

float LinearInterpolation(float x1, float x2, float factor) {
	return x1 + factor * (x2 - x1);
}

float Vec3_Length(const MLVector3 *pV) {
	return sqrtf(pV->x * pV->x + pV->y * pV->y + pV->z * pV->z);
}

MLVector3 *Vec3_Normalize(MLVector3 *pOut, const MLVector3 *pV) {
	*pOut = *pV / Vec3_Length(pV);
	return pOut;
}

float Vec3_Dot(const MLVector3 *pV1, const MLVector3 *pV2) {
	return pV1->x * pV2->x + pV1->y * pV2->y + pV1->z * pV2->z;
}

float Vec4_Dot(const MLVector4 *pV1, const MLVector4 *pV2) {
	return pV1->x * pV2->x + pV1->y * pV2->y + pV1->z * pV2->z + pV1->w * pV2->w;
}

MLVector3 *Vec3_Cross(MLVector3 *pOut, const MLVector3 *pV1, const MLVector3 *pV2) {
	*pOut = MLVector3(
		pV1->y * pV2->z - pV1->z * pV2->y,
		pV1->z * pV2->x - pV1->x * pV2->z,
		pV1->x * pV2->y - pV1->y * pV2->x
	);
	return pOut;
}

MLVector4 *Vec4_Transform(MLVector4 *pOut, const MLVector4 *pV, const MLMatrix4 *pM) {
	MLVector4 result;
	for (int i = 0; i < 4; i++) {
		MLVector4 rhs(pM->m[0][i], pM->m[1][i], pM->m[2][i], pM->m[3][i]);
		float val = Vec4_Dot(pV, &rhs);
		switch (i) {
		case 0:
			result.x = val;
			break;
		case 1:
			result.y = val;
			break;
		case 2:
			result.z = val;
			break;
		case 3:
			result.w = val;
			break;
		}
	}
	*pOut = result;
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
		0, -sinf(angle), cosf(angle), 0,
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

MLMatrix4 *Matrix_LookAt(MLMatrix4 *pOut, const MLVector3 *pEye, const MLVector3 *pAt,
	const MLVector3 *pUp) {
	MLVector3 xaxis, yaxis, zaxis;
	Vec3_Normalize(&zaxis, &(*pAt - *pEye));
	Vec3_Cross(&xaxis, pUp, &zaxis);
	Vec3_Normalize(&xaxis, &xaxis);
	Vec3_Cross(&yaxis, &zaxis, &xaxis);

	*pOut = MLMatrix4(
		xaxis.x, yaxis.x, zaxis.x, 0,
		xaxis.y, yaxis.y, zaxis.y, 0,
		xaxis.z, yaxis.z, zaxis.z, 0,
		-Vec3_Dot(pEye, &xaxis), -Vec3_Dot(pEye, &yaxis), -Vec3_Dot(pEye, &zaxis), 1
	);
	return pOut;
}

MLMatrix4 *Matrix_PerspectiveFov(MLMatrix4 *pOut, float fovY, float Aspect, float zn, float zf) {
	float cot = 1.0f / tanf(fovY * 0.5f);
	*pOut = MLMatrix4(
		cot / Aspect, 0, 0, 0,
		0, cot, 0, 0,
		0, 0, zf / (zf - zn), 1,
		0, 0, zf * zn / (zn - zf), 0
	);
	return pOut;
}

MLMatrix4 *Matrix_Viewport(MLMatrix4 *pOut, float x, float y, int width, int height, 
	float minZ, float maxZ) {
	float halfW = width * 0.5f;
	float halfH = height * 0.5f;
	*pOut = MLMatrix4(
		halfW, 0, 0, 0,
		0, -halfH, 0, 0,
		0, 0, maxZ - minZ, 0,
		x + halfW, y + halfH, minZ, 1
	);
	return pOut;
}