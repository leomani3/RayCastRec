#include <stdio.h>
#include <iostream>
#include "pch.h"
#include "vector3.h"
#include "ray.h"
#include "sphere.h"
#include "image_ppm.h"
#include <cmath>
#include "light.h"
#include "util.h"
#include <random>
#include <omp.h>
#include "intersection.h"

using namespace winrt;
using namespace Windows::Foundation;

int lightDef;
std::default_random_engine generator;

//Test s'il le rayon r intersect la sphere
std::optional<Intersection> intersect(Ray r, Sphere sphere) {
	Intersection res;

	Vec3<float> normalizedDir = normalise(r.direction);
	float a = dot(normalizedDir, normalizedDir); //a=1 car on a normalisé la direction
	float b = 2 * (dot(r.pos, normalizedDir) - dot(sphere.center, normalizedDir));
	float c = dot(r.pos, r.pos) + dot(sphere.center, sphere.center) - 2 * dot(sphere.center, r.pos) - (sphere.rayon * sphere.rayon);

	float delta = (b * b) - 4 * a * c;

	if (delta > 0) {
		float t = (-b - sqrt(delta)) / (2 * a);
		if (t > 0) {
			res.t = t;
			res.impactWorldPoint = r.pos + (normalizedDir * t);
			res.objectColor = sphere.color;
			return res;
		}
		else if (t < 0) {
			t = (-b + sqrt(delta)) / (2 * a);
			if (t > 0) {
				res.t = t;
				res.impactWorldPoint = r.pos + (normalizedDir * t);
				res.objectColor = sphere.color;
				return res;
			}
		}
		else {
			return std::nullopt;
		}
	}
	else if (delta < 0) {

		return std::nullopt;
	}
	else {
		res.t = -b - sqrt(delta) / (2 * a);
		res.impactWorldPoint = r.pos + (normalizedDir * res.t);
		res.objectColor = sphere.color;
		return res;
	}
	return std::nullopt;
}

//test si le rayon intersect avec tous les objets de la scène et renvoie l'intersection la plus proche
std::optional<Intersection> IntersectWithAllObjects(Ray ray, std::vector<Sphere> objects) {
	std::optional<Intersection> intersection = std::nullopt;
	for (int i = 0; i < objects.size(); i++) {
		std::optional<Intersection> tmp = intersect(ray, objects[i]);

		if ((tmp.has_value() && !intersection.has_value()) || (intersection.has_value() && tmp.has_value() && tmp.value().t < intersection.value().t))
		{
			intersection = tmp;
			intersection.value().objectIndex = i;
		}
	}

	return intersection;
}

bool CheckIfObstacle(Ray ray, std::vector<Sphere> objects) {
	std::optional<Intersection> intersection = IntersectWithAllObjects(ray, objects);
	if (intersection.has_value() && intersection.value().t <= norm(ray.direction)) {
		return true;
	}
	else {
		return false;
	}
}


Vec3<float> ComputeColor(std::optional<Intersection> intersection, std::vector<Sphere> objects, std::vector<Light> lights) {

	int lightDef = 4;

	Vec3<float> res = Vec3<float>{ 0, 0, 0};

	for (int l = 0; l < lights.size(); l++) {
		for (int k = 0; k < lightDef; k++) {
			std::uniform_real_distribution<float> randomX(-lights[l].width / 2.0f, lights[l].width / 2.f);
			std::uniform_real_distribution<float> randomY(-lights[l].height / 2.0f, lights[l].height / 2.f);
			std::uniform_real_distribution<float> randomZ(-lights[l].depth / 2.0f, lights[l].depth / 2.f);
			Vec3<float> randomOffset = { randomX(generator), randomY(generator), randomZ(generator) };

			Vec3<float> impactToLight = (lights[l].pos + randomOffset) - intersection.value().impactWorldPoint;
			Vec3<float> impactPoint = intersection.value().impactWorldPoint + normalise(impactToLight);
			Ray rayToLight = { impactPoint, impactToLight };

			if (CheckIfObstacle(rayToLight, objects)) { //dans l'ombre
				res = res + Vec3<float>{ 0, 0, 0 };
			}
			else { //éclairé
				Vec3<float> objectNormale = normalise(impactPoint - objects[intersection.value().objectIndex].center);
				float lightImpactAngle = dot(objectNormale, normalise(impactToLight));
				if (lightImpactAngle > 0) {
					res = res + (intersection.value().objectColor * lights[l].color * lightImpactAngle / std::pow(norm(impactToLight), 2));
				}
			}
		}
		res = res / lightDef;
	}

	return res;
}

