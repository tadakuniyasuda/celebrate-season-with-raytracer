#include <vector>
#include "vec3.h"
#include "ray.h"
#include "display.h"
#include <algorithm>

// Return the distance t to the nearest hit or -1 if the ray misses.
float hit_sphere(const vec3& center, float radius, const ray& r)
{
	vec3 oc = r.origin() - center;		// A - C

	float a = dot(r.direction(), r.direction());
	float b = 2.0f * dot(oc, r.direction());
	float c = dot(oc, oc) - radius * radius;

	float discriminant = b * b - 4 * a * c;
	if (discriminant < 0.0f)
	{
		return -1.0f;
	}

	float sqrtD = std::sqrt(discriminant);
	float t1 = (-b - sqrtD) / (2.0f * a);
	float t2 = (-b + sqrtD) / (2.0f * a);

	if (t1 > 0.001f) return t1;
	if (t2 > 0.001f) return t2;
	return -1.0f;
}


vec3 sky_color(const ray& r)
{
	vec3 dir = unit_vector(r.direction());

	float t = 0.5f * (dir.y() + 1.0f);

	vec3 horizon(0.60f, 0.15f, 0.48f);	// magenta
	vec3 night(0.09f, 0.03f, 0.22f);		// deep violet

	return (1.0f - t) * horizon + t * night;
}

vec3 ray_color(const ray& r)
{
	vec3 center(0.0f, 0.0f, -1.0f);
	float radius = 0.5f;

	float t = hit_sphere(center, radius, r);

	if (t > 0.0f)
	{


		vec3 hitPoint = r.at(t);
		vec3 normal = (hitPoint - center) / radius;

		float x = normal.x();
		float y = normal.y();

		// Width shrinks as y goes up -> a triangle pointing up.
		float eyeWidth = 0.45f * (0.45f - y);

		bool leftEye = y > 0.10f && y < 0.45f && std::fabs(x + 0.35f) < eyeWidth;
		bool rightEye = y > 0.10f && y < 0.45f && std::fabs(x - 0.35f) < eyeWidth;

		// A U-shaped curve: lowest at x = 0, rising toward the corners.
		float smileCurve = -0.28f + 0.9f * x * x;

		bool mouth = std::fabs(x) < 0.42f &&
			y > -0.35f && y < -0.05f &&
			std::fabs(y - smileCurve) < 0.04f;

		bool face = normal.z() > 0.0f && (leftEye || rightEye || mouth);
		if (face)
			return vec3(1.0f, 0.92f, 0.35f);    // candle yellow

		// light
		vec3 lightDir = unit_vector(vec3(-1.0f, 1.0f, 1.0f)); // upper left, in front

		float diffuse = std::max(0.0f, dot(normal, lightDir));
		float brightness = 0.20f + 0.80f * diffuse;

		vec3 pumpkin(1.0f, 0.35f, 0.05f);
		return brightness * pumpkin;

	}

	return sky_color(r);
}

int main()
{
	const int width = 800;
	const int height = 600;

	// Camera
	vec3 origin		(0.0f, 0.0f, 0.0f);
	vec3 lower_left	(-2.0f, -1.5f, -1.0f);
	vec3 horizontal(4.0f, 0.0f, 0.0f);
	vec3 vertical(0.0f, 3.0f, 0.0f);

	// Render into a buffer first, show it afterwords
	std::vector<vec3> image(width * height);

	for (int y = 0; y < height; y++)
		for (int x = 0; x < width; x++)		
		{
			float u = float(x) / float(width - 1);
			float v = float(height - 1 - y) / float(height - 1);

			vec3 direction = lower_left + u * horizontal + v * vertical - origin;
			ray r(origin, direction);

			image[y * width + x] = ray_color(r);
		}

	show_image("Halloween Ray Tracer - Part2", image, width, height);
	return 0;
}