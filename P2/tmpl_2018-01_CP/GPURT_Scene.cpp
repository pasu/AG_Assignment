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
    glBufferData(GL_SHADER_STORAGE_BUFFER, triangleCount() * sizeof(Triangle), NULL, GL_STREAM_DRAW);

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
    
}

void gpurt::Scene::bind() {

    // hard code
    // SSBO 0: screen pixels
    // SSBO 1: triangle vertices
    // SSBO 2: bvh
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, _ssbo_triangles_);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, _ssbo_bvh_);

}

void gpurt::Scene::unload() {
    if (glIsBuffer(_ssbo_triangles_)) {
        glDeleteBuffers(1, &_ssbo_triangles_);
    }
}

void gpurt::Scene::constructBVH() {

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

void gpurt::Scene::setTriangleOffset() {
    int offset = 0;
    for (GeometryGroup* g : _groups_) {
        g->setTriangleOffset(offset);
    }
}

void gpurt::Scene::updateTopLevelBVH() {
    
}

gpurt::Scene* gpurt::Scene::initScene1() {

    gpurt::Scene * scene = new gpurt::Scene();

    scene->_groups_.push_back(new gpurt::GeometryGroup("assets/GPURT/house_interior/room.obj"));
    
    std::cout << "Object Count: " << scene->_groups_[0]->geometryCount() << std::endl;

    std::cout << "Triangle Count: "<< scene->triangleCount() << endl;

    return scene;
}

void gpurt::Scene::init() {
    setTriangleOffset();
    constructBVH();
}