//Permet d'avancer dans le lancer de rayon. renvoie la couleur du rayon.
Vec3<float> CastRay(Ray ray, std::vector<Sphere> objects, std::vector<Light> lights, int depth) {
	if (depth > 10)
	{
		return Vec3<float>{ 0, 0, 0 };
	}
	Vec3<float> res = { 0, 0, 0 };
	//On test le rayon avec tous les objets de la scène et on retourne l'intersection la plus proche
	std::optional<Intersection> intersection = IntersectWithAllObjects(ray, objects);

	//ray.color = ray.color * ComputeColor(intersection, objects, lights);
	if (intersection.has_value())
	{
		res = ComputeColor(intersection, objects, lights);

		//test miroir
		if (objects[intersection.value().objectIndex].isMirror) {
			Vec3<float> objectNormale = normalise(intersection.value().impactWorldPoint - objects[intersection.value().objectIndex].center);
			Vec3<float> R = objectNormale * 2 * dot(normalise(ray.direction * (-1)), objectNormale) + normalise(ray.direction);
			Vec3<float> impactPoint = intersection.value().impactWorldPoint + R;
			Ray rebond = { impactPoint, R, ray.color };

			return ray.color * CastRay(rebond, objects, lights, ++depth);
		}
		else { // pas miroir
			return ray.color * res;
		}
	}
	else {
		return Vec3<float>{ 0, 0, 0 }; //le rayon n'a rien touché = néant
	}


}

int main()
{
	int screenWidth = 1024, screenHeight = 1024, screenSize;
	screenSize = screenHeight * screenWidth;
	lightDef = 50;

	//std::default_random_engine generator;

	char nameImgOut[250] = "test.ppm";

	OCTET* ImgOut;
	allocation_tableau(ImgOut, OCTET, screenSize * 3);

	std::vector<Sphere> objects;
	std::vector<Light> lights;

	Vec3<float> camera = { screenWidth / 2.0f, screenHeight / 2.0f, -5000000 };

	Sphere fondGris(Vec3<float>{screenWidth / 2.0f, screenHeight / 2.0f, 15000}, 14000, Vec3<float>{1, 1, 1}, false);
	objects.push_back(fondGris);
	Sphere solVert(Vec3<float>{screenWidth / 2.0f, (float)screenHeight + 16000 - 250, screenWidth / 2.0f + 2600}, 16000 + 50, Vec3<float>{0.1, 0.8, 0.1}, false);
	objects.push_back(solVert);
	Sphere droite(Vec3<float>{(float)screenWidth + 16000 - 300, (float)screenHeight / 2.0f, screenWidth / 2.0f + 3000}, 16000 + 50, Vec3<float>{1, 1, 0.1}, false);
	objects.push_back(droite);
	Sphere gauche(Vec3<float>{-16000 + 300, (float)screenHeight / 2.0f, screenWidth / 2.0f + 3000}, 16000 + 50, Vec3<float>{0.1, 1, 1}, false);
	objects.push_back(gauche);
	Sphere haut(Vec3<float>{screenWidth / 2.0f, -16000 + 300, screenWidth / 2.0f + 3000}, 16000 + 50, Vec3<float>{1, 0.1, 1}, false);
	objects.push_back(haut);

	Sphere sphere1(Vec3<float>{screenWidth / 2.0f, screenHeight / 2.0f, 600}, 200, Vec3<float>{1, 0.05, 0.05}, true);
	objects.push_back(sphere1);
	Sphere sphere2(Vec3<float>{screenWidth / 2.0f + 100, screenHeight / 2.0f, 200}, 50, Vec3<float>{0.05, 1, 0.05}, false);
	objects.push_back(sphere2);

	Light light{ Vec3<float>{screenWidth / 2.0f, screenHeight / 2.0f + 200, -200}, 50, 1, 50, Vec3<float>{90000000, 90000000, 90000000} };
	lights.push_back(light);
	/*Light light2{ Vec3<float>{screenWidth / 2.0f, screenHeight / 2.0f, -1000}, 50, 1, 50, Vec3<float>{500000, 500000, 500000} };
	lights.push_back(light2);*/



	//Pour tous les pixels
	for (int j = 0; j < screenHeight; j++)
	{
		int rayIndex = 0;
		for (int i = 0; i < screenWidth * 3; i+=3)
		{
			Vec3<float> rayDir = Vec3<float>{ (float)rayIndex, (float)j, 0 } - camera;
			Ray ray = Ray{ Vec3<float>{ (float)rayIndex, (float)j, 0 }, rayDir, Vec3<float>{1, 1, 1 } };

			Vec3<float> pixelColor = CastRay(ray, objects, lights, 0);

			pixelColor.x = clamp(0, 255, pixelColor.x);
			pixelColor.y = clamp(0, 255, pixelColor.y);
			pixelColor.z = clamp(0, 255, pixelColor.z);

			ImgOut[j * screenWidth * 3 + i] = pixelColor.x;
			ImgOut[j * screenWidth * 3 + i + 1] = pixelColor.y;
			ImgOut[j * screenWidth * 3 + i + 2] = pixelColor.z;
			
			rayIndex++;
		}
	}

	ecrire_image_ppm(nameImgOut, ImgOut, screenWidth, screenHeight);
}
