// Author: Peiyao Li
// Date:   Mar 1 2023

#include "Scene.hpp"
Scene::Scene(std::string& scenepath, std::string& scenename, std::string& objname)
{
    buildScene(scenepath, scenename, objname);
}

void Scene::buildScene(std::string& scenepath, std::string& scenename, std::string& objname)
{
    Timer timer;
    timer.start();
    // read camera & light radiance from xml
    tinyxml2::XMLDocument xmlDocument;    
    std::string xmlpath = scenepath + scenename + ".xml";
    tinyxml2::XMLError error = xmlDocument.LoadFile(xmlpath.c_str());
    if (error != tinyxml2::XML_SUCCESS) {
        ERRORM("Failed to Read XML: %s", xmlDocument.ErrorStr());
    }
    this->cam.initFromXML(xmlDocument); 
    std::map<std::string, glm::vec3> light_radiance;
    readRadiances(xmlDocument, light_radiance);   
    
    // read obj file (contain mesh & mat name)
    tinyobj::ObjReader reader;
    readMeshes(scenepath + objname + ".obj", reader);
    auto& shapes = reader.GetShapes();
    auto& attrib = reader.GetAttrib();
    auto& material_list = reader.GetMaterials();

    int num_faces = 0;
    for (int i = 0; i < shapes.size(); i++) {
        num_faces += shapes[i].mesh.num_face_vertices.size();
    }
    INFO("Scene Face Count: %d\n", num_faces);

    // read materials & textures
    readMaterials(scenepath, material_list, light_radiance);
    saveToScene(shapes, attrib);

    // build buffer & default material
    this->buf.init(cam.getWidth(), cam.getHeight());
    default_mat = static_cast<shared_ptr<Material>>(
        std::make_shared<PhongMaterial>());
    default_mat->_kd = glm::vec3(0.5, 0.5, 0.5);
    INFO("Render Image Size: %d x %d (W x H)\n", this->cam.getWidth(), this->cam.getHeight());

    timer.end();
    timer.printTimeCost("Read Scene");

    // build bvh tree
    timer.start();
    DEBUGM("Begin Build BVH\n");
    this->bvh_tree.buildTree(bvh_tree.getObjects());
    DEBUGM("Finish Build BVH\n");
    timer.end();
    timer.printTimeCost("Build BVH Tree");
    
}

void Scene::addObject(Hittable* obj)
{ 
    bvh_tree.getObjects().push_back(obj); 
}

void Scene::addMaterial(shared_ptr<Material> mat)
{
    materials.push_back(mat);
}

void Scene::render(std::string& output, int spp, int maxdepth)
{
    buf.setSpp(spp);

    for (int s = 0; s < spp; s++)
    {
        INFO("Render Sample %d\n", s + 1);
        for (int j = 0; j < cam.getHeight(); j++) {
            INFO("Height %d\r", j);
#pragma omp parallel for num_threads(8)
            for (int i = 0; i < cam.getWidth(); ++i) {
                Ray ray_sample = cam.genRayRandom(i, j);
                glm::vec3 color = Li(ray_sample, bvh_tree.getObjects(), maxdepth);

                if (std::isfinite(color[0]) && std::isfinite(color[1]) && std::isfinite(color[2])) {
                    buf.addColor(i, j, color);
                }
                else {
                    DEBUGM("Not finite number at sample %d x %d y %d\n", s, i, j);
                }
            }
        }
    if((s+1)==1||(s+1)==4|| (s + 1) == 8|| (s + 1) == 16|| (s + 1) == 64|| (s + 1) == 128|| (s + 1) == 256|| (s + 1) == 512|| (s + 1) == 1024|| (s + 1) == 2048|| (s + 1) == 4096)
    buf.renderToPic("./output/spp_"+std::to_string(s+1)+".jpg", 2.2, s+1);
    }
    buf.renderToPic("test.jpg", 2.2, spp);
    return;
}

