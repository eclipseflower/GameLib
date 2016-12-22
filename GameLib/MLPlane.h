#pragma once
class MLPlane {
public:
	// normal: a, b, c
	// plane: np + d = 0
	float a, b, c, d;
	MLPlane() {};
	MLPlane(float a, float b, float c, float d) {
		this->a = a;
		this->b = b;
		this->c = c;
		this->d = d;
	}
};