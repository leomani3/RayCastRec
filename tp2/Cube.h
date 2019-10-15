#pragma once
#include "vector3.h"

struct Cube {
	Vec3<float> pMin;
	Vec3<float> pMax;
	Vec3<float> color;

	Cube(Vec3<float> min, Vec3<float> max, Vec3<float> col) {
		pMin = min;
		pMax = max;
		color = col;
	}
};
