// Scene
// several groups
// top level bvh

// manage Triangles in GPU memory

#pragma once
#include"GPURT_GeometryGroup.h"

namespace gpurt {
    class Scene {
    private:
        vector<GeometryGroup*> _groups_;

        GLuint _ssbo_triangles_;

    public:
        Scene();
        static Scene* initScene1();

        int triangleCount();

        void load();// alloc GPU memory

        void bind();// bind buffers

        void unload();// release GPU memory
    };
}