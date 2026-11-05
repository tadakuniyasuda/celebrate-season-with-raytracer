// hittable.h
// 「レイが当たるもの」全てに共通のルールを定義する。抽象クラス。
// Unityで例えると、Colliderのようなもの。
// Colliderは「当たるもの」全てに共通のルールを定義する抽象クラス。
// Unity で例えると
// hittable -> Collider
// hit_record -> RaycastHit(当たった点、normals、距離)
#pragma once
#include "ray.h"

// =====================================================
// hit_record: 一回の当たりについて分かったこと全部
// =====================================================
struct hit_record {
	vec3 p;				// 当たった点
	vec3 normal;		// 法線（面の向き） 常にレイの来た側を向くようにする
	float t = 0.0f; 		// レイの始点から当たった点までの距離
	bool front_face = false; // 外部から当たったか=true / 内部からあたったか = false
	vec3 albedo;			// 表面の色（次回、マテリアルなどに置き換えます）
	float shine = 0.0f;   // ツヤ:　0= マット　1= ツヤツヤ

	// 法線の向きを揃える　
	void set_face_normal(const ray& r, const vec3& outward_normal) {
		front_face = dot(r.direction(), outward_normal) < 0.0f;
		normal = front_face ? outward_normal : -outward_normal;
	}
};

//全ての物体が応える質問「このrayはt_minからt_maxの間で当たりますか？あたるならrecに詳細を書いてtrueを返してね」
class hittable {
public : 
	// virtual destructor 
	virtual ~hittable() = default;
	// hit_record& rec = 結果を入れる「出力用」の因数 (unityのPhysics.Raycast(..., out RaycastHit hit)と同じ)
	virtual bool hit(const ray& r, float t_min, float t_max, hit_record& rec) const = 0;
};