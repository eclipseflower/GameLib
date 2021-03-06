#pragma once
struct MLVector {
	float x, y, z;
};

class MLVector3 : public MLVector {
public:
	// constructors
	MLVector3() {};
	MLVector3(float x, float y, float z) {
		this->x = x;
		this->y = y;
		this->z = z;
	}
	// unary operators
	MLVector3 operator - () const;
	// binary operators
	MLVector3 operator + (const MLVector3&) const;
	MLVector3 operator - (const MLVector3&) const;
	MLVector3 operator * (float) const;
	MLVector3 operator / (float) const;

	bool operator == (const MLVector3&) const;
	bool operator != (const MLVector3&) const;
};

class MLVector4 {
public:
	// members
	float x, y, z, w;
	// constructors
	MLVector4() {};
	MLVector4(float x, float y, float z, float w) {
		this->x = x;
		this->y = y;
		this->z = z;
		this->w = w;
	}
	// binary operators
	MLVector4 operator * (float) const;
	MLVector4 operator / (float) const;
	// assignment operators
	MLVector4& operator *= (float);
	MLVector4& operator /= (float);
};