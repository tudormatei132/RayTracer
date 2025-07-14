#pragma once
#include "Vector3.h"
#include "Sphere.h"
#include <vector>
class Ray {
public:
	Vector3 origin, direction;
	Ray(const Vector3& origin, const Vector3& destination);

	// Returns the moment when the ray hits the sphere, -1 if it doesn't
	double intersectsSphere(const Sphere& sphere);
	Vector3 colorPixel(const std::vector<Sphere>& spheres, const Vector3& backgroundColor);
};
