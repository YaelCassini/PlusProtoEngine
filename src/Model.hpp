// Author: Peiyao Li
// Date:   Mar 1 2023

#pragma once
#include <vector>
#include "Global.hpp"
#include "Ray.hpp"
#include "Emissive.hpp"
#include "AABB.hpp"

class AABB;
class HitRecord;
class Material;

class Hittable
{	
public:	
	shared_ptr<Material> _mat;
	std::string _mat_name;
	Hittable(){}
	Hittable(shared_ptr<Material> m):_mat(m){}

	virtual AABB boundingbox() const = 0;
	virtual bool hit(const Ray& r, const flt tmin, flt tmax, HitRecord& rec) = 0;
	virtual glm::vec3 getCenter() const = 0;
};



class HitRecord
{
public:
	glm::vec3 _pos;
	glm::vec3 _normal;
	glm::vec2 _uv;	
	flt _t = FLT_MAX;
	Hittable* _object;
	shared_ptr<Material> _mat;
	bool _front_face;

	inline void setFaceNormal(Ray& ray, const glm::vec3& normal) {
		_front_face = glm::dot(ray.getDirection(), normal) < 0;
		_normal = _front_face ? normal : -normal;
	}
};


class Sphere : public Hittable 
{
private:
	glm::vec3 center;
	flt radius;
public:
	Sphere() {}
	Sphere(glm::vec3 c, double r) : center(c), radius(r) {};
	Sphere(glm::vec3 c, double r, shared_ptr<Material> m) : Hittable(m), center(c), radius(r){};
	
	AABB boundingbox() const;
	virtual bool hit(Ray& r, flt tmin, flt tmax, HitRecord& rec);

	inline glm::vec3 getCenter()const { return center; }
	inline flt getRadius()const { return radius; }
};

class Triangle : public Hittable, public Emissive
{
private:
	glm::vec2 _tex[3];
	glm::vec3 _nrm[3];
	glm::vec3 _normal;
	flt _area;
	bool _has_vn = false;
	bool _has_vt = false;
	glm::vec3 _pos[3];

public:
	Triangle() {}
	Triangle(glm::vec3& vp1, glm::vec3& vp2, glm::vec3& vp3);

	virtual AABB boundingbox() const;
	virtual bool hit(const Ray& r, flt tmin, flt tmax, HitRecord& rec);
	virtual glm::vec3 getCenter()const { return (_pos[0] + _pos[1] + _pos[2]) / flt(3); }

	virtual flt getArea() { return _area; }	
	virtual flt sampleRay(Bvh* bvh_tree, HitRecord& rec, Ray& sample_ray, HitRecord& light_rec);
	virtual flt pdf(const HitRecord& rec, const HitRecord& light_rec);
	
	glm::vec3 samplePoint();
	void setVertexNormal(glm::vec3& vn1, glm::vec3& vn2, glm::vec3& vn3);
	void setVertexTexCoord(glm::vec2& vt1, glm::vec2& vt2, glm::vec2& vt3);

	inline glm::vec3 getFaceNormal() { return _normal; }
	inline void reverseFaceNormal() { _normal = -_normal; }	
	inline bool hasVt() { return _has_vt; }
	inline bool hasVn() { return _has_vn; }
};


