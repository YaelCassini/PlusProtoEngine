// Author: Peiyao Li
// Date:   Mar 1 2023
#pragma once
#include "Global.hpp"
#include "Model.hpp"
#include "AABB.hpp"
#define BOX AABB

class AAP {
public:
	char _xyz;
	float _p;
	AAP(const BOX& total);
	bool inside(const glm::vec3& mid) const;
};

class BvhNode 
{
private:
	int _flag;
	BOX _box;
	int _child; // >=0 leaf with tri_id, <0 left & right
	int _parent;
	Hittable* _object;

public:
	BvhNode() { _flag = 0; _child = 0; _parent = 0; }

	void construct(unsigned int id);
	void construct(unsigned int* lst, unsigned int num);	
	void refit();
	void setParent(int p) { _parent = p; }
	void resetParents(BvhNode* root);
	void travel();

	int hit(const Ray& ray, flt tmin, flt tmax, HitRecord& rec);

	inline Hittable* getObject() const { return _object; }
	inline BvhNode* left(){ return this - _child; }
	inline BvhNode* right(){ return this - _child + 1; }
	inline BOX& box(){ return _box; }
	inline int isLeaf() const { return _child >= 0; }
	inline int getID() const { return _child; }
	inline int getParentID() const { return _parent; }

	friend class Bvh;
};

class Bvh {
private:
	int _num;
	BvhNode* _nodes;
	std::vector<Hittable*> _objects;

public:
	Bvh(){ }
	Bvh(const std::vector<Hittable*> objects);

	void buildTree(const std::vector<Hittable*>& objects);
	void construct(const std::vector<Hittable*>& objects);
	void setObjects(const std::vector<Hittable*>& objects);
	void refit();
	void reorder();	

	void travel();

	bool hit(const Ray& r, flt tmin, flt tmax, HitRecord& rec);

	inline BvhNode* getRoot() { return _nodes; }
	inline int getNum() const { return _num; }
	inline std::vector<Hittable*>& getObjects() { return _objects; }

	~Bvh() { if (_nodes) delete _nodes; }
};

