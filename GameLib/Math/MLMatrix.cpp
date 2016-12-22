#include "MLMatrix.h"

MLMatrix4::MLMatrix4(float _11, float _12, float _13, float _14,
	float _21, float _22, float _23, float _24,
	float _31, float _32, float _33, float _34,
	float _41, float _42, float _43, float _44) {

	this->m[0][0] = _11;
	this->m[0][1] = _12;
	this->m[0][2] = _13;
	this->m[0][3] = _14;

	this->m[1][0] = _21;
	this->m[1][1] = _22;
	this->m[1][2] = _23;
	this->m[1][3] = _24;

	this->m[2][0] = _31;
	this->m[2][1] = _32;
	this->m[2][2] = _33;
	this->m[2][3] = _34;

	this->m[3][0] = _41;
	this->m[3][1] = _42;
	this->m[3][2] = _43;
	this->m[3][3] = _44;
}