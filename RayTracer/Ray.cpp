#include "Ray.h"
#include <cmath>
#include <iostream>
Ray::Ray(const Vector3& origin, const Vector3& direction) : origin(origin), direction(direction) {};

double Ray::intersectsSphere(const Sphere& sphere) {
	Vector3 diff = origin - sphere.center;

	double a = direction.dotProduct(direction);
	double b = 2 * diff.dotProduct(direction);
	double c = diff.dotProduct(diff) - sphere.radius * sphere.radius;
	double delta = b * b - 4 * a * c;
	
	if (delta < 0) {
		return -1;
	}
	
	double t1 = (-b - std::sqrt(delta)) / (2.0 * a);
	double t2 = (-b + std::sqrt(delta)) / (2.0 * a);
	double t = (t1 >= 0) ? t1 : t2;
	
	return (t >= 0) ? t : -1;
}

Vector3 Ray::colorPixel(const std::vector<Sphere>& spheres, const Vector3& backgroundColor) {
	double closest_t = 1e9;
	const Sphere *s = nullptr;
	for (auto const& sphere : spheres) {
		double t = intersectsSphere(sphere);
		if (t < closest_t && t != -1) {
			closest_t = t;
			s = &sphere;
		}
	}

	if (s != nullptr) {
		return s->color;
	}

	return backgroundColor;
}