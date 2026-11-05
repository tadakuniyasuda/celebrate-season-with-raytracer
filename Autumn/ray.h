// ray.h
#pragma once
#include "vec3.h"

class ray {
public:
    ray() {}
    ray(const vec3& origin, const vec3& direction)
        : orig(origin), dir(direction) {
    }

    vec3 origin() const { return orig; }
    vec3 direction() const { return dir; }

    // The point at distance t along the ray.
    vec3 at(float t) const { return orig + t * dir; }

private:
    vec3 orig;
    vec3 dir;
};