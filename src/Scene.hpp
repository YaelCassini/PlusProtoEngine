// Author: Peiyao Li
// Date:   Mar 1 2023
#pragma once

#include "Global.hpp"
#include "Camera.hpp"
#include "Model.hpp"
#include "Buffer.hpp"
#include "Material.hpp"
#include "BVH.hpp"
#include "Emissive.hpp"
#include "Timer.hpp"

class Scene
{
public:
	Camera cam;
	Buffer buf;
	Bvh bvh_tree;
	EmissiveGroup egroup;
	std::vector<shared_ptr<Material>> materials;
	shared_ptr<Material> default_mat;

	void readMeshes(
		const std::string& inputfile, 
		tinyobj::ObjReader& objreader);
	void readRadiances(
		const tinyxml2::XMLDocument& xmlconfig,
		std::map<std::string, glm::vec3>& light_radiance);
	void readMaterials(
		const std::string& objectdir,
		const std::vector<tinyobj::material_t>& material_info,
		std::map<std::string, glm::vec3>& light_radiance);
	void saveToScene(
		const std::vector<tinyobj::shape_t>& shapes,
		const tinyobj::attrib_t& attrib);

public:
	Scene() {}
	Scene(std::string& scenepath, std::string& scenename, std::string& objname);
	void buildScene(std::string& scenepath, std::string& scenename, std::string& objname);
	void addObject(Hittable* obj);
	void addMaterial(shared_ptr<Material> mat);
	glm::vec3 Li(Ray& r, std::vector<Hittable*>& objects, int depth);
	glm::vec3 sampleLight(Ray& ray, HitRecord& rec);

	void render(std::string& output, int spp, int maxdepth);
};