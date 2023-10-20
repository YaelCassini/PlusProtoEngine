// Author: Peiyao Li
// Date:   Jan 8 2023
#include "Model.hpp"
#include "Ray.hpp"
#include "BVH.hpp"

AABB Sphere::boundingbox() const
{
    AABB aabb(center);
    aabb += center + radius;
    aabb += center - radius;

    return aabb;
}

bool Sphere::hit(Ray& ray, flt tmin, flt tmax, HitRecord& rec)
{
    glm::vec3 oc = ray.getOrigin() - center;
    auto a = glm::dot(ray.getDirection(), ray.getDirection());
    auto half_b = glm::dot(oc, ray.getDirection());
    auto c = glm::dot(oc, oc) - radius * radius;
    auto discriminant = half_b * half_b - a * c;

    if (discriminant < 0)return false;

    rec._normal = glm::vec3(0.5, 0.5, 0.5);

    auto sqrtd = sqrt(discriminant);

    // Find the nearest root that lies in the acceptable range.
    auto root = (-half_b - sqrtd) / a;
    if (root < tmin || tmax < root) {
        root = (-half_b + sqrtd) / a;
        if (root < tmin || tmax < root)
            return false;
    }
       
    rec._t = root;
    rec._pos = ray.at(rec._t);
    glm::vec3 outward_normal = glm::normalize((rec._pos - center) / radius);
    rec.setFaceNormal(ray, outward_normal);
    rec._mat = _mat;
    return true;
}

Triangle::Triangle(glm::vec3& vp1, glm::vec3& vp2, glm::vec3& vp3)
{
    _pos[0] = vp1;
    _pos[1] = vp2;
    _pos[2] = vp3;

    _normal = glm::normalize(glm::cross(_pos[1] - _pos[0], _pos[2] - _pos[0]));
    _area = 0.5 * fabsf(glm::length(glm::cross(_pos[1] - _pos[0], _pos[2] - _pos[0])));
}

AABB Triangle::boundingbox() const
{
    AABB aabb(_pos[0]);
    aabb += _pos[1];
    aabb += _pos[2];

    return aabb;
}

flt Triangle::sampleRay(Bvh* bvh_tree, HitRecord& rec, Ray& sample_ray, HitRecord& light_rec)
{
    glm::vec3 sample_p = samplePoint();
    Ray light_ray(rec._pos, sample_p - rec._pos);


    bool flaghit = bvh_tree->hit(light_ray, kHitEps, FLT_MAX, light_rec);
    bool flaghitlight = glm::dot(light_rec._pos - sample_p, light_rec._pos - sample_p) < kEps;
    bool flaglightdirect = glm::dot(light_ray.getDirection(), light_rec._normal) < 0;
    bool flagobjdirect = glm::dot(light_ray.getDirection(), rec._normal) > 0;
    if (flaghit && flaghitlight && flaglightdirect && flagobjdirect)
    {
        sample_ray = light_ray;
        //std::cout << samplePdf(rec, light_rec) << std::endl;;
        return pdf(rec,light_rec);
    }

    return 0;
}

flt Triangle::pdf(const HitRecord& rec, const HitRecord& light_rec)
{
    flt distance = glm::length(rec._pos - light_rec._pos);
    flt cos = glm::dot(glm::normalize(rec._pos - light_rec._pos), light_rec._normal);
    flt pdf = distance * distance / getArea() / cos;
    return pdf;   
}

glm::vec3 Triangle::samplePoint()
{
    flt sqrt_a = random_float();
    flt b = random_float();

    glm::vec3 point;
    point = (1 - sqrt_a) * _pos[0] + (sqrt_a * (1 - b)) * _pos[1] + (b * sqrt_a) * _pos[2];
    return point;
}

bool Triangle::hit(const Ray& r, flt tmin, flt tmax, HitRecord& rec)
{
    glm::vec3 vertex0 = _pos[0];
    glm::vec3 vertex1 = _pos[1];
    glm::vec3 vertex2 = _pos[2];
    glm::vec3 edge1, edge2, h, s, q;
    flt a, f, u, v, w;
    edge1 = vertex1 - vertex0;
    edge2 = vertex2 - vertex0;
    h = glm::cross(r.getDirection(), edge2);
    a = glm::dot(edge1, h);
    if (a > -kEps && a < kEps)
        return false; // This ray is parallel to this triangle.
    f = 1.0f / a;
    s = r.getOrigin() - vertex0;
    u = f * glm::dot(s, h);
    if (u < 0.0f || u > 1.0f)
        return false;
    q = glm::cross(s, edge1);
    v = f * glm::dot(r.getDirection(), q);
    if (v < 0.0f || u + v > 1.0f)
        return false;
    w = 1.0f - u - v;
    // At this stage we can compute t to find out where the intersection point is on the line.
    float t = f * glm::dot(edge2, q);
    if (t > tmin && t < tmax) // ray intersection
    {
        rec._pos = r.getOrigin() + r.getDirection() * t;
        rec._t = t;
        rec._normal = _normal;
        rec._uv = _tex[0] * w + _tex[1] * u + _tex[2] * v;
        rec._mat = _mat;
        rec._object = static_cast<Triangle*>(this);
        return true;
    }
    else // This means that there is a line intersection but not a ray intersection.
        return false;
    return false;
}

void Triangle::setVertexNormal(glm::vec3& vn1, glm::vec3& vn2, glm::vec3& vn3)
{
    _has_vn = true;
    _nrm[0] = vn1;
    _nrm[1] = vn2;
    _nrm[2] = vn3;
}

void Triangle::setVertexTexCoord(glm::vec2& vt1, glm::vec2& vt2, glm::vec2& vt3)
{
    _has_vt = true;
    _tex[0] = vt1;
    _tex[1] = vt2;
    _tex[2] = vt3;
}



