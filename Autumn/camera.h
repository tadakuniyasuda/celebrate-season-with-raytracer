// camera.h
//  world（物体のリスト）を受け取って、画像（色の配列）を返す
//    main()       → 「何を置くか」だけ
//    camera       → 「どう撮るか」
//    display.h    → 「どう表示するか」
// ============================================================
#pragma once
#include <algorithm>   // std::max
#include <cmath>       // std::pow
#include "vec3.h"
#include "hittable.h"
#include <Vector>


class camera {
public:
    int width = 800;
    int height = 450;

    std::vector<vec3> render(const hittable& world) const
    {
        // ratioから窓を計算
        float viewport_height = 2.0f;
        float viewport_width = viewport_height * float(width) / float(height);

        vec3 origin(0.0f, 0.0f, 0.0f);                                  // 目の位置
        vec3 horizontal(viewport_width, 0.0f, 0.0f);                    // 窓の幅
        vec3 vertical(0.0f, viewport_height, 0.0f);                     // 窓の高さ
        // 窓の左下の角: 中心から半分左・半分下、そして 1 前（ - z）
        vec3 lower_left = origin - horizontal / 2.0f - vertical / 2.0f
            - vec3(0.0f, 0.0f, 1.0f);

        // rayを全ピクセル飛ばす
        std::vector<vec3> image(width * height);
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                // ピクセル位置を0から１(vは上下反転)
                float u = float(x) / float(width - 1);
                float v = float(height - 1 - y) / float(height - 1);
                // 目から窓のこの位置を通るレイ
                ray r(origin, lower_left + u * horizontal + v * vertical - origin);
                // 2dのx,y →　1Dの番号 (y * 幅 +x)
                image[y * width + x] = ray_color(r, world);
            }
        }
        return image;
    }
private:
    // レイが何色をみるのか
    vec3 ray_color(const ray& r, const hittable& world) const
    {
        vec3 sunDir = unit_vector(vec3(-1.0f, 0.45f, 0.35f));  // 太陽の方向
        vec3 sunColor(1.0f, 0.85f, 0.6f);                       // 夕日の色
        vec3 ambient(0.22f, 0.20f, 0.30f);                      // 環境の光

        hit_record rec;
        if (world.hit(r, 0.001f, 1e30f, rec)) {
            // ---- 影レイ ----
            // 当たった点から太陽に向けて、もう1本レイを飛ばす
            // 途中で何かに当たる = 太陽が遮られている = 影の中
            ray shadowRay(rec.p, sunDir);
            hit_record blocker;
            bool inShadow = world.hit(shadowRay, 0.001f, 1e30f, blocker);

            // lambert
            float diffuse = inShadow ? 0.0f : std::max(0.0f, dot(rec.normal, sunDir));

            // Blinn-Phong
            vec3 view = -unit_vector(r.direction());
            vec3 halfway = unit_vector(sunDir + view);
            float spec = inShadow ? 0.0f
                : std::pow(std::max(0.0f, dot(rec.normal, halfway)), 60.0f) * rec.shine;

            // ---- 3つの光を足す ----
            //   環境光 × 表面の色      … 影でも真っ黒にならない
            // + 太陽光 × 表面の色 × 向き … 日の当たる面
            // + ハイライト × 太陽の色    … ツヤ（表面の色は掛けない = 光そのものの色）
            return ambient * rec.albedo + diffuse * (sunColor * rec.albedo) + spec * sunColor;
        }

        // ---- 何にも当たらなかった: 空 ----
        // 2色のgradient
        vec3 dir = unit_vector(r.direction());
        float t = 0.5f * (dir.y() + 1.0f);
        vec3 horizon(1.0f, 0.68f, 0.38f);
        vec3 top(0.85f, 0.40f, 0.35f);
        return (1.0f - t) * horizon + t * top;
    }
};