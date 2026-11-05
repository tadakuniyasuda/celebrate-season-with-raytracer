
// ============================================================
//  hittable_list.h
//  たくさんの物体をまとめて「1つの hittable」として扱うリスト
//  レイを投げると全部の物体を調べて、一番手前の当たりを返す
//  （Unity の Physics.Raycast が一番近い Collider を返すのと同じ）
// ============================================================
#pragma once
#include <memory>
#include <vector>
#include "hittable.h"

// リスト自体も hittable の一種
// → カメラは「球1個」でも「800枚の落ち葉」でも区別せず、world.hit() と聞くだけでいい
class hittable_list : public hittable
{
public:
	// 物体を入れておく配列
	// shared_ptr<hittable> = 使う人がいなくなると自動で削除されるポインタ（C# の参照に近い）
	// sphere など色々な子クラスを1つの vector に入れるには、ポインタで持つ必要がある
	std::vector<std::shared_ptr<hittable>> objects;

	// 物体をリストに追加する
	void add(std::shared_ptr<hittable> object) { objects.push_back(object); }

	// レイが何かに当たるか調べて、一番手前の当たりを rec に入れる
	bool hit(const ray& r, float t_min, float t_max, hit_record& rec) const override
	{
		hit_record temp_rec;           // 1つの物体を調べた結果を一時的に入れる
		bool hit_anything = false;     // 何か1つでも当たったか
		float closest_so_far = t_max;  // 今までで一番近い当たりの距離（最初は「とても遠く」）

		// objects の中身を1つずつ取り出して調べる（C#のforeachと同じ）
		for (const auto& object : objects)
		{
			// 一番近いほうを記録
			if (object->hit(r, t_min, closest_so_far, temp_rec))
			{
				hit_anything = true;
				closest_so_far = temp_rec.t;	// 範囲を狭める
				rec = temp_rec;
			}
		}
		return hit_anything;
	}

};