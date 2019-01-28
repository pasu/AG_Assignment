#include"precomp.h"
#include "GPURT_Scene.h"

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
    
}

void gpurt::Scene::bind() {

    // hard code
    // SSBO 0: screen pixels
    // SSBO 1: triangle vertices
    // SSBO 2: bvh
    // SSBO 3: material
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, _ssbo_triangles_);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, _ssbo_bvh_);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, _ssbo_material_);

}

void gpurt::Scene::unload() {
    if (glIsBuffer(_ssbo_triangles_)) {
        glDeleteBuffers(1, &_ssbo_triangles_);
    }
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

gpurt::Scene* gpurt::Scene::initScene1() {

    gpurt::Scene * scene = new gpurt::Scene();

    scene->_groups_.push_back(new gpurt::GeometryGroup("assets/GPURT/room.obj"));
    
    std::cout << "Object Count: " << scene->_groups_[0]->geometryCount() << std::endl;

    std::cout << "Triangle Count: "<< scene->triangleCount() << endl;

    return scene;
}

void gpurt::Scene::init() {
    setOffset();
    mergeBVH();
}
