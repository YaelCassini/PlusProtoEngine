// Author: Peiyao Li
// Date:   Mar 1 2023
#pragma once
#include "Global.hpp"
class Ray;
class Bvh;
class HitRecord;

class Emissive
{
private:
	glm::vec3 _radiance;
public:
	virtual flt getArea() = 0;
	virtual flt sampleRay(Bvh* bvh_tree, HitRecord& rec, Ray& sample_ray, HitRecord& light_rec) = 0;
	virtual flt pdf(const HitRecord& rec, const HitRecord& light_rec) = 0;
};


class EmissiveGroup: public Emissive
{
private:
	std::vector<shared_ptr<Emissive>> _lights;
	std::vector<flt> _weight_sum;
	flt _area;

public:
	EmissiveGroup(){}
	EmissiveGroup(const std::vector<shared_ptr<Emissive>>& lights) { init(lights); }
	void init(const std::vector<shared_ptr<Emissive>>& lights);

	virtual flt getArea();
	virtual flt sampleRay(Bvh* bvh_tree, HitRecord& rec, Ray& sample_ray, HitRecord& light_rec);
	virtual flt pdf(const HitRecord& rec, const HitRecord& light_rec);
};