#include <vector>
#include "vec3.h"
#include "ray.h"
#include "display.h"

vec3 sky_color(const ray& r)
{
	vec3 dir = unit_vector(r.direction());


	 Moon (does this ray point (almost) straight at the moon?)
	vec3 moonDir = unit_vector(vec3(0.3f, 0.3f, -.3f));
	if (dot(dir, moonDir) > 0.995f)
	{
		return vec3(1.0f, 0.95f, 0.7f);   // pale yellow
	}


	float t = 0.5f * (dir.y() + 1.0f);

	vec3 horizon(0.60f, 0.15f, 0.48f);	// magenta
	vec3 night(0.09f, 0.03f, 0.22f);		// deep violet

	//vec3 horizon(1.0, 0.4, 0.0f);		// orange
	//vec3 night(0.35, 0.0f, 0.3);		// dark blue

	return (1.0f - t) * horizon + t * night;
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

			image[y * width + x] = sky_color(r);
		}

	show_image("Halloween Ray Tracer - Part1", image, width, height);
	return 0;
}