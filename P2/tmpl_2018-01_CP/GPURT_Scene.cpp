#include"precomp.h"
#include "GPURT_Scene.h"
#include"RTCameraController.h"

using namespace gpurt;

gpurt::Scene::Scene() {
    
    _ssbo_triangles_ = 0;
    _ssbo_bvh_ = 0;
}


int gpurt::Scene::triangleCount() {
    int number = 0;
    for (GeometryGroup* g: _groups_) {
        number += g->triangleCount();
    }
    return number;
}

void gpurt::Scene::upload() {
    if (_ssbo_triangles_) {
        cout << "Warning: repeat uploading scene to GPU memory." << endl;
    }

    // upload triangle vertices
    glGenBuffers(1, &_ssbo_triangles_);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, _ssbo_triangles_);
    glBufferData(GL_SHADER_STORAGE_BUFFER, triangleCount() * sizeof(Triangle), NULL, GL_STATIC_DRAW);

    int offset = 0;
    for (GeometryGroup* group : _groups_) {
        for (Geometry* geometry : group->geometries()) {
            glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset, geometry->c_size(), geometry->c_pointer());
            offset += geometry->c_size();
        }
    }

    // upload bvh
    glGenBuffers(1, &_ssbo_bvh_);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, _ssbo_bvh_);
    glBufferData(GL_SHADER_STORAGE_BUFFER, _merged_.c_size(), &_merged_[0], GL_STATIC_DRAW);

    // upload material
    glGenBuffers(1, &_ssbo_material_);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, _ssbo_material_);
    int material_count = 0;
    for (const auto& g : _groups_) {
        material_count += g->mtlCount();
    }
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(Mtl)*material_count, NULL, GL_STATIC_DRAW);
    offset = 0;
    for (auto& g : _groups_) {
        int size = g->mtlCount() * sizeof(Mtl);
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset, size, g->mtlPointer());
        offset += size;
    }


    _vplg_.init(*_groups_[0]);

    glGenBuffers(1, &_ssbo_nee_triangle_id_);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, _ssbo_nee_triangle_id_);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(_vplg_.light_triangle_id),& _vplg_.light_triangle_id[0], GL_STREAM_DRAW);
}

void gpurt::Scene::bind() {
    current = this;
    camera._moved_ = true;
    // hard code
    // SSBO 0: screen pixels
    // SSBO 1: triangle vertices
    // SSBO 2: bvh
    // SSBO 3: material
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, _ssbo_triangles_);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, _ssbo_bvh_);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, _ssbo_material_);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, _ssbo_nee_triangle_id_);

}

void gpurt::Scene::unload() {
    if (glIsBuffer(_ssbo_triangles_)) {
        glDeleteBuffers(1, &_ssbo_triangles_);
    }
}

void gpurt::Scene::frame() {

    _vplg_.shuffel();


    glBindBuffer(GL_SHADER_STORAGE_BUFFER, _ssbo_nee_triangle_id_);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER,0, sizeof(_vplg_.light_triangle_id), &_vplg_.light_triangle_id[0]);

    updateCamera(camera);
    camera.copyMCamera(_m_camera_);
    _camera_moved_ = camera.moved();
}

void gpurt::Scene::mergeBVH() {

    _bvh_size_ = 0;

    for (GeometryGroup* group : _groups_) {
        _bvh_size_ += group->bvhSize();
    }

    _merged_._bvh_.resize(_bvh_size_);

    int bvh_offset = 0;
    for (GeometryGroup* g : _groups_) {
        g->copyBVH(&_merged_[0], bvh_offset);
    }

    updateTopLevelBVH();

}

void gpurt::Scene::setOffset() {
    int triangle_offset = 0;
    int geometry_offset = 0;
    int material_offset = 0;

    for (GeometryGroup* g : _groups_) {
        g->setOffset(triangle_offset,geometry_offset, material_offset);
    }
}

void gpurt::Scene::updateTopLevelBVH() {
    
}

gpurt::Scene* gpurt::Scene::initScene(const char* filename) {

    gpurt::Scene * scene = new gpurt::Scene();


    std::cout << filename << endl;
    auto group = new gpurt::GeometryGroup(filename);
    scene->_groups_.push_back(group);

    
    std::cout << "Object Count: " << scene->_groups_[0]->geometryCount() << std::endl;

    std::cout << "Triangle Count: "<< scene->triangleCount() << endl;
    scene->camera.moveUp(1.5);
    scene->init();
    scene->upload();
    return scene;
}

void gpurt::Scene::init() {
    setOffset();
    mergeBVH();
}

gpurt::Scene* gpurt::Scene::current = nullptr;