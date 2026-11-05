#pragma once
#include <cmath>
#include "hittable.h"

class sphere : public hittable {
public:
	sphere(const vec3& center, float radius, const vec3& albedo, float shine = 0.0f)
		: center(center), radius(radius), albedo(albedo), shine(shine) {
	}

	bool hit(const ray& r, float t_min, float t_max, hit_record& rec) const override
	{
		vec3 oc = r.origin() - center;
		float a = dot(r.direction(), r.direction());
		float b = 2.0f * dot(oc, r.direction());
		float c = dot(oc, oc) - radius * radius;

		float discriminant = b * b - 4.0f * a * c;
		if (discriminant < 0.0f)
		{
			return false;   // miss
		}

		// Try the near hit first then the far one.
		float sqrtD = std::sqrt(discriminant);
		float t = (-b - sqrtD) / (2.0f * a);
		if(t <= t_min || t >= t_max)
		{
			t = (-b + sqrtD) / (2.0f * a);
			if (t <= t_min || t >= t_max)
			{
				return false; // miss  Both outside the windows
			}
		}

		rec.t = t;
		rec.p = r.at(t);
		vec3 outward_normal = (rec.p - center) / radius;
		rec.set_face_normal(r, outward_normal);
		rec.albedo = albedo;
		rec.shine = shine;
		return true;
	}

private:
	vec3 center;
	float radius;
	vec3 albedo;
	float shine;
};