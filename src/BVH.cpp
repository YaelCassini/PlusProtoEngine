// Author: Peiyao Li
// Date:   Mar 1 2023
#include "BVH.hpp"

static glm::vec3* s_centers;
static BOX* s_boxes;
static unsigned int* s_idx_buffer;
static BvhNode* s_current;

void BvhNode::construct(unsigned int id)
{
	_child = id;
	_box = s_boxes[id];
}

void BvhNode::construct(unsigned int* lst, unsigned int num)
{
	for (unsigned int i = 0; i < num; i++)
		_box += s_boxes[lst[i]];

	if (num == 1) {
		_child = lst[0];
		return;
	}

	// try to split them
	_child = int(this - s_current);
	s_current += 2;

	if (num == 2) {
		left()->construct(lst[0]);
		right()->construct(lst[1]);
		return;
	}

	AAP pln(_box);
	unsigned int left_idx = 0, right_idx = num - 1;
	for (unsigned int t = 0; t < num; t++) {
		int i = lst[left_idx];

		if (pln.inside(s_centers[i]))
			left_idx++;
		else {// swap it
			unsigned int tmp = lst[left_idx];
			lst[left_idx] = lst[right_idx];
			lst[right_idx--] = tmp;
		}
	}

	int half = num / 2;

	if (left_idx == 0 || left_idx == num) {
		left()->construct(lst, half);
		right()->construct(lst + half, num - half);
	}
	else {
		left()->construct(lst, left_idx);
		right()->construct(lst + left_idx, num - left_idx);
	}
}

void BvhNode::refit()
{
	if (isLeaf()) {
		_box = s_boxes[_child];

	}
	else {
		left()->refit();
		right()->refit();

		_box = left()->_box + right()->_box;
	}
}

void BvhNode::resetParents(BvhNode* root)
{
	if (this == root)
		setParent(-1);

	if (isLeaf())
		return;

	left()->resetParents(root);
	right()->resetParents(root);

	left()->setParent(this - root);
	right()->setParent(this - root);
}

void BvhNode::travel() {
	if (isLeaf()) {
		std::cout << _child << std::endl;
		printVec3(_box.getMin());
		printVec3(_box.getMax());

		return;
	}

	left()->travel();
	right()->travel();
}

int BvhNode::hit(const Ray& ray, flt tmin, flt tmax, HitRecord& rec)
{
	flt t_tmp = FLT_MAX;
	bool flag = _box.hit(ray, tmin, tmax, t_tmp);
	if (!flag) return -1;
	if (isLeaf()) {
		HitRecord rec_tmp;
		bool obj_hit = _object->hit(ray, tmin, tmax, rec_tmp);
		//std::cout << "bbox idx:" << _child << " objhit:" << obj_hit << " t:"<<rec.t<<std::endl;
		if (obj_hit) {
			rec = rec_tmp;
			return _child;
		}
		return -1;
	}
	HitRecord left_rec, right_rec;
	int left_idx = left()->hit(ray, tmin, tmax, left_rec);
	int right_idx = right()->hit(ray, tmin, tmax, right_rec);
	//if (left_idx == 2) std::cerr << left_rec.t << std::endl;
	if (left_idx >= 0 && right_idx >= 0) {
		if (left_rec._t <= right_rec._t) {
			rec = left_rec;
			return left_idx;
		}
		else {
			rec = right_rec;
			return right_idx;
		}
	}
	else if (left_idx >= 0) {
		rec = left_rec;
		return left_idx;
	}
	else if (right_idx >= 0) {
		rec = right_rec;
		return right_idx;
	}
	else
		return -1;
}


Bvh::Bvh(const std::vector<Hittable*> objects)
{
	buildTree(objects);
}

void Bvh::buildTree(const std::vector<Hittable*>& objects)
{
	_num = 0;
	_nodes = NULL;

	construct(objects);
	reorder();
	getRoot()->resetParents(_nodes); //update the parents after reorder ...
	setObjects(objects);

	if (s_boxes) delete s_boxes;
}

