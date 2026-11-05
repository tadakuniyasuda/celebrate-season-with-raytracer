#pragma once
#include <memory>
#include <vector>
#include "hittable.h"

class hittable_list : public hittable
{
public:
	std::vector<std::shared_ptr<hittable>> objects;
	void add(std::shared_ptr<hittable> object) { objects.push_back(object); }

	bool hit(const ray& r, float t_min, float t_max, hit_record& rec) const override
	{
		hit_record temp_rec;
		bool hit_anything = false;
		float closest_so_far = t_max;

		for (const auto& object : objects)
		{
			// only accept hits nearer than the best so far.  This is the key to getting the closest hit, not just any hit.
			if (object->hit(r, t_min, closest_so_far, temp_rec))
			{
				hit_anything = true;
				closest_so_far = temp_rec.t;	// shrink the window
				rec = temp_rec;
			}
		}
		return hit_anything;
	}

};