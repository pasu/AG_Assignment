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

        
        gpurt::BVH _merged_;

        GLuint _ssbo_triangles_;
        GLuint _ssbo_bvh_;

        int _bvh_size_;


        void constructBVH();
        void setTriangleOffset();

    public:
        Scene();
        static Scene* initScene1();

        void init();

        void updateTopLevelBVH();

        int triangleCount();

        int geometryGroupCount()const { return _groups_.size(); }

        void upload();// alloc GPU memory

        void bind();// bind buffers

        void unload();// release GPU memory

        int getBVHSize()const { return _bvh_size_; };
    };
}