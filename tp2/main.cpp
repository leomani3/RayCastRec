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

	float delta = (b * b) - (4 * a * c);

	if (delta > 0) {
		float t = (-b - sqrt(delta)) / (2 * a);
		if (t > 0) {
			res.t = t;
			res.impactWorldPoint = r.pos + (normalizedDir * t);
			res.objectColor = sphere.color;
		}
		else if (t < 0) {
			t = (-b + sqrt(delta)) / (2 * a);
			if (t > 0) {
				res.t = t;
				res.impactWorldPoint = r.pos + (normalizedDir * t);
				res.objectColor = sphere.color;
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
	}
	return res;
}

//test si le rayon intersect avec tous les objets de la scène et renvoie l'intersection la plus proche
std::optional<Intersection> IntersectWithAllObjects(Ray ray, std::vector<Sphere> objects) {
	std::optional<Intersection> intersection = std::nullopt;

	for (int i = 0; i < objects.size(); i++)
	{
		std::optional<Intersection> tmpIntersection = intersect(ray, objects[i]);
		DebugIntersection(tmpIntersection.value());
		if ((!intersection.has_value() && tmpIntersection.has_value()) || (intersection.has_value() && tmpIntersection.has_value() && tmpIntersection.value().t < intersection.value().t))
		{
			intersection = tmpIntersection;
			intersection.value().objectIndex = i;
		}
	}
	//DebugIntersection(intersection.value());
	return intersection;
}

bool CheckIfObstacle(Ray ray, std::vector<Sphere> objects) {
	std::optional<Intersection> intersection = IntersectWithAllObjects(ray, objects);

	if (intersection.has_value() && intersection.value().t <= norm(ray.direction))
		return true;
	else
		return false;
}

//Permet d'avancer dans le lancer de rayon. renvoie la couleur du rayon.
Vec3<float> CastRay(Ray ray, int n, std::vector<Sphere> objects, std::vector<Light> lights) {
	//On test le rayon avec tous les objets de la scène et on retourne l'intersection la plus proche
	std::optional<Intersection> intersection = IntersectWithAllObjects(ray, objects);

	//Si le rayon a touché quelque chose
	if (intersection.has_value())
	{
		//printf("intersect\n");
		//calcul de la couleur de l'objet
		float R = 0, G = 0, B = 0;
		for (int l = 0; l < lights.size(); l++) // pour toutes les lumières
		{
			for (int lightPoint = 0; lightPoint < lightDef; lightPoint++) //pour chaque petit point de la lumière
			{
				std::uniform_real_distribution<float> randomX(-lights[l].width / 2.0f, lights[l].width / 2.f);
				std::uniform_real_distribution<float> randomY(-lights[l].height / 2.0f, lights[l].height / 2.f);
				std::uniform_real_distribution<float> randomZ(-lights[l].depth / 2.0f, lights[l].depth / 2.f);
				Vec3<float> randomOffset = { randomX(generator), randomY(generator), randomZ(generator) };
				Vec3<float> fromImpactToLight = (lights[l].pos + randomOffset) - intersection.value().impactWorldPoint;

				Vec3<float> objectNormal = intersection.value().impactWorldPoint - objects[intersection.value().objectIndex].center;

				Ray rayToLight = Ray{intersection.value().impactWorldPoint, fromImpactToLight};
				float lightImpactAngle = dot(normalise(objectNormal), normalise(fromImpactToLight));

				if(!CheckIfObstacle) {
					R += intersection.value().objectColor.x * lights[l].color.x * lightImpactAngle * std::pow(norm(fromImpactToLight), 2);
					G += intersection.value().objectColor.y * lights[l].color.y * lightImpactAngle * std::pow(norm(fromImpactToLight), 2);
					B += intersection.value().objectColor.z * lights[l].color.z * lightImpactAngle * std::pow(norm(fromImpactToLight), 2);
				}
			}
			R /= lightDef;
			G /= lightDef;
			B /= lightDef;
		}

		Vec3<float> color = {R, G, B};


		//test de la fin de récursion

		//TODO : Il faudra mettre quelque part là dedans le test pour voir si c'est un miroir Si c'est le cas, il faudra juste renvoyer la couleur du prochaine rayon sans l'additionner
		if (n == 1)
		{
			//return ray.color;
			return color;
		}
		//continue la récursion
		else
		{
			return ray.color * CastRay(ray, n - 1, objects, lights);
		}
	}
	//Si le rayon n'a rien touché, renvoie une couleur réprésentant le vide (ici noir)
	else {
		//printf("pas intersect\n");
		return Vec3<float>{ 0.0, 0.0, 0.0 };
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

	/*Sphere fondGris(Vec3<float>{screenWidth / 2.0f, screenHeight / 2.0f, 15000}, 14000, Vec3<float>{1, 1, 1});
	objects.push_back(fondGris);
	Sphere solVert(Vec3<float>{screenWidth / 2.0f, (float)screenHeight + 16000 - 250, screenWidth / 2.0f + 2600}, 16000 + 50, Vec3<float>{0.1, 0.8, 0.1});
	objects.push_back(solVert);
	Sphere droite(Vec3<float>{(float)screenWidth + 16000 - 300, (float)screenHeight / 2.0f, screenWidth / 2.0f + 3000}, 16000 + 50, Vec3<float>{1, 1, 0.1});
	objects.push_back(droite);
	Sphere gauche(Vec3<float>{-16000 + 300, (float)screenHeight / 2.0f, screenWidth / 2.0f + 3000}, 16000 + 50, Vec3<float>{0.1, 1, 1});
	objects.push_back(gauche);
	Sphere haut(Vec3<float>{screenWidth / 2.0f, -16000 + 300, screenWidth / 2.0f + 3000}, 16000 + 50, Vec3<float>{1, 0.1, 1});
	objects.push_back(haut);*/

	Sphere sphereRouge(Vec3<float>{screenWidth / 2.0f - 100, screenHeight / 2.0f, 500}, 50, Vec3<float>{1, 0.05, 0.05});
	objects.push_back(sphereRouge);
	/*Sphere sphereVerte(Vec3<float>{screenWidth / 2.0f + 250, screenHeight / 2.0f - 200, 250}, 30, Vec3<float>{0.05, 1, 0.05});
	objects.push_back(sphereVerte);*/

	/*Light light1{ Vec3<float>{screenWidth / 2.0f, 0, 600}, Vec3<float>{100000000, 100000000, 100000000}};
	lights.push_back(light1);*/
	Light light2{ Vec3<float>{screenWidth / 2.0f, screenHeight / 2.0f - 200, 0}, 50, 1, 50, Vec3<float>{50000000, 50000000, 50000000} };
	lights.push_back(light2);



	for (int j = 0; j < screenHeight; j++)
	{
		int rayIndex = 0;
		for (int i = 0; i < screenWidth; i+=3)
		{
			Vec3<float> rayDir = Vec3<float>{ (float)rayIndex, (float)j, 0 } - camera;
			//VecDebug(rayDir);
			Ray ray = Ray{camera, rayDir };

			//DebugRay(ray);
			//CastRay(ray, 1, objects, lights);
			CastRay(Ray{ Vec3<float>{0, 0, 0}, Vec3<float>{0, 0, -1} }, 1, objects, lights);
			
			rayIndex++;
		}
	}

/*#pragma omp parallel for
	for (int j = 0; j < screenHeight; j++) {
		int rayIndex = 0;
		for (int i = 0; i < screenWidth * 3; i += 3) {
			int indexCurrentObject;

			//---------------------RAYCAST INITIAL-------------------------
			Intersection initialIntersection = Intersection();
			Vec3<float> perspective = Vec3<float>{ (float)rayIndex, (float)j, 0 } - camera;
			perspective = normalise(perspective);
			for (int k = 0; k < objects.size(); k++) {
				Intersection tmp = intersect(Ray(Vec3<float>{(float)rayIndex, (float)j, 0}, perspective), objects[k]);
				if ((!initialIntersection.t.has_value() && tmp.t.has_value()) || (tmp.t.has_value() && initialIntersection.t.has_value() && tmp.t.value() < initialIntersection.t.value())) {
					initialIntersection.t = tmp.t.value();
					initialIntersection.color = tmp.color;
					indexCurrentObject = k;
				}
			}
			//---------------------------------------------------------------


			if (initialIntersection.t.has_value()) {
				Vec3<float> currentPixel = camera + multiplyByScalar(perspective, initialIntersection.t.value());
				Vec3<float> normale = currentPixel - objects[indexCurrentObject].center;
				normale = normalise(normale);
				currentPixel = currentPixel + multiplyByScalar(normale, 1.1);

				int R = 0, G = 0, B = 0;
				for (int l = 0; l < lights.size(); l++)
				{
					//Pour lightDef points de chaque lumière surfacique
					for (int lightPoint = 0; lightPoint < lightDef; lightPoint++)
					{
						//-----------------RAYCAST ÉCLAIRAGE--------------------------------
						std::uniform_real_distribution<float> randomX(-lights[l].width / 2.0f, lights[l].width / 2.f);
						std::uniform_real_distribution<float> randomY(-lights[l].height / 2.0f, lights[l].height / 2.f);
						std::uniform_real_distribution<float> randomZ(-lights[l].depth / 2.0f, lights[l].depth / 2.f);
						Vec3<float> randomOffset = { randomX(generator), randomY(generator), randomZ(generator) };
						Vec3<float> fromPixelToLight = (lights[l].pos + randomOffset) - currentPixel;

						std::optional<float> obstacleBeforeLight;
						for (int i = 0; i < objects.size(); i++) {
							Intersection tmp = intersect(Ray{ currentPixel, fromPixelToLight }, objects[i]);
							if (tmp.t.has_value() && tmp.t.value() <= norm(fromPixelToLight)) {
								obstacleBeforeLight = tmp.t.value();
							}
						}
						//-------------------------------------------------------------------

						if (!obstacleBeforeLight.has_value()) {
							Vec3<float> currentPixelNormaleNormalized = normalise(currentPixel - objects[indexCurrentObject].center);
							Vec3<float> fromPixelToLightNormalized = normalise(fromPixelToLight);

							float lightImpactAngle = dot(currentPixelNormaleNormalized, fromPixelToLightNormalized);
							if (lightImpactAngle > 0)
							{
								R += initialIntersection.color.x * lights[l].color.x * lightImpactAngle / std::pow(norm(fromPixelToLight), 2);
								G += initialIntersection.color.y * lights[l].color.y * lightImpactAngle / std::pow(norm(fromPixelToLight), 2);
								B += initialIntersection.color.z * lights[l].color.z * lightImpactAngle / std::pow(norm(fromPixelToLight), 2);
							}
						}
					}
					R /= lightDef;
					G /= lightDef;
					B /= lightDef;
				}
				R = std::clamp(R, 0, 255);
				G = std::clamp(G, 0, 255);
				B = std::clamp(B, 0, 255);

				ImgOut[j * screenWidth * 3 + i] = R;
				ImgOut[j * screenWidth * 3 + i + 1] = G;
				ImgOut[j * screenWidth * 3 + i + 2] = B;
			}
			else {
				ImgOut[j * screenWidth * 3 + i] = 127;
				ImgOut[j * screenWidth * 3 + i + 1] = 127;
				ImgOut[j * screenWidth * 3 + i + 2] = 127;
			}
			rayIndex++;
		}
		//rayIndex = 0;

		//Fonction : intersectWithScene(Ray r);
		
	}*/

	ecrire_image_ppm(nameImgOut, ImgOut, screenWidth, screenHeight);
}
