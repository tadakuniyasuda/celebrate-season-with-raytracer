#pragma once
#include <cmath>

class vec3 {
public:
    float e[3];

    // The book's version leaves e[] uninitialised, so a vec3 you forgot to set holds random garbage. 
    // Starting from zero costs nothing and prevents a whole class of strange-looking bugs.
    vec3() : e{ 0.0f, 0.0f, 0.0f } {}
    vec3(float x, float y, float z) : e{ x, y, z } {}

    // Same data, two vocabularies: position or colour.
    float x() const { return e[0]; }
    float y() const { return e[1]; }
    float z() const { return e[2]; }
    float r() const { return e[0]; }
    float g() const { return e[1]; }
    float b() const { return e[2]; }

    vec3& operator+=(const vec3& v) {
        e[0] += v.e[0]; e[1] += v.e[1]; e[2] += v.e[2];
        return *this;
    }

    float length() const {
        return std::sqrt(e[0] * e[0] + e[1] * e[1] + e[2] * e[2]);
    }
};

// Without inline, each file gets its own copy of operator+, and the linker fails with "already defined".
// const vec3& avoids copying.It passes a read, only reference instead of a copy. 
// For 12 bytes the speed difference is tiny, but it's the C++ habit I see everywhere in engine code.

inline vec3 operator+(const vec3& a, const vec3& b) {
    return vec3(a.e[0] + b.e[0], a.e[1] + b.e[1], a.e[2] + b.e[2]);
}

inline vec3 operator-(const vec3& a, const vec3& b) {
    return vec3(a.e[0] - b.e[0], a.e[1] - b.e[1], a.e[2] - b.e[2]);
}

// t * v and v * t are two separate functions. 
// C++ doesn't assume multiplication works both ways, so I define one and have the other call it.
inline vec3 operator*(float t, const vec3& v) {
    return vec3(t * v.e[0], t * v.e[1], t * v.e[2]);
}

inline vec3 operator*(const vec3& v, float t) { return t * v; }

inline vec3 operator/(const vec3& v, float t) { return (1.0f / t) * v; }

inline float dot(const vec3& a, const vec3& b) {
    return a.e[0] * b.e[0] + a.e[1] * b.e[1] + a.e[2] * b.e[2];
}

// For unit vectors, it gives 1 when they point the same way, 
// 0 when they're perpendicular, and −1 when they're opposite.
inline vec3 unit_vector(const vec3& v) { return v / v.length(); }