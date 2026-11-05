// main.cpp
#include <memory>
#include "hittable_list.h"
#include "sphere.h"
#include "camera.h"
#include "display.h"
#include <random>
#include <cmath>

int main()
{
    // Colours
    vec3 ground(0.42f, 0.17f, 0.15f);   // brown earth
    vec3 persimmon(0.95f, 0.38f, 0.06f);   // 柿 orange
    vec3 calyx(0.12f, 0.30f, 0.10f);   // ヘタ green
    vec3 chestnut(0.45f, 0.22f, 0.10f);   // 栗 brown

    hittable_list world;

    // The ground: a huge sphere whose top is at y = -0.5
    world.add(std::make_shared<sphere>(vec3(0.0f, -100.5f, -1.0f), 100.0f, ground));

    // Big persimmon in the middle, with its calyx on top
    world.add(std::make_shared<sphere>(vec3(0.0f, -0.0f, -1.2f), 0.5f, persimmon));
    world.add(std::make_shared<sphere>(vec3(0.0f, 0.53f, -1.2f), 0.08f, calyx));

    // Smaller persimmon behind, on the left
    world.add(std::make_shared<sphere>(vec3(-0.9f, -0.2f, -1.7f), 0.3f, persimmon));
    world.add(std::make_shared<sphere>(vec3(-0.9f, 0.12f, -1.7f), 0.05f, calyx));

    // Chestnut in front, on the right
    world.add(std::make_shared<sphere>(vec3(0.75f, -0.3f, -1.1f), 0.2f, chestnut));
    // Fallen leaves: small spheres sunk into the ground
    //std::mt19937 rng(42);
    //std::uniform_real_distribution<float> rnd(0.0f, 1.0f);
    //vec3 leafColors[] = { vec3(0.85f, 0.15f, 0.05f), vec3(0.95f, 0.45f, 0.05f),
    //                      vec3(0.90f, 0.70f, 0.10f), vec3(0.60f, 0.15f, 0.05f) };
    //for (int i = 0; i < 120; i++) {
    //    float x = -3.0f + 6.0f * rnd(rng);
    //    float z = -4.0f + 3.0f * rnd(rng);
    //    float r = 0.04f + 0.05f * rnd(rng);
    //    float dz = z + 1.0f;
    //    float groundY = -100.5f + std::sqrt(100.0f * 100.0f - x * x - dz * dz);  // ground height here
    //    world.add(std::make_shared<sphere>(vec3(x, groundY - r * 0.5f, z), r, leafColors[i % 4]));
    //}

    // Fallen leaves: lots of small spheres sunk into the ground
    const int LEAVES = 800;
    std::mt19937 rng(42);
    std::uniform_real_distribution<float> rnd(0.0f, 1.0f);
    vec3 leafColors[] = { vec3(0.85f, 0.15f, 0.05f), vec3(0.95f, 0.45f, 0.05f),
                          vec3(0.90f, 0.70f, 0.10f), vec3(0.60f, 0.15f, 0.05f),
                          vec3(0.75f, 0.30f, 0.05f) };
    int placed = 0;
    while (placed < LEAVES) {
        float x = -3.5f + 7.0f * rnd(rng);
        float z = -5.0f + 4.75f * rnd(rng);
        float r = 0.03f + 0.04f * rnd(rng);

        // Skip spots under the persimmons and the chestnut
        float d1 = std::sqrt(x * x + (z + 1.2f) * (z + 1.2f));
        float d2 = std::sqrt((x + 0.9f) * (x + 0.9f) + (z + 1.7f) * (z + 1.7f));
        float d3 = std::sqrt((x - 0.75f) * (x - 0.75f) + (z + 1.1f) * (z + 1.1f));
        if (d1 < 0.45f || d2 < 0.28f || d3 < 0.18f) continue;

        float dz = z + 1.0f;
        float groundY = -100.5f + std::sqrt(100.0f * 100.0f - x * x - dz * dz);
        vec3 color = leafColors[int(rnd(rng) * 5.0f) % 5];
        world.add(std::make_shared<sphere>(vec3(x, groundY - r * 0.5f, z), r, color));
        placed++;
    }

    camera cam;
    std::vector<vec3> image = cam.render(world);

    show_image("Autumn Ray Tracer - Part 1", image, cam.width, cam.height);
    return 0;
}