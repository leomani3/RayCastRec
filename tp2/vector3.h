#pragma once
#include <iostream>

template<typename T>
struct Vec3 {
	T x, y, z;

	Vec3() {};

	Vec3(T x2, T y2, T z2) {
		x = x2;
		y = y2;
		z = z2;
	}
};


//Operateur egalitaire
template<typename T>
bool operator==(const Vec3<T>& a, const Vec3<T>& b) {
	return a.x == b.x && a.y == b.y && a.z == b.z;
};

//Operateur multiplicateur
template<typename T>
Vec3<T> operator*(const Vec3<T>& a, const Vec3<T>& b) {
	return Vec3{ a.x * b.x, a.y * b.y, a.z * b.z };
};

//Operateur multiplicateur par un scalair
template<typename T>
Vec3<T> operator*(const Vec3<T>& a, float f) {
	return Vec3{ a.x * f, a.y * f, a.z * f };
};

//Operateur diviseur
template<typename T>
Vec3<T> operator/(const Vec3<T>& a, const Vec3<T>& b) {
	return Vec3{ a.x / b.x, a.y / b.y, a.z / b.z };
};

//Operateur de soustraction
template<typename T>
Vec3<T> operator-(const Vec3<T>& a, const Vec3<T>& b) {
	return Vec3<T>{ a.x - b.x, a.y - b.y, a.z - b.z };
};

//Operateur d'addition
template<typename T>
Vec3<T> operator+(const Vec3<T>& a, const Vec3<T>& b) {
	return Vec3<T>{ a.x + b.x, a.y + b.y, a.z + b.z };
};

//Operateur de division par un scalair
template<typename T>
Vec3<T> operator/(const Vec3<T>& a, float f) {
	return Vec3<T>{ a.x / f, a.y / f, a.z / f};
};


//Calcul de la nomre d'un vecteur
template<typename T>
float norm(const Vec3<T>& v) {
	return std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
};


//Calcul du produit scalaire
template<typename T>
float dot(const Vec3<T>& a, const Vec3<T>& b) {
	return a.x* b.x + a.y * b.y + a.z * b.z;
};

//fonction de normalisation d'un vecteur
template<typename T>
Vec3<T> normalise(const Vec3<T> &v) {
	Vec3<T> res{ v.x / norm(v), v.y / norm(v), v.z / norm(v) };
	return res;
};

//Multiplication d'un vecteur par un scalair
template<typename T>
Vec3<T> multiplyByScalar(const Vec3<T> &v, int nb) {
	Vec3<T> res{ v.x * nb, v.y * nb, v.z * nb };
	return res;
}

template<typename T>
void VecDebug(Vec3<T> v) {
	printf("vector3 : [%f, %f, %f]\n", v.x, v.y, v.z);
}
