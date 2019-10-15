#pragma once
#include "vector3.h"

struct Sphere {
	Vec3<float> center;
	float rayon;
	Vec3<float> color;
	bool isMirror;

	Sphere(Vec3<float> c, float r, Vec3<float> col, bool b) {
		center = c;
		rayon = r;
		color = col;
		isMirror = b;
	}
};