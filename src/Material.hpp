// Author: Peiyao Li
// Date:   Mar 1 2023

#pragma once
#include "Global.hpp"
#include "Model.hpp"
#include "Ray.hpp"

class Texture {
private:
    int _width, _height;
    flt* _data;

public:
    Texture() {};
    inline glm::vec3 at(int x, int y) const {
        int id = picChannel * ((y % _height + _height) % _height * _width + (x % _width + _width) % _width);
        return glm::vec3(_data[id + 0], _data[id + 1], _data[id + 2]);
    }
    inline glm::vec3 at(flt x, flt y) const {
        return this->at(static_cast<int>(round(x)), static_cast<int>(round(y)));
    }
    inline glm::vec3 at(const glm::vec2& uv) const {
        return this->at((_width)*uv[0], (_height) * (1.0f - uv[1]));
    }

    inline int getWidth() { return _width; }
    inline int getHeight() { return _height; }
    inline flt* getData() { return _data; }

    inline void setSize(int w, int h) { _width = w; _height = h; }
    inline void setWidth(int w) { _width = w; }
    inline void setHeight(int h) { _height = h; }
    inline void setData(flt* d) { _data = d; }

    ~Texture() { if (!_data) delete _data; }
};

class Material
{
public:
    MatType _type;
    std::string _name;

    Texture _texture;
    bool _has_texture;

	glm::vec3 _kd;
	glm::vec3 _ks;
	glm::vec3 _tr;    
    glm::vec3 _ke;
    bool _is_emissive;
	flt _ns;
	flt _ni;

public:
    virtual flt scatter(Ray& ray, HitRecord& rec, Ray& scattered) = 0;
    virtual glm::vec3 bsdf(glm::vec3& wi, HitRecord& rec, glm::vec3& wo) = 0;
    virtual flt pdf(const glm::vec3 wi, const HitRecord rec, const glm::vec3 wo) = 0;
};


class PhongMaterial : public Material
{
private:
    flt scatterUniform(glm::vec3& wi, const glm::vec3 normal, const glm::vec3 wo);
    flt scatterLambertian(glm::vec3& wi, const glm::vec3 normal);
    flt scatterSpecular(glm::vec3& wi, const glm::vec3 normal, const glm::vec3 wo);
    flt pdfLambertian(const glm::vec3 wi, const glm::vec3 normal);
    flt pdfSpecular(const glm::vec3 wi, const glm::vec3 normal, const glm::vec3 wo);

public:
	virtual flt scatter(Ray& ray, HitRecord& rec, Ray& scattered);
    virtual glm::vec3 bsdf(glm::vec3& wi, HitRecord& rec, glm::vec3& wo);
    virtual flt pdf(const glm::vec3 wi, const HitRecord rec, const glm::vec3 wo);
};

class GlassMaterial : public Material
{
public:
	virtual flt scatter(Ray& ray, HitRecord& rec, Ray& scattered);
    virtual glm::vec3 bsdf(glm::vec3& wi, HitRecord& rec, glm::vec3& wo);
    virtual flt pdf(const glm::vec3 wi, const HitRecord rec, const glm::vec3 wo);
};