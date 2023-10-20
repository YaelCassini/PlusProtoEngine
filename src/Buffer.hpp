// Author: Peiyao Li
// Date:   Mar 1 2023
#pragma once
#include "Global.hpp"

class Buffer
{
private:
    int _width, _height;
    int _samplePerPixel = 1;
    std::vector<std::vector<glm::vec3>> _data;

public:
    Buffer() {};
    Buffer(int w, int h);
    void init(int w, int h);
    void init(int w, int h, int spp);
    void clear();
    void setSpp(int spp);
    void setColor(int x, int y, glm::vec3 color);
    void addColor(int x, int y, glm::vec3 color);
    void renderToPic(const std::string& pic_path, const flt gamma, int spp) const;

    inline int getWidth() const { return _width; }
    inline int getHeight() const { return _height; }
};