#pragma once
// camera.h
#pragma once
#include <algorithm>
#include "vec3.h"
#include "hittable.h"
#include <Vector>


class camera {
public:
    int width = 800;
    int height = 450;

    std::vector<vec3> render(const hittable& world) const
    {
        // Viewport size comes from the image's aspect ratio.
        float viewport_height = 2.0f;
        float viewport_width = viewport_height * float(width) / float(height);

        vec3 origin(0.0f, 0.0f, 0.0f);
        vec3 horizontal(viewport_width, 0.0f, 0.0f);
        vec3 vertical(0.0f, viewport_height, 0.0f);
        vec3 lower_left = origin - horizontal / 2.0f - vertical / 2.0f
            - vec3(0.0f, 0.0f, 1.0f);

        std::vector<vec3> image(width * height);
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                float u = float(x) / float(width - 1);
                float v = float(height - 1 - y) / float(height - 1);

                ray r(origin, lower_left + u * horizontal + v * vertical - origin);
                image[y * width + x] = ray_color(r, world);
            }
        }
        return image;
    }
private:
    vec3 ray_color(const ray& r, const hittable& world) const
    {
        vec3 sunDir = unit_vector(vec3(-1.0f, 0.45f, 0.35f));  // low evening sun, front-left
        vec3 sunColor(1.0f, 0.85f, 0.6f);                       // warm sunlight
        vec3 ambient(0.22f, 0.20f, 0.30f);                      // cool light from the sky

        hit_record rec;
        if (world.hit(r, 0.001f, 1e30f, rec)) {
            // Shadow ray: is anything between this point and the sun?
            ray shadowRay(rec.p, sunDir);
            hit_record blocker;
            bool inShadow = world.hit(shadowRay, 0.001f, 1e30f, blocker);

            float diffuse = inShadow ? 0.0f : std::max(0.0f, dot(rec.normal, sunDir));

            // Highlight (Blinn-Phong)
            vec3 view = -unit_vector(r.direction());
            vec3 halfway = unit_vector(sunDir + view);
            float spec = inShadow ? 0.0f
                : std::pow(std::max(0.0f, dot(rec.normal, halfway)), 60.0f) * rec.shine;

            return ambient * rec.albedo + diffuse * (sunColor * rec.albedo) + spec * sunColor;
        }

        vec3 dir = unit_vector(r.direction());
        float t = 0.5f * (dir.y() + 1.0f);
        vec3 horizon(1.0f, 0.68f, 0.38f);
        vec3 top(0.85f, 0.40f, 0.35f);
        return (1.0f - t) * horizon + t * top;
    }
};