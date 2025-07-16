#pragma once
#include <cmath>
struct Vector3 {
	double x, y, z;
	Vector3() : x(0), y(0), z(0) {};
	Vector3(double x, double y, double z) : x(x), y(y), z(z) {};

	Vector3 operator+(const Vector3& other) const {
		return Vector3(x + other.x, y + other.y, z + other.z);
	}

	Vector3 operator*(double other) const {
		return Vector3(x * other, y * other, z * other);
	}

	Vector3 operator-(const Vector3& other) const {
		return Vector3(x - other.x, y - other.y, z - other.z);
	}

	// cross product
	Vector3 operator*(Vector3 other) const {
		return Vector3(y * other.z - z * other.y, z * other.x - other.z * x, x * other.y - y * other.x);
	}

	double dotProduct(const Vector3& other) const {
		return x * other.x + y * other.y + z * other.z;
	}

	Vector3 normalized() {
		double vecLength = std::sqrt(x * x + y * y + z * z);
		return Vector3(x / vecLength, y / vecLength, z / vecLength);
	}

	void normalize() {
		double vecLength = std::sqrt(x * x + y * y + z * z);
		
		x /= vecLength;
		y /= vecLength;
		z /= vecLength;
	}
	
	void add(Vector3 other) {
		x += other.x;
		y += other.y;
		z += other.z;
	}

};