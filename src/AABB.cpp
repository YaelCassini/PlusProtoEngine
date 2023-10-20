// Author: Peiyao Li
// Date:   Mar 1 2023

#include "AABB.hpp"

AABB::AABB(const glm::vec3& v) {
	_min = _max = v;
}

void AABB::init() {
	_max = glm::vec3(-FLT_MAX, -FLT_MAX, -FLT_MAX);
	_min = glm::vec3(FLT_MAX, FLT_MAX, FLT_MAX);
}

void AABB::print() const
{
	std::cout << "_min:" << _min.x << " " << _min.y << " " << _min.z 
		<< "_max:" << _max.x << " " << _max.y << " " << _max.z
		<< std::endl;
}


bool AABB::overlaps(const AABB& b) const
{
	if (_min[0] > b._max[0]) return false;
	if (_min[1] > b._max[1]) return false;
	if (_min[2] > b._max[2]) return false;

	if (_max[0] < b._min[0]) return false;
	if (_max[1] < b._min[1]) return false;
	if (_max[2] < b._min[2]) return false;

	return true;
}

bool AABB::inside(const glm::vec3& p) const
{
	if (p[0] < _min[0] || p[0] > _max[0]) return false;
	if (p[1] < _min[1] || p[1] > _max[1]) return false;
	if (p[2] < _min[2] || p[2] > _max[2]) return false;

	return true;
}

bool AABB::hit(const Ray& r, flt tmin, flt tmax, flt& thit) const
{
	for (int a = 0; a < 3; a++) {
		auto invD = 1.0f / r.getDirection()[a];
		auto t0 = (_min[a] - r.getOrigin()[a]) * invD;
		auto t1 = (_max[a] - r.getOrigin()[a]) * invD;
		if (invD < 0.0f)
			std::swap(t0, t1);
		tmin = t0 > tmin ? t0 : tmin;
		tmax = t1 < tmax ? t1 : tmax;
		if (tmax < tmin)
			return false;
	}
	thit = tmin;
	return true;
}