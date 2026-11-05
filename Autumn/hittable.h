// hittable.h
#pragma once
#include "ray.h"

// Everything we learn about one hit: like Unity's raycastHit.  
// The ray's t, the point of intersection, the normal at that point, and the albedo (colour) of the object hit.
struct hit_record {
	vec3 p;				// Where the ray hit
	vec3 normal;		// Surface direction, always facing the ray
	float t = 0.0f; 		// How far along the ray the hit was
	bool front_face = false; // Did the ray hit the front face of the object?
	vec3 albedo;			// the surface color (temporary)
	float shine = 0.0f;   // 0 = matte, 1 = shiny

	void set_face_normal(const ray& r, const vec3& outward_normal) {
		front_face = dot(r.direction(), outward_normal) < 0.0f;
		normal = front_face ? outward_normal : -outward_normal;
	}
};

// the one question every object must answer: does this ray hit me?
class hittable {
public : 
	virtual ~hittable() = default;
	virtual bool hit(const ray& r, float t_min, float t_max, hit_record& rec) const = 0;
};