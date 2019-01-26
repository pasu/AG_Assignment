#include"precomp.h"
#include "GPURT_Scene.h"

using namespace gpurt;

gpurt::Scene::Scene() {
    _ssbo_triangles_ = 0;
}


int gpurt::Scene::triangleCount() {
    int number = 0;
    for (GeometryGroup* g: _groups_) {
        number += g->triangleCount();
    }
    return number;
}

void gpurt::Scene::load() {
    if (_ssbo_triangles_) {
        cout << "Warning: repeat uploading scene to GPU memory." << endl;
    }

    glGenBuffers(1, &_ssbo_triangles_);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, _ssbo_triangles_);
    glBufferData(GL_SHADER_STORAGE_BUFFER, triangleCount() * sizeof(Triangle), NULL, GL_STREAM_DRAW);

    int offset = 0;
    for (GeometryGroup* group : _groups_) {
        for (Geometry* geometry : group->_geometries_) {
            glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset, geometry->c_size(), geometry->c_pointer());
            offset += geometry->c_size();
        }
    }
    
}

void gpurt::Scene::bind() {
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, _ssbo_triangles_);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, _ssbo_triangles_);
}

void gpurt::Scene::unload() {
    if (glIsBuffer(_ssbo_triangles_)) {
        glDeleteBuffers(1, &_ssbo_triangles_);
    }
}


gpurt::Scene* gpurt::Scene::initScene1() {

    gpurt::Scene * scene = new gpurt::Scene();

    scene->_groups_.push_back(new gpurt::GeometryGroup("assets/GPURT/untitled.obj"));
    
    std::cout << "Object Count: " << scene->_groups_[0]->_geometries_.size() << std::endl;

    std::cout << "Triangle Count: "<< scene->triangleCount() << endl;

    return scene;
}