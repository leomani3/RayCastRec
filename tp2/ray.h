#pragma once
#include "vector3.h"

struct Ray {
	Vec3<float> pos;
	Vec3<float> direction;
	Vec3<float> color;

	Ray(Vec3<float> p, Vec3<float> d) {
		pos = p;
		direction = d;
		color = Vec3<float>{ 0, 0, 0 };
	}

	Ray(Vec3<float> p, Vec3<float> d, Vec3<float> c) {
		pos = p;
		direction = d;
		color = c;
	}
};

void DebugRay(Ray r) {
	printf("Ray : pos = [%f, %f, %f] , direction = [%f, %f, %f], color = [%f, %f, %f]", r.pos.x, r.pos.y, r.pos.z, r.direction.x, r.direction.y, r.direction.z, r.color.x, r.color.y, r.color.z);
}
