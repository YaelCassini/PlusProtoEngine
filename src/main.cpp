// Author: Peiyao Li
// Date:   Mar 1 2023

#include "Global.hpp" 
#include "Scene.hpp"
#include <io.h>
#include <direct.h>

int main() {
    int maxDepth = 6;
    int numSamplePerPixel = 4096;
    std::string output = "test.jpg";
    
    std::string sceneDir = "./scene/myscene/";
    std::string sceneName = "myscene";
    std::string objName = "myscene";


    std::cout << "Please input the scene folder..." << std::endl;
    std::cin >> sceneDir;
    std::cout << "Please input the scene name..." << std::endl;
    std::cin >> sceneName;
    std::cout << "Please input the obj name..." << std::endl;
    std::cin >> objName;
    std::cout << "Please input the sample per pixel..." << std::endl;
    std::cin >> numSamplePerPixel;
    std::cout << "Please input the max bounce depth..." << std::endl;
    std::cin >> maxDepth;

    //判断输出文件夹是否存在，不存在则创建
    const char* dir = "./output/";
    if (_access(dir, 0) == -1)
    {
        _mkdir(dir);
    }

    Scene scene(sceneDir, sceneName, objName);
    scene.render(output, numSamplePerPixel, maxDepth);

    // read xml & obj & mtl firstly, then add other objects
    //shared_ptr<Material> mat = std::make_shared<PhongMaterial>();
    //Hittable* sphere = new Sphere(glm::vec3(0, 0, -1), 0.5, mat);
    //Hittable* sphere02 = new Sphere(glm::vec3(0, -100.5, -1), 100, mat);
    //scene.addObject(sphere);
    //scene.addObject(sphere02);

    //delete sphere;
    //delete sphere02;
    return 0;
}



