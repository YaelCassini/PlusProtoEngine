// Author: Peiyao Li
// Date:   Mar 1 2023
#pragma once
#include "Global.hpp"
#include "Ray.hpp"

class Camera
{
private:
	glm::vec3 _pos;
	glm::vec3 _lookat;
	glm::vec3 _up;
	flt _fovy, _aspect;
	uchar _type;
	int _width, _height;
	flt _near, _far;
	flt _focal_length = 1.0;

	glm::vec3 _left_top_pos;
	glm::vec3 _dposw, _dposh;

public:
	Camera() {}
	Camera(const unsigned char t, 
		const glm::vec3& p, const glm::vec3& l, const glm::vec3& u, flt f, int w, int h) :
		_type(t), _pos(p), _lookat(l), _up(u), _fovy(f), _width(w), _height(h) {
		_near = 0.01;
		_far = 10000;
		precomputeCamera();
	}
	void initFromXML(const tinyxml2::XMLDocument& xmlconfig);
	void precomputeCamera();	
	
	inline int getWidth() const { return _width; }
	inline int getHeight() const { return _height; }
	Ray genRay(int x, int y);
	Ray genRayRandom(int x, int y);
};
