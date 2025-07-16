#pragma once
#include <string>
#include <vector>
#include "Vector3.h"

struct Pixel{
	float r, g, b;
	Pixel(const Vector3& vec) : r(vec.x), g(vec.y), b(vec.z) {};
	Pixel(double a, double b, double c) : r(a), g(b), b(c) {};
	Pixel() {
		r = g = b = 0;
	}
};

class Image {
public:
	int height, width;
	double maxColor, aspectRatio;
	std::vector<std::vector<Pixel>> pixels;
	std::string filename;
	std::vector<unsigned char> pixelArray;
	Image(int width, int height, double maxColor, const std::string& filename);
	void loadImage();
	void saveImage();
	void saveImage(std::string& saveFile);
	void saveImage(const char* saveFile);
	Image(const std::string& filename);
	
};