void Bvh::construct(const std::vector<Hittable*>& objects)
{
	_objects = objects;
	BOX total;
	_num = objects.size();

	s_centers = new glm::vec3[_num];
	s_boxes = new BOX[_num];

	int obj_idx = 0;
	int vtx_offset = 0;

	for (int i = 0; i < objects.size(); i++)
	{
		total += objects[i]->boundingbox();
		s_boxes[i] = objects[i]->boundingbox();
		s_centers[i] = objects[i]->getCenter();
	}

	AAP pln(total);
	s_idx_buffer = new unsigned int[_num];
	unsigned int left_idx = 0, right_idx = _num;

	for (int i = 0; i < objects.size(); i++)
	{
		if (pln.inside(s_centers[i]))
			s_idx_buffer[left_idx++] = i;
		else
			s_idx_buffer[--right_idx] = i;
	}

	_nodes = new BvhNode[_num * 2 - 1];
	_nodes[0]._box = total;
	s_current = _nodes + 3;

	if (_num == 1) {
		_nodes[0]._child = 0;
	}
	else {
		_nodes[0]._child = -1;

		if (left_idx == 0 || left_idx == _num)
			left_idx = _num / 2;

		_nodes[0].left()->construct(s_idx_buffer, left_idx);
		_nodes[0].right()->construct(s_idx_buffer + left_idx, _num - left_idx);
	}

	delete[] s_idx_buffer;
	delete[] s_centers;

	refit();
}

void Bvh::setObjects(const std::vector<Hittable*>& objects)
{
	for (int i = 0; i < _num * 2 - 1; i++)
	{
		int obj_idx = _nodes[i]._child;
		if (obj_idx >= 0)
		{
			_nodes[i]._object = objects[obj_idx];
		}
	}
}

void Bvh::refit()
{
	getRoot()->refit();
}

void Bvh::reorder()
{
	if (true)
	{
		std::queue<BvhNode*> q;
		// We need to perform a breadth-first traversal to fill the 
		// the first pass get idx for each node ...
		int* buffer = new int[_num * 2 - 1];
		int idx = 0;
		q.push(getRoot());
		while (!q.empty()) {
			BvhNode* node = q.front();
			buffer[node - _nodes] = idx++;
			q.pop();

			if (!node->isLeaf()) {
				q.push(node->left());
				q.push(node->right());
			}
		}

		// the 2nd pass, get right nodes ...
		BvhNode* new_nodes = new BvhNode[_num * 2 - 1];
		idx = 0;
		q.push(getRoot());
		while (!q.empty()) {
			BvhNode* node = q.front();
			q.pop();

			new_nodes[idx] = *node;
			if (!node->isLeaf()) {
				int loc = node->left() - _nodes;
				new_nodes[idx]._child = idx - buffer[loc];
			}
			idx++;

			if (!node->isLeaf()) {
				q.push(node->left());
				q.push(node->right());
			}
		}

		delete[] buffer;
		delete[] _nodes;
		_nodes = new_nodes;
	}
}

void Bvh::travel()
{
	getRoot()->travel();
}

bool Bvh::hit(const Ray& r, flt tmin, flt tmax, HitRecord& rec)
{
	flt t_hit = FLT_MAX;
	int obj_idx = _nodes->hit(r, tmin, tmax, rec);
	if (obj_idx >= 0)
	{
		//return true;
		bool flag = _objects[obj_idx]->hit(r, tmin, tmax, rec);
		return flag;
	}
	return false;
}


AAP::AAP(const BOX& total) {
	glm::vec3 center = total.center();
	char xyz = 2;

	if (total.width() >= total.height() && total.width() >= total.depth()) {
		xyz = 0;
	}
	else
		if (total.height() >= total.width() && total.height() >= total.depth()) {
			xyz = 1;
		}

	_xyz = xyz;
	_p = center[xyz];
}

bool AAP::inside(const glm::vec3& mid) const {
	return mid[_xyz] < _p;
}
