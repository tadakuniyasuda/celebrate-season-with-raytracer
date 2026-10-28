// ============================================================
//  Halloween Ray Tracer
//  光線（レイ）を1ピクセルずつ飛ばして、カボチャと夜空を描き、
//  最後に「発光している部分」だけをぼかして重ねる（ブルーム）。
//
//  全体の流れ:
//    1. 各ピクセルからレイを飛ばす            → main() のループ
//    2. レイが何に当たったか調べて色を決める  → ray_color()
//    3. 光っている部分だけをぼかす            → gaussian_weights() / blur_pass()
//    4. 元の画像にぼかした光を足して表示      → main() の最後
// ============================================================

#include <vector>
#include <algorithm>   // std::max
#include <cmath>       // std::sqrt, std::exp, std::fabs, std::ceil
#include <cstdlib>     // std::abs
#include "vec3.h"
#include "ray.h"
#include "display.h"


// ------------------------------------------------------------
//  Shade: 1ピクセル分の「答え」を2つまとめて返すための入れ物
//    color : カメラから見えている色（普通の画像用）
//    glow  : そこから出ている光（ブルーム用）。光らない所は黒(0,0,0)
//  → Unreal の Base Color と Emissive Color を分けているのと同じ考え方
// ------------------------------------------------------------
struct Shade {
	vec3 color;
	vec3 glow;
};


// ------------------------------------------------------------
//  レイと球の当たり判定
//    戻り値: 当たった場所までの距離 t（一番手前のもの）
//            当たらなければ -1
//
//  考え方:
//    球の表面の点 P は「中心 C からの距離がちょうど半径 r」
//        (P - C)・(P - C) = r * r
//    レイ上の点は P(t) = A + t*b（A:始点, b:方向）
//    これを代入すると t の2次方程式 a*t² + b*t + c = 0 になる
// ------------------------------------------------------------
float hit_sphere(const vec3& center, float radius, const ray& r)
{
	vec3 oc = r.origin() - center;  // 始点 - 中心 (A - C)

	// 2次方程式の係数 a, b, c
	float a = dot(r.direction(), r.direction());
	float b = 2.0f * dot(oc, r.direction());
	float c = dot(oc, oc) - radius * radius;

	// 判別式: マイナスなら解なし = レイは球に当たらない
	float discriminant = b * b - 4.0f * a * c;
	if (discriminant < 0.0f)
	{
		return -1.0f;
	}

	// 解の公式で2つの t を求める
	float sqrtD = std::sqrt(discriminant);
	float t1 = (-b - sqrtD) / (2.0f * a);  // 手前側（球に入る所）
	float t2 = (-b + sqrtD) / (2.0f * a);  // 奥側（球から出る所）

	// 0.001 より大きいものだけ有効
	if (t1 > 0.001f) return t1;
	if (t2 > 0.001f) return t2;  // カメラが球の中にいる場合

	// どちらも後ろ → 当たっていない扱い
	// ※この return を消すと C++ では「未定義動作」になる
	return -1.0f;
}


// ------------------------------------------------------------
//  夜空の色
//  レイが上を向くほど濃い紫、水平に近いほど明るい紫
// ------------------------------------------------------------
vec3 sky_color(const ray& r)
{
	vec3 dir = unit_vector(r.direction());  // 長さ1の向きにする

	// dir.y は -1（真下）〜 +1（真上）。これを 0〜1 に変換する
	float t = 0.5f * (dir.y() + 1.0f);

	vec3 horizon(0.6f, 0.15f, 0.48f);  // 地平線の色（明るい紫）
	vec3 night(0.09f, 0.03f, 0.22f);   // 真上の色（濃い紫）

	// 線形補間（Lerp）: t=0 なら horizon、t=1 なら night
	return (1.0f - t) * horizon + t * night;
}