glm::vec3 Scene::Li(Ray& r, std::vector<Hittable*>& objects, int depth)
{
    glm::vec3 color(0.0f);
    glm::vec3 throughput(1.0f);
    glm::vec3 wo, wi;
    Ray ray = r;
    HitRecord rec;
    bool look_light = true;
    int bounce = 0;
    int in_glass = 0;
    for (bounce = 0; bounce < depth; bounce++) {  
        
        if (!bvh_tree.hit(ray, kHitEps, INFINITY, rec)) {
            break; // No intersection
        }
        
        if (!rec._mat) {
            rec._mat = default_mat; // default phong material
        }

        //color = rec.mat->kd;
        //break;
        
        // material self emissive
        if (rec._mat->_type == MatType::LIGHT) {
            if (glm::dot(rec._normal, ray.getDirection()) < 0)
            {
                if (look_light)
                {
                    //printVec3(color);
                    glm::vec3 emissive_color = rec._mat->_ke;
                    color += throughput * emissive_color;
                }
            }
            //DEBUGM("Bounce %d: Light color: %f %f %f\n", bounce, color[0], color[1], color[2]);
            // 遇到光源后是否继续bounce

            break;
        }

        wo = -ray.getDirection();
        // material glass
        if (rec._mat->_type == MatType::GLASS) {
            in_glass = in_glass + 1;
            /*DEBUGM("Bounce %d Glass", bounce);*/
            //break;
            Ray scattered;
            flt attenuation = rec._mat->scatter(ray, rec, scattered);
            wi = scattered.getDirection();
            throughput *= rec._mat->bsdf(wi, rec, wo);
            ray = scattered;
            //DEBUGM("Bounce %d: Glass color: %f %f %f\n", bounce, color[0], color[1], color[2]);

            continue;
        }

        look_light = false;
        rec._normal = glm::dot(rec._normal, wo) > 0 ? rec._normal : -rec._normal;
        color += throughput * sampleLight(ray, rec);

        Ray scattered;      
        flt pdf = rec._mat->scatter(ray, rec, scattered);
        wi = scattered.getDirection();
        if (glm::dot(wi, rec._normal) > 0 && pdf > kEps) {
            flt cos = fabs(glm::dot(wi, rec._normal));
            throughput *= rec._mat->bsdf(wi, rec, wo) * cos / pdf;
            ray = scattered;        
        }
        else {
            break;
        }

        if (bounce >= 3)
        {
            flt ran = random_float();
            if (ran < glm::compMax(throughput))
                throughput /= glm::compMax(throughput);
            else
                break;
        }

        //DEBUGM("Bounce %d: color: %f %f %f\n", bounce, color[0], color[1], color[2]);

    }
    //DEBUGM("Return: Bounce %d: color: %f %f %f\n", bounce, color[0], color[1], color[2]);
    return color;
}

glm::vec3 Scene::sampleLight(Ray& ray, HitRecord& rec)
{
    Ray light_ray;
    HitRecord light_rec;    
    glm::vec3 wi;
    glm::vec3 wo = -ray.getDirection();
    flt light_pdf, bsdf_pdf;
    
    glm::vec3 light_color;
    glm::vec3 color(0.0f);

    // sample light
    light_pdf = egroup.sampleRay(&bvh_tree, rec, light_ray, light_rec);
    wi = light_ray.getDirection();
    bsdf_pdf = rec._mat->pdf(wi, rec, wo);
    if (light_pdf > kEps && bsdf_pdf > kEps) {
        flt weight = light_pdf / (light_pdf + bsdf_pdf);
        //flt weight = 1.0;
        flt cos = glm::dot(wi, rec._normal);
        glm::vec3 bsdf = rec._mat->bsdf(wi, rec, wo);
        if (light_rec._mat) {
            light_color = light_rec._mat->_ke;
        }
        else {
            ERRORM("The light has no material.");
        }
        color += weight * light_color * bsdf * cos / light_pdf;
    }

    // sample bsdf
    bsdf_pdf = rec._mat->scatter(ray, rec, light_ray);
    wi = light_ray.getDirection();
    // sample hit light
    bool flaghit = bvh_tree.hit(light_ray, kHitEps, FLT_MAX, light_rec);
    bool flaghitlight = false;
    bool flaglightdirect = false;
    if (flaghit)
    {
        flaghitlight = light_rec._mat->_type == LIGHT;
        flaglightdirect = glm::dot(light_ray.getDirection(), light_rec._normal) < 0;
    }

    if (flaghit && flaghitlight && flaglightdirect)
    {
        light_pdf = egroup.pdf(rec, light_rec);
        if (light_pdf > kEps && bsdf_pdf > kEps) {
            flt weight = bsdf_pdf / (light_pdf + bsdf_pdf);
            //flt weight = 1.0;
            flt cos = glm::dot(wi, rec._normal);
            glm::vec3 bsdf = rec._mat->bsdf(wi, rec, wo);

            if (light_rec._mat) {
                light_color = light_rec._mat->_ke;
            }
            else {
                ERRORM("The light has no material.");
            }
            color += weight * light_color * bsdf * cos / bsdf_pdf;
        }
    }
    
    //color = glm::vec3(flaghit);
    return color;
}

