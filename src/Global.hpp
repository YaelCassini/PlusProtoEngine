// Author: Peiyao Li
// Date:   Mar 1 2023
#pragma once

#include <iostream>
#include <cmath>
#include <cstdlib>
#include <stdlib.h>
#include <limits>
#include <memory>
#include <vector>
#include <queue>
#include <functional>
#include <random>
#include <omp.h> 

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/component_wise.hpp>
#include "tinyxml2.h"
#include "tiny_obj_loader.h"
#include "stb/stb_image.h"
#include "stb/stb_image_write.h"

typedef float flt;
typedef unsigned char uchar;

#define _CRT_SECURE_NO_WARNINGS
#define IF_DEBUG 0
#define ERRORM(fmt, ...)                                  \
    do {                                                  \
        fprintf(stdout, "[ERROR] " fmt, ##__VA_ARGS__);   \
        exit(-1);                                         \
    } while (false)

#define INFO(fmt, ...)                                 \
    do {                                               \
        fprintf(stdout, "[INFO] " fmt, ##__VA_ARGS__); \
    } while (false)

#define DEBUGM(fmt, ...)                                    \
    do {                                                    \
        if (IF_DEBUG)                                       \
            fprintf(stdout, "[DEBUG] " fmt, ##__VA_ARGS__); \
    } while (false)

#define picChannel 3
// Usings
using std::shared_ptr;
using std::make_shared;
const flt kHitEps = 1e-3;
const flt kEps = 1e-8;
const flt infinity = std::numeric_limits<flt>::infinity();
const flt pi = 3.1415926535897932385;

enum MatType {
    LIGHT = 0,
    DIFFUSE = 1,
    GLASS = 2
};

inline void printVec3(glm::vec3 vec)
{
    std::cout << vec.x <<" "
        <<vec.y << " "
        << vec.z << std::endl;
}

inline flt degrees_to_radians(flt degrees) {
    return degrees * pi / 180;
}

// radians
inline glm::vec3 spherical_to_cartesian(flt theta, flt phi) {
    glm::vec3 res;
    res.x = glm::sin(theta) * glm::cos(phi);
    res.y = glm::sin(theta) * glm::sin(phi);
    res.z = glm::cos(theta);
    return res;
}

inline glm::vec3 spherical_to_cartesian_cos(flt cos_theta, flt cos_phi) {
    flt sin_theta = sqrtf(1 - cos_theta * cos_theta);
    flt sin_phi = sqrtf(1 - cos_phi * cos_phi);
    glm::vec3 res;
    res.x = sin_theta * cos_phi;
    res.y = sin_theta * sin_phi;
    res.z = cos_theta;
    return res;
}

inline flt random_float() {
    static std::uniform_real_distribution<flt> distribution(0.0, 1.0);
    static std::mt19937 generator;
    static std::function<flt()> rand_generator =
        std::bind(distribution, generator);
    return rand_generator();
}

inline flt random_range(flt min, flt max) {
    flt ran = random_float();
    ran = ran * (max - min) + min;
    return ran;
}

inline glm::vec3 random_in_unit_sphere() {
    while (true) {
        auto p = glm::vec3(random_range(-1, 1), random_range(-1, 1), random_range(-1, 1));
        if (glm::length(p) >= 1) continue;
        return p;
    }
}

inline glm::vec3 random_unit_vector() {
    auto a = random_range(0, 2 * pi);
    auto z = random_range(-1, 1);
    auto r = sqrt(1 - z * z);
    return glm::vec3(r * cos(a), r * sin(a), z);
}


// 这两个函数不管wi wo应该都和normal同向， 就是普通的计算反射折射，因此在调用时需要注意方向
inline glm::vec3 reflect(const glm::vec3& ray_in, const glm::vec3& normal) {
    glm::vec3 nrm = normal;
    if (glm::dot(ray_in, normal) > 0)
        nrm = -nrm;
    return ray_in - 2 * dot(ray_in, nrm) * nrm;
}

inline glm::vec3 refract(const glm::vec3& ray_in, const glm::vec3& normal, flt ior1, flt ior2)
{
    glm::vec3 nrm = normal;
    if (glm::dot(ray_in, normal) > 0)
        nrm = -nrm;
    flt cos_theta = fabs(glm::dot(ray_in, nrm));
    flt sin_theta = sqrtf(1 - cos_theta * cos_theta);

    flt sin_theta_sca = sin_theta * ior1 / ior2;
    flt cos_theta_sca = sqrtf(1 - sin_theta_sca * sin_theta_sca);

    // vertical to normal
    glm::vec3 vertical = ray_in + cos_theta * nrm;
    glm::vec3 scatter_vertical = vertical / sin_theta * sin_theta_sca;
    glm::vec3 scatter_parallel = -cos_theta_sca * nrm;
    return scatter_vertical + scatter_parallel;
}