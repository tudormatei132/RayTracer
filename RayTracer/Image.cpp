#include "Image.h"
#include <fstream>
#include <sstream>

std::ifstream fin;
std::ofstream fout;

Image::Image(int width, int height, double maxColor, const std::string& filename)
	: width(width), height(height), maxColor(maxColor), filename(filename) {
	pixels.resize(height, std::vector<Pixel>(width));
	aspectRatio = (double) width / height;
	pixelArray.resize(height * width * 3, 0);
}

Image::Image(const std::string& filename)
	:filename(filename) {
	width = 0;
	height = 0;
	maxColor = 255;
	aspectRatio = 0;
}
// Implementation for loading the image from the file

void Image::loadImage() {
	

	fin.open(filename, std::ios::in);

	

	if (!fin) {
		throw std::runtime_error("Could not open file for reading: " + filename);
	}

	// Read the image from the file

	std::string header;
	std::getline(fin, header);
	
	while (std::getline(fin, header)) {
		if (header.size() > 0 && header[0] == '#') {
			continue; // Skip comments
		}
		std::istringstream iss(header);
		iss >> width >> height;
		if (iss.fail()) {
			throw std::runtime_error("Error reading image dimensions from file: " + filename);
		}
		break; // Exit after reading the dimensions
	}

	aspectRatio = (double) width / height;

	// Allocate memory for the vector of pixels
	if (pixels.capacity() == 0) {
		pixels.resize(height, std::vector<Pixel>(width));
		pixelArray.resize(height * width * 3);
	}

	// Get maxColor
	while (std::getline(fin, header)) {
		if (header.size() > 0 && header[0] == '#') {
			continue; // Skip comments
		}
		std::istringstream iss(header);
		iss >> maxColor;
		if (iss.fail()) {
			throw std::runtime_error("Error reading max color value from file: " + filename);
		}
		break; // Exit after reading the max color value
	}
	int j = 0;
	while (std::getline(fin, header)) {
		
		if (header.size() > 0 && header[0] == '#') {
			continue;
		}
		std::istringstream iss(header);
		for (int i = 0; i < width; i++) {
			float r, g, b;
			iss >> r >> g >> b;
			if (iss.fail()) {
				throw std::runtime_error("Error reading pixel data from file: " + filename);
			}
			pixels[j][i] = Pixel(r, g, b);
		}
		j++;
	}
	fin.close();
}

void Image::saveImage(std::string& saveFile) {
	fout.open(saveFile, std::ios::out);
	if (!fout) {
		throw std::runtime_error("Could not open file for writing: " + saveFile);
	}

	if (height == 0 || width == 0 || pixels.capacity() == 0 || pixels[0].capacity() == 0) {
		throw std::runtime_error("No pixel data to write to file: " + saveFile);
	}

	// Write the image to the file
	fout << "P3\n" << width << " " << height << "\n" << maxColor << "\n";
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			fout << pixels[i][j].r << " " << pixels[i][j].g << " " << pixels[i][j].b << " ";
		}
		fout << "\n";
	}
	fout.close();
}

void Image::saveImage() {
	if (filename.empty()) {
		throw std::runtime_error("Filename is empty, cannot save image.");
	}
	saveImage(filename);
}

void Image::saveImage(const char* saveFile) {
	if (saveFile == nullptr || std::string(saveFile).empty()) {
		throw std::runtime_error("Filename is empty, cannot save image.");
	}
	std::string file = std::string(saveFile);
	saveImage(file);
}