// ------------------------------------------------------------
//  1本のレイが「何を見るか」を決める関数
//    戻り値: { 見える色, 出ている光 }
// ------------------------------------------------------------
Shade ray_color(const ray& r)
{
	// カボチャ（球）の中心と半径
	vec3 center(0.0f, 0.0f, -1.0f);
	float radius = 0.5f;

	float t = hit_sphere(center, radius, r);

	// ========== カボチャに当たらなかった場合: 月 か 夜空 ==========
	if (t <= 0.0f)
	{
		vec3 dir = unit_vector(r.direction());
		vec3 moonDir = unit_vector(vec3(0.3f, 0.3f, -0.3f));  // 月の「方向」（位置ではない）

		// 内積 = 2つの向きがどれだけ同じか（1 = 完全に同じ向き）
		// 0.995 より大きい = 月の方向からほぼズレていない → 月のピクセル
		// ※ 月は円を「描いて」いない。条件を満たしたピクセルの集まりが円になる
		if (dot(dir, moonDir) > 0.995f)
		{
			vec3 moon(1.0f, 0.95f, 0.0f);
			return { moon, moon };  // 見える色も、出す光も月の色 → 月にも光の輪がつく
		}

		return { sky_color(r), vec3() };  // 普通の空: 光は出さない
	}

	// ========== カボチャに当たった場合 ==========
	// 当たった点と、その点の法線（表面が向いている方向）
	vec3 hitPoint = r.at(t);
	// 球の法線 = 中心から当たった点への向き。(半径で割ると長さ1になる)
	vec3 normal = (hitPoint - center) / radius;

	// ---------- 顔を彫る ----------
	// カメラ側から見ると、法線の x, y は顔の上の 2D 座標として使える
	//   (0,0) = 顔の真ん中、端 = ±1、x は右がプラス、y は上がプラス
	float x = normal.x();
	float y = normal.y();

	// 目（三角形）:
	//   「縦の線からの距離 < 幅」で縦の帯ができる
	//   その幅を y が上がるほど狭くする → 上がとがった三角形になる
	float eyeWidth = 0.45f * (0.45f - y);  // y=0.45 で幅 0（三角形の頂点）

	// fabs(x + 0.35) = x=-0.35（左目の中心）からの距離
	bool leftEye = y > 0.10f && y < 0.45f && std::fabs(x + 0.35f) < eyeWidth;
	bool rightEye = y > 0.10f && y < 0.45f && std::fabs(x - 0.35f) < eyeWidth;

	// 口（放物線の帯）:
	//   y = -0.28 + 0.9x² は x=0 が一番低い U 字のカーブ（＝笑顔）
	float smileCurve = -0.28f + 0.9f * x * x;

	// 「カーブからの距離 < 0.04」で太さ 0.08 の帯になる
	// fabs(x) < 0.42 で口の左右の端を切る
	bool mouth = std::fabs(x) < 0.42f && y > -0.35f && y < -0.05f
		&& std::fabs(y - smileCurve) < 0.04f;

	// normal.z > 0 = カメラ側を向いている面だけ
	// （これがないと、球の裏側にも同じ顔が彫られてしまう）
	bool face = normal.z() > 0.0f && (leftEye || rightEye || mouth);

	if (face)
	{
		// 顔はライトの影響を受けない（中のロウソクで光っているイメージ）
		vec3 candle(1.0f, 0.92f, 0.35f);
		return { candle, candle };  // 見える色 = ロウソク色、出す光 = ロウソク色
	}

	// ---------- 胴体のライティング（Lambert） ----------
	// 光の方向: (0, -1, 0) = 下から照らす（懐中電灯をあごの下から当てる感じ）
	vec3 lightDir = unit_vector(vec3(0.0f, -1.0f, 0.0f));

	// 内積 = 面が光の方をどれだけ向いているか
	//   1: 正面から光が当たる / 0: 真横 / マイナス: 裏側 → 0 に切り上げる
	float diffuse = std::max(0.0f, dot(normal, lightDir));

	// 0.2 = 環境光（真っ黒にならないための最低限の明るさ）
	float brightness = 0.2f + 0.8f * diffuse;

	vec3 pumpkin(1.0f, 0.35f, 0.05f);           // カボチャのオレンジ
	return { brightness * pumpkin, vec3() };    // 胴体は光を出さない
}


// ------------------------------------------------------------
//  ガウスぼかし用の重み（ウェイト）を作る
//    weight(d) = exp( -d² / (2σ²) )
//      d     : 中心からの距離（ピクセル）
//      sigma : ぼかしの広さ（大きいほど柔らかく広がる）
//      exp   : 0 のとき 1、値が大きくなるほど滑らかに 0 へ近づく関数
//
//    中心 = 1.0、1σ = 0.61、2σ = 0.14、3σ = 0.01
//    → 3σ より先はほぼ 0 なので、そこで打ち切る（radius = 3σ）
//
//    左右対称なので片側（0 〜 radius）だけ保存すればよい
// ------------------------------------------------------------
std::vector<float> gaussian_weights(float sigma)
{
	int radius = int(std::ceil(3.0f * sigma));  // sigma=12 なら 36

	// radius + 1 個: 距離 0（中心）〜 36 まで、両端を含めて 37 個
	std::vector<float> w(radius + 1);

	for (int d = 0; d <= radius; d++)
	{
		w[d] = std::exp(-(d * d) / (2.0f * sigma * sigma));
	}
	return w; 
}


