// =============================================================================
// Autumn Ray Tracer - Part 1
// This program renders a simple autumn scene with persimmons, a chestnut, and fallen leaves.
// 
// 付随ファイル
// vec3.h / ray.h   - 基本の部分(Vector and ray)
// hittable.ht      - 当たるものの共通ルール
// sphere.h         - 球
// hittalbe_list.h  - 物体のリスト(一番手前のものを返す)
// camera.h		    - レイを飛ばして画像を作る
// display.h        - 画像をウィンドウズに表示する
// 
// =============================================================================
#include <memory>           // std::make_shared
#include "hittable_list.h"  
#include "sphere.h"
#include "camera.h"
#include "display.h"
#include <random>           // 落ち葉をランダムに配置するため
#include <cmath>            // std::sqrt

//　描画計測
#include <chrono>
#include <iostream>

int main()
{
	// Colours(r,g,b) are in the range [0.0, 1.0]
    vec3 ground(0.42f, 0.17f, 0.15f);   // brown earth
    vec3 persimmon(0.95f, 0.38f, 0.06f);   // 柿 orange
    vec3 calyx(0.12f, 0.30f, 0.10f);   // ヘタ green
    vec3 chestnut(0.45f, 0.22f, 0.10f);   // 栗 brown

    hittable_list world;    // シーンの物体を全部入れるリスト

    // ---- 柿と栗 ---- 
	// 地面に「乗せる」:中心のy = -0.5 + 半径
    world.add(std::make_shared<sphere>(vec3(0.0f, -100.5f, -1.0f), 100.0f, ground));

    // Big persimmon in the middle, with its calyx on top
    // 最後の数字はshine(ツヤ) : 下記は少し、栗はツヤツヤ
    // std::make_shared<sphere>(...): 球を作って、そのポインタを返す
    world.add(std::make_shared<sphere>(vec3(0.0f, -0.0f, -1.2f), 0.5f, persimmon));     // 大きい柿(r=0.5f, y = 0.0)
    world.add(std::make_shared<sphere>(vec3(0.0f, 0.53f, -1.2f), 0.08f, calyx));        // そのヘタ(てっぺんに重なる)

    // Smaller persimmon behind, on the left
	world.add(std::make_shared<sphere>(vec3(-0.9f, -0.2f, -1.7f), 0.3f, persimmon));    // 奥の小さい柿(r=0.3f, y = -0.2)
	world.add(std::make_shared<sphere>(vec3(-0.9f, 0.12f, -1.7f), 0.05f, calyx));       // そのヘタ

    // Chestnut in front, on the right
	world.add(std::make_shared<sphere>(vec3(0.75f, -0.3f, -1.1f), 0.2f, chestnut));     // 栗(r=0.2f, y = -0.3)
    // ヘタと栗は重なっているが、リストが一番手前を選ぶので緑の防止に見える

    // ---- 落ち葉 ----
	// 小さい球をランダムに配置する。柿と栗の下には置かない。
    const int LEAVES = 1600;     //　数を増やすほど、描画時間も増える
	std::mt19937 rng(42);       //  乱数　42 は「種」同じ種なら同じ結果になる
	std::uniform_real_distribution<float> rnd(0.0f, 1.0f);  // 0.0～1.0の範囲で一様分布の乱数を作る
	vec3 leafColors[] = { vec3(0.85f, 0.15f, 0.05f), vec3(0.95f, 0.45f, 0.05f),     // 赤、オレンジ
                          vec3(0.90f, 0.70f, 0.10f), vec3(0.60f, 0.15f, 0.05f),     // 黄、茶色
                          vec3(0.75f, 0.30f, 0.05f) };                              // 濃いオレンジ
    int placed = 0;
    while (placed < LEAVES) {
		// 落ち葉の位置と半径をランダムに決める
		float x = -3.5f + 7.0f * rnd(rng);          // 横 -3.5～3.5の範囲でランダム
		float z = -5.0f + 4.75f * rnd(rng);         // 奥行 -5.0～-0.25の範囲でランダム
		float r = 0.03f + 0.04f * rnd(rng);         // 半径 0.03～0.07の範囲でランダム

        // 栗や柿の↓には置かない
        // (上からみた2Dの距離が近すぎたらやり直す)
        float d1 = std::sqrt(x * x + (z + 1.2f) * (z + 1.2f));                      // 大きい柿から
        float d2 = std::sqrt((x + 0.9f) * (x + 0.9f) + (z + 1.7f) * (z + 1.7f));   // 小さい柿から
        float d3 = std::sqrt((x - 0.75f) * (x - 0.75f) + (z + 1.1f) * (z + 1.1f)); // 栗から
		if (d1 < 0.45f || d2 < 0.28f || d3 < 0.18f) continue;                       // 近すぎるのでやり直し

        // この(x,z)での地面の高さ
		// 地面は球なので場所によって高さが変わる。球の方程式 x^2 + y^2 + z^2 = r^2 から y を求める。
        float dz = z + 1.0f;
        float groundY = -100.5f + std::sqrt(100.0f * 100.0f - x * x - dz * dz);
		// 落ち葉の色をランダムに選ぶ(0～4    )
        vec3 color = leafColors[int(rnd(rng) * 5.0f) % 5];
        // 中心を地面より半径の半分下におくことで、上の部分が「はみ出る」
        world.add(std::make_shared<sphere>(vec3(x, groundY - r * 0.5f, z), r, color));
        placed++;
    }

	// 撮影して表示する
	// 描画時間は物体の数に比例
    // 全ピクセル x （視線レイ + 影のレイ）　x 全物体 -> 約29億回の当たり判定
    camera cam;

    //描画計測
    auto t0 = std::chrono::steady_clock::now();
    std::vector<vec3> image = cam.render(world);
    auto t1 = std::chrono::steady_clock::now();
    std::cout << "objects: " << world.objects.size() << "  time: "
        << std::chrono::duration<double>(t1 - t0).count() << " s\n";


    //std::vector<vec3> image = cam.render(world);

    show_image("Autumn Ray Tracer - Part 1", image, cam.width, cam.height);
    return 0;
}