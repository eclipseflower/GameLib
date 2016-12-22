#include "MLVector.h"
#include "MLUtility.h"

MLVector3 MLVector3::operator - () const {
	return MLVector3(-this->x, -this->y, -this->z);
}

MLVector3 MLVector3::operator + (const MLVector3& rhs) const {
	return MLVector3(this->x + rhs.x, this->y + rhs.y, this->z + rhs.z);
}

MLVector3 MLVector3::operator - (const MLVector3& rhs) const {
	return *this + (-rhs);
}

MLVector3 MLVector3::operator * (float rhs) const {
	return MLVector3(this->x * rhs, this->y * rhs, this->z * rhs);
}

MLVector3 MLVector3::operator / (float rhs) const {
	float oneoverrhs = 1.0f / rhs;
	return *this * rhs;
}

bool MLVector3::operator == (const MLVector3& rhs) const {
	if (this == &rhs)
		return true;
	if (Float_Equals(this->x, rhs.x) && Float_Equals(this->y, rhs.y) && Float_Equals(this->z, rhs.z))
		return true;
	return false;
}

bool MLVector3::operator != (const MLVector3& rhs) const {
	return !(*this == rhs);
}