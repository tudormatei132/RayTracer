#include <iostream>
#include <SDL3/SDL.h>
#include <vector>
#include <thread>
#include <numbers>
#include <time.h>
#include "Image.h"
#include "Sphere.h"
#include "Vector3.h"
#include "Ray.h"

static SDL_Window * window = NULL;
static SDL_Renderer* renderer = NULL;
static SDL_Texture* texture = NULL;

Vector3 worldUp = Vector3(0, 1, 0), right = Vector3(0, 0, 0), up = Vector3(0, 0, 0);
double fov = 60.0;
int frameCount = 0;
size_t lastTick = 0;
#define WINDOW_WIDTH 600
#define WINDOW_HEIGHT 400

//void updateViewOMP(Image& img, const Vector3& camera, const Vector3& background, const std::vector<Sphere>& spheres) {
//
//    #pragma omp parallel for collapse(2) schedule(dynamic)
//    for (int i = 0; i < img.height; i++) {
//        for (int j = 0; j < img.width; j++) {
//            double u = double(j) / (img.width - 1);
//            double v = double(i) / (img.height - 1);
//
//            double x = (2 * u - 1) * img.aspectRatio;
//            double y = (1 - 2 * v);
//            Vector3 direction = (Vector3(x, y, -1) - camera);
//            direction.normalize();
//            Ray ray = Ray(camera, direction);
//            img.pixels[i][j] = Pixel(ray.colorPixel(spheres, background, cameraDir));
//            int index = (i * img.width + j) * 3;
//            img.pixelArray[index + 0] = static_cast<unsigned char>(img.pixels[i][j].r);
//            img.pixelArray[index + 1] = static_cast<unsigned char>(img.pixels[i][j].g);
//            img.pixelArray[index + 2] = static_cast<unsigned char>(img.pixels[i][j].b);
//
//        }
//    }
//}

void updateView(Image& img, const Vector3& camera, const Vector3& background, const std::vector<Sphere>& spheres, int start, int end, 
                const Vector3& cameraDir) {
    
    double viewHeight = 2.0 * tan((fov / 2.0) * std::numbers::pi / 180.0);
    double viewWidth = viewHeight * img.aspectRatio;
    Vector3 lowerLeftCorner = camera + cameraDir - right * (viewWidth / 2) - up * (viewHeight / 2);

    Vector3 horizontal = right * viewWidth;
    Vector3 vertical = up * viewHeight;


    for (int i = start; i < end; i++) {
        for (int j = 0; j < img.width; j++) {
            // screen coordinates
            double u = double(j) / (img.width - 1);
            double v = 1.0 - double(i) / (img.height - 1);


            Vector3 direction = (lowerLeftCorner + horizontal * u + vertical * v - camera);
            direction.normalize();
            Ray ray = Ray(camera, direction);
            img.pixels[i][j] = Pixel(ray.colorPixel(spheres, background, cameraDir, camera));
            int index = (i * img.width + j) * 3;
            img.pixelArray[index + 0] = static_cast<unsigned char>(img.pixels[i][j].r);
            img.pixelArray[index + 1] = static_cast<unsigned char>(img.pixels[i][j].g);
            img.pixelArray[index + 2] = static_cast<unsigned char>(img.pixels[i][j].b);

        }
    }
}

void updateViewParallel(Image& img, const Vector3& camera, const Vector3& background, const std::vector<Sphere>& spheres,
                        const Vector3& cameraDir) {
    int numThreads = std::thread::hardware_concurrency();
    std::vector<std::thread> threads;
    int partialSize = img.height / numThreads;
    right = (cameraDir * worldUp).normalized();
    up = (right * cameraDir).normalized();
    for (int i = 0; i < numThreads; i++) {
        int start = i * partialSize;
        int end;
        if (i == numThreads - 1) {
            end = img.height;
        }
        else {
            end = start + partialSize;
        }

        threads.emplace_back(updateView, std::ref(img), std::cref(camera), std::cref(background), std::cref(spheres), start, end, std::cref(cameraDir));

    }

    for (auto& t : threads) {
        t.join();
    }

}

Vector3 rotateCamera(const Vector3& cameraDir, const Vector3& axis, double angle) {
    double cosA = cos(angle);
    double sinA = sin(angle);

    return cameraDir * cosA + axis * (axis.dotProduct(cameraDir)) * (1 - cosA) + axis * cameraDir * sinA;

}


