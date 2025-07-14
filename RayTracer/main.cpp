#include <iostream>
#include "Image.h"
#include "Sphere.h"
#include "Vector3.h"
#include "Ray.h"
int main() {
	
	Image img(400, 300, 255, "test.ppm");
	
	std::vector<Sphere> spheres;
	spheres.push_back(Sphere(Vector3(0.0, 0.0, -5.0), 1.0, Vector3(255, 0, 0)));
	spheres.push_back(Sphere(Vector3(3.0, 0.0, -6.0), 1.0, Vector3(0, 255, 0))); 

	Vector3 camera = Vector3(0, 0, 0);
	Vector3 background = Vector3(0, 0, 0);

	for (int i = 0; i < img.height; i++) {
		for (int j = 0; j < img.width; j++) {
			double u = double(j) / (img.width - 1);
			double v = double(i) / (img.height - 1);

			double x = (2 * u - 1) * img.aspectRatio;
			double y = (1 - 2 * v);
			
			Ray ray = Ray(camera, (Vector3(x, y, -1) - camera).normalized());
			img.pixels[i][j] = Pixel(ray.colorPixel(spheres, background));
		}
	}

	img.saveImage("result.ppm");


	return 0;
}