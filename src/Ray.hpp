// Author: Peiyao Li
// Date:   Mar 1 2023
#pragma once
#include "Global.hpp"

class Ray
{
private:
	glm::vec3 _origin;
	glm::vec3 _direction;

public:
	Ray(){}
	Ray(const glm::vec3& o, const glm::vec3& d) :_origin(o), _direction(glm::normalize(d)) {}

	inline glm::vec3 at(flt t) { return _origin + t * _direction; }

	inline void setOrigin(const glm::vec3& o) { _origin = o; }
	inline void setDirection(const glm::vec3& d) { _direction = d; }	
	inline glm::vec3 getOrigin() const { return _origin; }
	inline glm::vec3 getDirection() const { return _direction; }
};