void Scene::readRadiances(
    const tinyxml2::XMLDocument& xmlconfig,
    std::map<std::string, glm::vec3>& light_radiance)
{
    auto lightNode = xmlconfig.FirstChildElement("light");
    if (!lightNode) {
        ERRORM("No light source found in xml file\n");
    }
    while (lightNode) {
        auto mtl = lightNode->Attribute("mtlname");
        if (!mtl) {
            ERRORM("light has no attribute name \"mtlname\"\n");
        }
        std::string mtlname(mtl);
        auto radiance_str = lightNode->Attribute("radiance");
        if (!radiance_str) {
            ERRORM("light has no attribute name \"radiance\"\n");
        }
        glm::vec3 radiance;
        if (sscanf_s(radiance_str, "%f,%f,%f", &radiance[0], &radiance[1], &radiance[2]) != 3) {
            ERRORM("cannot read 3 floats in radiance attribute\n");
        }

        light_radiance.insert(std::make_pair(mtlname, radiance));
        DEBUGM("light: %s  radiance: %f %f %f\n", mtlname.c_str(), radiance[0], radiance[1], radiance[2]);

        lightNode = lightNode->NextSiblingElement("light");
    }
}

void Scene::readMeshes(const std::string& inputfile,
    tinyobj::ObjReader& objreader)
{
    tinyobj::ObjReaderConfig objreader_config;

    if (!objreader.ParseFromFile(inputfile, objreader_config)) {
        if (!objreader.Error().empty()) {
            ERRORM("TinyObjReader %s\n", objreader.Error().c_str());
        }
        ERRORM("TinyObjReader error\n");
    }

    if (!objreader.Warning().empty()) {
        INFO("TinyObjReader %s\n", objreader.Warning().c_str());
    }
}

void Scene::readMaterials(
    const std::string& objectdir,
    const std::vector<tinyobj::material_t>& material_info,
    std::map<std::string, glm::vec3>& light_radiance)
{
    for (const auto& material_loader : material_info) {
        shared_ptr<Material> material;
        if (material_loader.ior > 1.0f)
        {
            material = make_shared<GlassMaterial>();
            material->_type = GLASS;
        }
        else
        {
            material = make_shared<PhongMaterial>();
            material->_type = DIFFUSE;
        }
        material->_name = material_loader.name;

        // Kd
        for (int i = 0; i < 3; i++) {
            material->_kd[i] = material_loader.diffuse[i];
        }

        // Ks
        for (int i = 0; i < 3; i++) {
            material->_ks[i] = material_loader.specular[i];
        }

        // tr
        for (int i = 0; i < 3; i++) {
            material->_tr[i] = material_loader.transmittance[i];
        }

        // brightness and refraction
        material->_ns = material_loader.shininess;
        material->_ni = material_loader.ior;

        // if a material give out light
        if (light_radiance.count(material_loader.name)) {
            glm::vec3 emissive = light_radiance[material_loader.name];
            material->_ke = emissive;
            material->_is_emissive = true;
            material->_type = LIGHT;
        }

        // if a material has texture
        if (material_loader.diffuse_texname.length() > 0) {
            material->_has_texture = true;
            auto& texture = material->_texture;
            int load_channel;
            int texture_width, texture_height;
            flt* texture_data = stbi_loadf((objectdir + material_loader.diffuse_texname).c_str(),
                &texture_width, &texture_height, &load_channel, picChannel);
            if (!texture_data) {
                ERRORM("Cannot load texture %s\n", material_loader.diffuse_texname.c_str());
            }
            if (load_channel != 3) {
                ERRORM("The channel of input texture is not equal to 3\n");
            }

            texture.setSize(texture_width, texture_height);
            texture.setData(texture_data);

        }

        DEBUGM("Material Name: %s Type: %d\nKd: %f %f %f Ks: %f %f %f\nTr: %f %f %f Ke: %f %f %f\nNs: %f Ni: %f Has_Tex: %d\n",
            material->_name.c_str(), material->_type,
            material->_kd[0], material->_kd[1], material->_kd[2],
            material->_ks[0], material->_ks[1], material->_ks[2],
            material->_tr[0], material->_tr[1], material->_tr[2],
            material->_ke[0], material->_ke[1], material->_ke[2],
            material->_ns, material->_ni,
            material->_has_texture);

        addMaterial(static_cast<shared_ptr<Material>>(material));
    }
    INFO("Material Count: %d\n", materials.size());
}

