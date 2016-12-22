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