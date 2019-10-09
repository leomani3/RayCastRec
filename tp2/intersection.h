#pragma once
#include "vector3.h"
#include <optional>
struct Intersection {
	float t;
	Vec3<float> impactWorldPoint;
	Vec3<float> objectColor;
	int objectIndex;

	Intersection(float t, Vec3<float> impact, Vec3<float> c) {
		t = t;
		impactWorldPoint = impact;
		objectColor = c;
		objectIndex = 0;
	}

	Intersection() {
		t = 0;
		impactWorldPoint = Vec3<float>{ 0, 0, 0 }; 
		objectColor = Vec3<float>{ 0, 0, 0 };
		objectIndex = -1;
	}
};

void DebugIntersection(Intersection inter) {
	printf("Intersection : t = %f, impactWoldPoint = [%f, %f, %f] object color = [%f, %f, %f]\n", inter.t, inter.impactWorldPoint.x, inter.impactWorldPoint.y, inter.impactWorldPoint.z, inter.objectColor.x, inter.objectColor.y, inter.objectColor.z);
}