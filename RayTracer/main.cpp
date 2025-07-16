#include <iostream>
#include <SDL3/SDL.h>
#include <vector>
#include <thread>
#include "Image.h"
#include "Sphere.h"
#include "Vector3.h"
#include "Ray.h"

static SDL_Window * window = NULL;
static SDL_Renderer* renderer = NULL;
static SDL_Texture* texture = NULL;



#define WINDOW_WIDTH 400
#define WINDOW_HEIGHT 300

void updateViewOMP(Image& img, const Vector3& camera, const Vector3& background, const std::vector<Sphere>& spheres) {
    #pragma omp parallel for collapse(2) schedule(dynamic)
    for (int i = 0; i < img.height; i++) {
        for (int j = 0; j < img.width; j++) {
            double u = double(j) / (img.width - 1);
            double v = double(i) / (img.height - 1);

            double x = (2 * u - 1) * img.aspectRatio;
            double y = (1 - 2 * v);
            Vector3 direction = (Vector3(x, y, -1) - camera);
            direction.normalize();
            Ray ray = Ray(camera, direction);
            img.pixels[i][j] = Pixel(ray.colorPixel(spheres, background));
            int index = (i * img.width + j) * 3;
            img.pixelArray[index + 0] = static_cast<unsigned char>(img.pixels[i][j].r);
            img.pixelArray[index + 1] = static_cast<unsigned char>(img.pixels[i][j].g);
            img.pixelArray[index + 2] = static_cast<unsigned char>(img.pixels[i][j].b);

        }
    }
}

void updateView(Image& img, const Vector3& camera, const Vector3& background, const std::vector<Sphere>& spheres, int start, int end) {
    
    for (int i = start; i < end; i++) {
        for (int j = 0; j < img.width; j++) {
            double u = double(j) / (img.width - 1);
            double v = double(i) / (img.height - 1);

            double x = (2 * u - 1) * img.aspectRatio;
            double y = (1 - 2 * v);
            Vector3 direction = (Vector3(x, y, -1) - camera);
            direction.normalize();
            Ray ray = Ray(camera, direction);
            img.pixels[i][j] = Pixel(ray.colorPixel(spheres, background));
            int index = (i * img.width + j) * 3;
            img.pixelArray[index + 0] = static_cast<unsigned char>(img.pixels[i][j].r);
            img.pixelArray[index + 1] = static_cast<unsigned char>(img.pixels[i][j].g);
            img.pixelArray[index + 2] = static_cast<unsigned char>(img.pixels[i][j].b);

        }
    }
}

void updateViewParallel(Image& img, const Vector3& camera, const Vector3& background, const std::vector<Sphere>& spheres) {
    int numThreads = std::thread::hardware_concurrency();
    std::vector<std::thread> threads;
    int partialSize = img.height / numThreads;

    for (int i = 0; i < numThreads; i++) {
        int start = i * partialSize;
        int end;
        if (i == numThreads) {
            end = img.height;
        }
        else {
            end = start + partialSize;
        }

        threads.emplace_back(updateView, std::ref(img), std::cref(camera), std::cref(background), std::cref(spheres), start, end);

    }

    for (auto& t : threads) {
        t.join();
    }

}

void displayImage(Image& image, Vector3& camera, Vector3& background, std::vector<Sphere>& spheres) {

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
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            }
            if (event.type == SDL_EVENT_KEY_DOWN) {
                switch (event.key.scancode) {
                    
                    case SDL_SCANCODE_W: {
                        camera.y += 0.1;
                        break;
                    }
                    

                    case SDL_SCANCODE_S: {
                        camera.y -= 0.1;
                        break;
                    }
                
                    case SDL_SCANCODE_A: {
                        camera.x -= 0.1;
                        break;
                    }

                    case SDL_SCANCODE_D: {
                        camera.x += 0.1;
                        break;
                    }
                    
                    default: {
                        SDL_Log("Not a valid input");
                    }

                }
                updateViewOMP(image, camera, background, spheres);
                SDL_UpdateTexture(texture, nullptr, image.pixelArray.data(), image.width * 3);
                SDL_RenderClear(renderer);
                SDL_RenderTexture(renderer, texture, nullptr, nullptr);
                SDL_RenderPresent(renderer);
            }
        }
        SDL_Delay(1);  
    }

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}





int main() {
    Image img(WINDOW_WIDTH, WINDOW_HEIGHT, 255, "test.ppm");

    std::vector<Sphere> spheres;
    spheres.push_back(Sphere(Vector3(0.0, 0.0, -5.0), 1.0, Vector3(255, 0, 0)));
    spheres.push_back(Sphere(Vector3(3.0, 0.0, -6.0), 1.0, Vector3(0, 255, 0)));

    Vector3 camera = Vector3(0, 0, 0);
    Vector3 background = Vector3(0, 0, 0);

    updateViewParallel(img, camera, background, spheres);

    img.saveImage("result.ppm");

    displayImage(img, camera, background, spheres);


    return 0;
}