void Scene::saveToScene(
    const std::vector<tinyobj::shape_t>& shapes,
    const tinyobj::attrib_t& attrib)
{
    std::vector<shared_ptr<Emissive>> light_objects;

    DEBUGM("Object Count: %d\n", shapes.size());
    for (size_t s = 0; s < shapes.size(); s++) {
        size_t point_index_offset = 0;
        DEBUGM("Object %d : Face Count: %d\n", s, shapes[s].mesh.num_face_vertices.size());
        for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
            size_t fv = shapes[s].mesh.num_face_vertices[f];
            if (fv != 3) {
                ERRORM("Polygon not triangulated at object %zu face %zu\n", s, f);
            }
            glm::vec3 vp[3];
            glm::vec3 vn[3];
            glm::vec2 vt[3];
            bool has_uv = false;
            bool has_normal = false;
            for (size_t v = 0; v < fv; v++) {
                tinyobj::index_t idx = shapes[s].mesh.indices[point_index_offset + v];
                if (3 * size_t(idx.vertex_index) + 2 >= attrib.vertices.size())
                    ERRORM("vectices id exceed %d\n", 3 * size_t(idx.vertex_index) + 2);
                tinyobj::real_t vx = attrib.vertices[3 * size_t(idx.vertex_index) + 0];
                tinyobj::real_t vy = attrib.vertices[3 * size_t(idx.vertex_index) + 1];
                tinyobj::real_t vz = attrib.vertices[3 * size_t(idx.vertex_index) + 2];
                vp[v] = glm::vec3(vx, vy, vz);

                if (idx.normal_index >= 0) {
                    has_normal = true;
                    if (3 * size_t(idx.normal_index) + 2 >= attrib.normals.size())
                        ERRORM("normal id exceed %d\n", 3 * size_t(idx.normal_index) + 2);

                    tinyobj::real_t nx = attrib.normals[3 * size_t(idx.normal_index) + 0];
                    tinyobj::real_t ny = attrib.normals[3 * size_t(idx.normal_index) + 1];
                    tinyobj::real_t nz = attrib.normals[3 * size_t(idx.normal_index) + 2];
                    vn[v] = glm::vec3(nx, ny, nz);
                }

                if (idx.texcoord_index >= 0) {
                    has_uv = true;
                    if (2 * size_t(idx.texcoord_index) + 1 >= attrib.texcoords.size())
                        ERRORM("texcoord id exceed %d\n", 2 * size_t(idx.texcoord_index) + 1);

                    tinyobj::real_t tx = attrib.texcoords[2 * size_t(idx.texcoord_index) + 0];
                    tinyobj::real_t ty = attrib.texcoords[2 * size_t(idx.texcoord_index) + 1];
                    vt[v] = glm::vec2(tx, ty);
                }
            }
            point_index_offset += fv;

            Triangle* tri = new Triangle(vp[0], vp[1], vp[2]);
            if (has_normal) {
                tri->setVertexNormal(vn[0], vn[1], vn[2]);
                if (glm::dot(vn[0], tri->getFaceNormal()) < 0) {
                    tri->reverseFaceNormal();
                }
            }
            if (has_uv) {
                tri->setVertexTexCoord(vt[0], vt[1], vt[2]);
            }
            int material_id = shapes[s].mesh.material_ids[f];
            if (material_id >= materials.size())
                ERRORM("material_id exceed %d\n", material_id);
            tri->_mat = materials[material_id];
            addObject(static_cast<Hittable*>(tri));

            // light triangles
            if (tri->_mat->_type == LIGHT) {
                light_objects.push_back(static_cast<shared_ptr<Emissive>>(tri));
            }
        }
    }

    this->egroup.init(light_objects);
    INFO("Build Light Groups.\n");
}
