#pragma once

// SDL_MAIN_HANDLED tells SDL not to replace your main() with its own entry point. 
// Without it, Visual Studio gives a linker error about SDL_main.
#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <algorithm>
#include <cstdint>
#include <vector>
#include "vec3.h"

// 0..1 float -> 0..255 byte (values outside 0..1 are clamped)
inline uint32_t to_byte(float c) {
    return uint32_t(255.99f * std::clamp(c, 0.0f, 1.0f));
}

inline void show_image(const char* title,
    const std::vector<vec3>& image,
    int width, int height)
{
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow(title,
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        width, height, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    SDL_Texture* texture = SDL_CreateTexture(renderer,
        SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC,
        width, height);

    // Pack each pixel into 0xAARRGGBB.
    std::vector<uint32_t> pixels(width * height);
    for (int i = 0; i < width * height; i++) {
        pixels[i] = (255u << 24)
            | (to_byte(image[i].r()) << 16)
            | (to_byte(image[i].g()) << 8)
            | to_byte(image[i].b());
    }
    SDL_UpdateTexture(texture, nullptr, pixels.data(),
        width * sizeof(uint32_t));

    bool running = true;
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = false;
        }
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, nullptr, nullptr);
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}