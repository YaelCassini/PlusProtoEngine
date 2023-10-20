// Author: Peiyao Li
// Date:   Mar 1 2023
#pragma once
#include "Global.hpp"
#include "Ray.hpp"

class AABB
{
private:
	glm::vec3 _min, _max;
public:
	AABB() {}
	AABB(const glm::vec3& v);
	void init();
	
	bool hit(const Ray& r, flt tmin, flt tmax, flt& thit) const;

	bool overlaps(const AABB& b) const;
	bool inside(const glm::vec3& p) const;

	inline glm::vec3 center() const { return (_min + _max) * flt(0.5); }
	inline flt volume() const { return width() * height() * depth(); }
	inline flt width()  const { return _max[0] - _min[0]; }
	inline flt height() const { return _max[1] - _min[1]; }
	inline flt depth()  const { return _max[2] - _min[2]; }

	inline glm::vec3 getMax() const { return _max; }
	inline glm::vec3 getMin() const { return _min; }

	AABB& operator += (const glm::vec3& p)
	{
		_min = glm::min(_min, p);
		_max = glm::max(_max, p);
		return *this;
	}
	AABB operator + (const glm::vec3& p)
	{
		AABB _res;
		_res._min = glm::min(_min, p);
		_res._max = glm::max(_max, p);
		return _res;
	}
	AABB& operator += (const AABB& b)
	{
		_min = glm::min(_min, b._min);
		_max = glm::max(_max, b._max);
		return *this;
	}
	AABB operator +(const AABB& v)
	{
		AABB _res;
		_res._min = glm::min(_min, v._min);
		_res._max = glm::max(_max, v._max);
		return _res;
	}
	void print() const;
};

