#include <vector>
#include "vec3.h"
#include "ray.h"
#include "display.h"
#include <algorithm>
#include <iostream>
#include <chrono>
#include <iostream>

struct Shade {
	vec3 color;
	vec3 glow;
};

// 火っとの最短距離tか、-1を返す。
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

	vec3 horizon(0.60f, 0.15f, 0.48f);		// 紫
	vec3 night(0.09f, 0.03f, 0.22f);		// 濃い紫

	return (1.0f - t) * horizon + t * night;
}

Shade ray_color(const ray& r)
{
	vec3 center(0.0f, 0.0f, -1.0f);
	float radius = 0.5f;

	float t = hit_sphere(center, radius, r);
	if (t <= 0.0f)                                  // 当たらない場合
	{
		vec3 dir = unit_vector(r.direction());
		vec3 moonDir = unit_vector(vec3(0.3f, 0.3f, -0.3f));

		if (dot(dir, moonDir) > 0.995f)
		{
			vec3 moon(1.0f, 0.95f, 0.0f);
			return { moon, moon };                  // 月 と bloom用
		}
		return { sky_color(r), vec3() };            // 空と無
	}

	if (t > 0.0f)									//当たる場合
	{


		vec3 hitPoint = r.at(t);
		vec3 normal = (hitPoint - center) / radius;

		float x = normal.x();
		float y = normal.y();

		// yが上がれば幅が狭くなる
		float eyeWidth = 0.45f * (0.45f - y);

		bool leftEye = y > 0.10f && y < 0.45f && std::fabs(x + 0.35f) < eyeWidth;
		bool rightEye = y > 0.10f && y < 0.45f && std::fabs(x - 0.35f) < eyeWidth;

		// U-型カーブ　(x = 0で一番下)
		float smileCurve = -0.28f + 0.9f * x * x;

		bool mouth = std::fabs(x) < 0.42f &&
			y > -0.35f && y < -0.05f &&
			std::fabs(y - smileCurve) < 0.04f;

		bool face = normal.z() > 0.0f && (leftEye || rightEye || mouth);
		if (face)
		{

			vec3 candle(1.0f, 0.92f, 0.35f);    // 　黄色
			return { candle, vec3(1.0f, 0.45f, 0.1f) };

		}

		

		//　光を追加
		vec3 lightDir = unit_vector(vec3(0.0f, -1.0f, 0.0f)); 

		float diffuse = std::max(0.0f, dot(normal, lightDir));
		float brightness = 0.20f + 0.80f * diffuse;

		vec3 pumpkin(1.0f, 0.35f, 0.05f);
		return { brightness * pumpkin, vec3() };

	}

	return { sky_color(r), vec3() };
}
// ウェイト radius=3 * sigma, d=0 
std::vector<float> gaussian_weights(float sigma)
{
	int radius = int(std::ceil(3.0f * sigma));
	//radius = 3 * sigma;
	// 自分自身を含め(radius + 1)個のスロット
	std::vector<float> w(radius + 1);
	for (int d = 0; d <= radius; d++)
		w[d] = std::exp(-(d * d) / (2.0f * sigma * sigma));
	return w;
}

// 1D ブラー横、縦の順でブラー
std::vector<vec3> blur_pass(const std::vector<vec3>& src,
	int width, int height,
	const std::vector<float>& weights,
	bool horizontal)
{
	const int radius = int(weights.size()) - 1;
	std::vector<vec3> dst(width * height);

	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			vec3 sum;
			float total = 0.0f;

			for (int d = -radius; d <= radius; d++) {
				int sx = horizontal ? x + d : x;
				int sy = horizontal ? y : y + d;
				if (sx < 0 || sx >= width || sy < 0 || sy >= height)
					continue;                     // imageの外: skip

				float w = weights[std::abs(d)];
				sum += src[sy * width + sx] * w;
				total += w;
			}
			dst[y * width + x] = sum / total;
		}
	}
	return dst;
}

// Direct 2D Gaussian: (2r+1) x (2r+1) squareを各pixelが計算
std::vector<vec3> blur_2d(const std::vector<vec3>& src,
	int width, int height,
	const std::vector<float>& weights)
{
	const int radius = int(weights.size()) - 1;
	std::vector<vec3> dst(width * height);

	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			vec3 sum;
			float total = 0.0f;

			for (int dy = -radius; dy <= radius; dy++) {
				int sy = y + dy;
				if (sy < 0 || sy >= height) continue;

				for (int dx = -radius; dx <= radius; dx++) {
					int sx = x + dx;
					if (sx < 0 || sx >= width) continue;

					// 2D ウェイト(horizontal weight x vertical weight)
					float w = weights[std::abs(dx)] * weights[std::abs(dy)];
					sum += src[sy * width + sx] * w;
					total += w;
				}
			}
			dst[y * width + x] = sum / total;
		}
	}
	return dst;
}

int main()
{
	const int width = 800;
	const int height = 600;

	// カメラ
	vec3 origin		(0.0f, 0.0f, 0.0f);
	vec3 lower_left	(-2.0f, -1.5f, -1.0f);
	vec3 horizontal(4.0f, 0.0f, 0.0f);
	vec3 vertical(0.0f, 3.0f, 0.0f);

	// bufferにレンダーし、最後に表示
	std::vector<vec3> image(width * height);
	std::vector<vec3> glow(width * height);
	for (int y = 0; y < height; y++)
		for (int x = 0; x < width; x++)		
		{
			float u = float(x) / float(width - 1);
			float v = float(height - 1 - y) / float(height - 1);

			vec3 direction = lower_left + u * horizontal + v * vertical - origin;
			ray r(origin, direction);

			Shade s = ray_color(r);

			image[y * width + x] = s.color;
			glow[y * width + x] = s.glow;
		}
	std::vector<float> weights = gaussian_weights(12.0f);
	std::cout << "radius = " << weights.size() - 1 << "\n"; 

	std::vector<vec3> blurred = blur_pass(glow, width, height, weights, true);
	blurred = blur_pass(blurred, width, height, weights, false);

	// bloom追加
	const float bloomStrength = 1.5f;
	for (int i = 0; i < width * height; i++)
		image[i] += blurred[i] * bloomStrength;
	show_image("Halloween Ray Tracer - Part2", image, width, height);

	return 0;
}