// ------------------------------------------------------------
//  1方向だけのぼかし（横 or 縦）
//    各ピクセル = 周りのピクセルの「重み付き平均」
//
//  分離可能（セパラブル）ブラー:
//    2D でまとめてぼかすと 1ピクセルあたり 73×73 = 5,329 回読む
//    横 → 縦 の2回に分けると 73 + 73 = 146 回で、同じ結果になる
//    （exp(a+b) = exp(a)×exp(b) なので、2D の重み = 横の重み × 縦の重み）
//    実測: Release で約 43 倍速い
// ------------------------------------------------------------
std::vector<vec3> blur_pass(const std::vector<vec3>& src,   // const& = コピーしない＆書き換えない
	int width, int height,
	const std::vector<float>& weights,
	bool horizontal)                                     // true = 横方向, false = 縦方向
{
	const int radius = int(weights.size()) - 1;
	std::vector<vec3> dst(width * height);  // 結果を入れる新しい画像

	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			vec3 sum;            // 色の合計（vec3() は 0 で初期化される）
			float total = 0.0f;  // 使った重みの合計

			// 中心から -radius 〜 +radius まで、周りのピクセルを見る
			for (int d = -radius; d <= radius; d++)
			{
				// 横なら x をずらす、縦なら y をずらす
				int sx = horizontal ? x + d : x;
				int sy = horizontal ? y : y + d;

				// 画像の外は飛ばす
				if (sx < 0 || sx >= width || sy < 0 || sy >= height)
				{
					continue;
				}

				// 左右対称なので、-5 も +5 も weights[5] を使う
				float w = weights[std::abs(d)];

				// 2D の (sx, sy) を 1D の配列の番号に変換: y * 幅 + x
				sum += src[sy * width + sx] * w;  // 重み*色
				total += w;  // 重みだけの合計
			}

			// 使った重みの合計で割る（＝重み付き平均）
			//  → 全体の明るさが変わらない
			//  → 画像の端で暗くならない
			dst[y * width + x] = sum / total;
		}
	}
	return dst;
}


// ------------------------------------------------------------
//  メイン: レンダリング → ぼかし → 合成 → 表示
// ------------------------------------------------------------
int main()
{
	const int width = 800;
	const int height = 600;

	// ---------- カメラ ----------
	// 目は原点にあり、-z 方向を見ている
	// 1単位前に 4×3 の「窓」があり、各ピクセルはその窓の1点に対応
	// ※ 4×3 と 800×600 の比率をそろえないと、球が楕円に伸びる
	vec3 origin(0.0f, 0.0f, 0.0f);
	vec3 lower_left(-2.0f, -1.5f, -1.0f);  // 窓の左下の角
	vec3 horizontal(4.0f, 0.0f, 0.0f);     // 窓の横幅
	vec3 vertical(0.0f, 3.0f, 0.0f);       // 窓の高さ

	// ---------- 1. レンダリング ----------
	// 先に画像全体をバッファ（メモリ）に書いてから、最後にまとめて表示する
	// → 表示前に「ぼかし」などの後処理ができる（エンジンのポストプロセスと同じ）
	std::vector<vec3> image(width * height);  // 見える色
	std::vector<vec3> glow(width * height);   // 光っている部分だけ

	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			// ピクセル位置を 0〜1 に変換
			float u = float(x) / float(width - 1);                // 左 0 → 右 1
			float v = float(height - 1 - y) / float(height - 1);  // 下 0 → 上 1（画面の y は下向きなので反転）

			// カメラからこのピクセルを通るレイを作る
			vec3 direction = lower_left + u * horizontal + v * vertical - origin;
			ray r(origin, direction);

			// このレイが見るもの（色と光）
			Shade s = ray_color(r);

			// 2D の (x, y) を 1D の番号に: y * 幅 + x
			image[y * width + x] = s.color;
			glow[y * width + x] = s.glow;
		}
	}

	// ---------- 2. 光の部分だけをぼかす ----------
	std::vector<float> weights = gaussian_weights(12.0f);  // sigma = 12 → 半径 36 ピクセル

	std::vector<vec3> blurred = blur_pass(glow, width, height, weights, true);  // 横にぼかす
	blurred = blur_pass(blurred, width, height, weights, false);                // その結果を縦にぼかす

	// ---------- 3. ぼかした光を元の画像に足す ----------
	// 光は「足し算」: 2つのライトで照らすと1つより明るい
	// 1.0 を超えた値は display.h の to_byte() で 0〜1 にクランプされる
	const float bloomStrength = 1.5f;  // 0 でブルームなし、大きいほど強く光る
	for (int i = 0; i < width * height; i++)
	{
		image[i] += blurred[i] * bloomStrength;
	}

	// ---------- 4. 表示 ----------
	show_image("Halloween Ray Tracer", image, width, height);

	return 0;
}
