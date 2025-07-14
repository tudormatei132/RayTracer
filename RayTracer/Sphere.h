#pragma once
#include "Vector3.h"
class Sphere {
public:
	Vector3 center;
	double radius;
	Vector3 color;
	Sphere(const Vector3& center, const double radius, const Vector3& color);
};
