// Author: Peiyao Li
// Date:   Mar 1 2023
#include "Emissive.hpp"
#include "Model.hpp"

void EmissiveGroup::init(const std::vector<shared_ptr<Emissive>>& lights)
{
	this->_lights = lights;
	_weight_sum.resize(lights.size());
	flt area_sum = 0;
	for (int i = 0; i < this->_lights.size(); i++)
	{
		area_sum += lights[i]->getArea();
		_weight_sum[i] = area_sum;
	}
	for (int i = 0; i < this->_lights.size(); i++)
	{
		_weight_sum[i] /= area_sum;
	}
	_area = area_sum;
}

flt EmissiveGroup::getArea() { 
	return _area;
}

flt EmissiveGroup::pdf(const HitRecord& rec, const HitRecord& light_rec)
{
	flt distance = glm::length(rec._pos - light_rec._pos);
	flt cos = glm::dot(glm::normalize(rec._pos - light_rec._pos), light_rec._normal);
	flt pdf = distance * distance / getArea() / cos;
	return pdf; 
}


flt EmissiveGroup::sampleRay(Bvh* bvh_tree, HitRecord& rec, Ray& sample_ray, HitRecord& light_rec)
{
	flt ran = random_float();
	// light sample
	for (int i = 0; i < _lights.size(); i++)
	{
		if (ran <= _weight_sum[i])
		{
			_lights[i]->sampleRay(bvh_tree, rec, sample_ray, light_rec);
			flt _pdf = pdf(rec, light_rec);
			return _pdf;
		}
		
	}
	ERRORM("There is no light.");
}