// Author: Peiyao Li
// Date:   Mar 1 2023
#include "Camera.hpp"
#include "tinyxml2.h"

using namespace tinyxml2;
#define _CRT_SECURE_NO_WARNINGS
using namespace std;


void Camera::initFromXML(const tinyxml2::XMLDocument& xmlconfig)
{
    const XMLNode* camXMLNode = xmlconfig.FirstChildElement("camera");
    if (!camXMLNode) {
        ERRORM("No camera founded in the xml file\n");
    }
    auto camElement = camXMLNode->ToElement();

    camXMLNode->FirstChildElement("eye")->ToElement()->QueryFloatAttribute("x", &(_pos[0]));
    camXMLNode->FirstChildElement("eye")->ToElement()->QueryFloatAttribute("y", &(_pos[1]));
    camXMLNode->FirstChildElement("eye")->ToElement()->QueryFloatAttribute("z", &(_pos[2]));

    camXMLNode->FirstChildElement("lookat")->ToElement()->QueryFloatAttribute("x", &(_lookat[0]));
    camXMLNode->FirstChildElement("lookat")->ToElement()->QueryFloatAttribute("y", &(_lookat[1]));
    camXMLNode->FirstChildElement("lookat")->ToElement()->QueryFloatAttribute("z", &(_lookat[2]));

    camXMLNode->FirstChildElement("up")->ToElement()->QueryFloatAttribute("x", &(_up[0]));
    camXMLNode->FirstChildElement("up")->ToElement()->QueryFloatAttribute("y", &(_up[1]));
    camXMLNode->FirstChildElement("up")->ToElement()->QueryFloatAttribute("z", &(_up[2]));

    camElement->QueryFloatAttribute("fovy", &(_fovy));
    camElement->QueryIntAttribute("width", &(_width));
    camElement->QueryIntAttribute("height", &(_height));

    precomputeCamera();
}


void Camera::precomputeCamera()
{
    DEBUGM("Camera arguments:\n");
    DEBUGM("type: %d\n", _type);
    DEBUGM("eye: %f %f %f\n", _pos[0], _pos[1], _pos[2]);
    DEBUGM("lookat: %f %f %f\n", _lookat[0], _lookat[1], _lookat[2]);
    DEBUGM("up: %f %f %f\n", _up[0], _up[1], _up[2]);
    DEBUGM("fovy: %f \n", _fovy);

    _aspect = 1.0f * _width / _height;

    // Default focal_length = 1
    flt viewport_height = 2 * _focal_length * tan(glm::radians(_fovy * 0.5f));
    flt viewport_width = viewport_height * _aspect;

    DEBUGM("Viewport size: %f %f\n", viewport_width, viewport_height);

    glm::vec4 left_top_corner_origin = glm::vec4(-viewport_width / 2, viewport_height / 2, -_focal_length, 1);
    glm::vec4 right_top_corner_origin = glm::vec4(viewport_width / 2, viewport_height / 2, -_focal_length, 1);
    glm::vec4 left_bottom_corner_origin = glm::vec4(-viewport_width / 2, -viewport_height / 2, -_focal_length, 1);

    auto lookat_mat_inv = glm::inverse(glm::lookAt(_pos, _lookat, _up));

    _left_top_pos = glm::vec3(lookat_mat_inv * left_top_corner_origin);
    glm::vec3 right_top_corner = glm::vec3(lookat_mat_inv * right_top_corner_origin);
    glm::vec3 left_bottom_corner = glm::vec3(lookat_mat_inv * left_bottom_corner_origin);

    _dposw = (right_top_corner - _left_top_pos) / (1.0f * _width);
    _dposh = (left_bottom_corner - _left_top_pos) / (1.0f * _height);

    DEBUGM("left_top_pos: %f %f %f\n", _left_top_pos[0], _left_top_pos[1], _left_top_pos[2]);
    DEBUGM("dw: %f %f %f\n", _dposw[0], _dposw[1], _dposw[2]);
    DEBUGM("dh: %f %f %f\n", _dposh[0], _dposh[1], _dposh[2]);
}


Ray Camera::genRay(int x, int y)
{
    flt xpos = flt(x + 0.5);
    flt ypos = flt(y + 0.5);
    Ray ray(_pos, _left_top_pos + glm::vec3(xpos * _dposw + ypos * _dposh) - _pos);
    return ray;
}

Ray Camera::genRayRandom(int x, int y)
{
    flt xpos = flt(x + random_float());
    flt ypos = flt(y + random_float());
    glm::vec3 dir = _left_top_pos + glm::vec3(xpos * _dposw + ypos * _dposh) - _pos;
    Ray ray(_pos, _left_top_pos + glm::vec3(xpos * _dposw + ypos * _dposh) - _pos);
    return ray;
}


