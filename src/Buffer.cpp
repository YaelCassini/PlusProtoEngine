// Author: Peiyao Li
// Date:   Mar 1 2023
#include "stb/stb_image.h"
#include "stb/stb_image_write.h"
#include "Buffer.hpp"

Buffer::Buffer(int w, int h) {
	this->init(w, h);
}

void Buffer::init(int w, int h)
{
    init(w, h, 1);
}

void Buffer::init(int w, int h, int spp)
{
    _width = w;
    _height = h;
    _samplePerPixel = spp;
    _data.resize(_height);
    for (auto& col : _data) {
        col.resize(_width);
    }
}

void Buffer::setSpp(int spp) {
    _samplePerPixel = spp; 
}

void Buffer::clear()
{
	for (auto& col : _data) {
		for (auto pixel : col)
		{
			pixel = glm::vec3(0, 0, 0);
		}
	}
}

void Buffer::setColor(int x, int y, glm::vec3 color)
{
    _data[y][x] = color;
}

void Buffer::addColor(int x, int y, glm::vec3 color)
{
    _data[y][x] += color;
}

void Buffer::renderToPic(const std::string& output_path, const flt gamma, int spp) const
{
    uchar* img = new uchar[_height * _width * picChannel];
    int pt = 0;
    for (int y_t = 0; y_t < _height; y_t++) {
        for (int x_t = 0; x_t < _width; x_t++) {
            glm::vec3 color = _data[y_t][x_t] / (spp * 1.0f);

            // Check if color is in range
            for (int i = 0; i < 3; i++)
            {
                color[i] = glm::clamp(color[i], flt(0.0), flt(1.0));
            }
            // Gamma correction
            for (int i = 0; i < 3; i++) {
                color[i] = powf(color[i], 1 / gamma);
            }

            for (int i = 0; i < 3; i++) {
                img[pt + i] = static_cast<uchar>(color[i] * 255);
            }
            pt = pt + 3;
        }
    }
    stbi_write_jpg(output_path.c_str(), _width, _height, picChannel, img, 100);
    delete[] img;
}