#pragma once
#include "vector3.h"

struct Light {
	Vec3<float> pos;
	float width;
	float height;
	float depth;
	Vec3<float> color;

	Light(Vec3<float> p, float w, float h, float d, Vec3<float> col) {
		pos = p;
		width = w;
		height = h;
		depth = d;
		color = col;
	}
};