void displayImage(Image& image, Vector3& camera, Vector3& background, std::vector<Sphere>& spheres, Vector3& cameraDir) {

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << "\n";
        return;
    }



    if (!SDL_CreateWindowAndRenderer("Raytracer", WINDOW_WIDTH, WINDOW_HEIGHT, 0, &window, &renderer)) {
        SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
        return;
    }
    
    SDL_Texture* texture = SDL_CreateTexture(renderer,
        SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STREAMING,
        image.width, image.height);

    if (!texture) {
        std::cerr << "SDL_CreateTexture failed: " << SDL_GetError() << "\n";
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return;
    }

    if (image.pixelArray.size() != size_t(image.width * image.height * 3)) {
        std::cerr << "pixelArray size mismatch!\n";
        SDL_DestroyTexture(texture);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return;
    }

    if (!SDL_UpdateTexture(texture, nullptr, image.pixelArray.data(), image.width * 3)) {
        std::cerr << "SDL_UpdateTexture failed: " << SDL_GetError() << "\n";
    }

    SDL_RenderClear(renderer);
    SDL_RenderTexture(renderer, texture, nullptr, nullptr);
    SDL_RenderPresent(renderer);
    
    SDL_Event event;
    bool running = true;
    while (running) {
        frameCount++;
        double angle = 0.05;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            }
            if (event.type == SDL_EVENT_KEY_DOWN) {
                switch (event.key.scancode) {
                    
                    case SDL_SCANCODE_W: {
                        camera.add(cameraDir * 0.5);
                        break;
                    }
                    

                    case SDL_SCANCODE_S: {
                        camera.add(cameraDir * -0.5);
                        break;
                    }
                
                    case SDL_SCANCODE_A: {
                        cameraDir = rotateCamera(cameraDir, worldUp, angle);
                        break;
                    }

                    case SDL_SCANCODE_D: {
                        cameraDir = rotateCamera(cameraDir, worldUp, -angle);
                        break;
                    }
                    // TODO: limit the rotation angle for up/down rotations
                    case SDL_SCANCODE_UP: {
                        cameraDir = rotateCamera(cameraDir, right, angle);
                        break;
                    }

                    case SDL_SCANCODE_DOWN: {
                        cameraDir = rotateCamera(cameraDir, right, -angle);
                        break;
                    }

                    case SDL_SCANCODE_U: {
                        spheres[0].center.add(Vector3(0, 1, 0));
                        break;
                    }
                    
                    case SDL_SCANCODE_I: {
                        spheres[0].center.add(Vector3(0, -1, 0));
                        break;
                    }

                    default: {
                        SDL_Log("Not a valid input");
                    }

                }
                
                
            }
        }
        SDL_Delay(1);  
        /*if (frameCount % 20 == 0) {
            size_t thisTick = SDL_GetTicks();
            double fps = ((double)(frameCount) / 1000.0) / ((double)(thisTick - lastTick));
            lastTick = thisTick;
            frameCount = 0;
            SDL_Log("%f", fps);
        }*/
        updateViewParallel(image, camera, background, spheres, cameraDir);
        SDL_UpdateTexture(texture, nullptr, image.pixelArray.data(), image.width * 3);
        SDL_RenderClear(renderer);
        SDL_RenderTexture(renderer, texture, nullptr, nullptr);
        SDL_RenderPresent(renderer);


        size_t thisTick = SDL_GetTicks();
        double fps = (1000.0 / (thisTick - lastTick));
        lastTick = thisTick;
        SDL_Log("%f", fps);
    }

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}





int main() {
    Image img(WINDOW_WIDTH, WINDOW_HEIGHT, 255, "test.ppm");

    std::vector<Sphere> spheres;
    spheres.push_back(Sphere(Vector3(0, 0, -6.0), 2.0, Vector3(255, 0, 0)));
    spheres.push_back(Sphere(Vector3(3.0, 1.0, -6.0), 1.0, Vector3(0, 255, 0)));
    spheres.push_back(Sphere(Vector3(2.0, 2.0, -7.0), 2.0, Vector3(0, 0, 255)));
    Vector3 camera = Vector3(0, 0, 0);
    Vector3 cameraDir = Vector3(0, 0, -1).normalized();
    Vector3 background = Vector3(0, 0, 0);

    updateViewParallel(img, camera, background, spheres, cameraDir);

    img.saveImage("result.ppm");

    displayImage(img, camera, background, spheres, cameraDir);


    return 0;
}
