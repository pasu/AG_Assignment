// Scene
// several groups
// top level bvh

// manage Triangles in GPU memory

#pragma once
#include"GPURT_GeometryGroup.h"
#include"GPURT_VirtualPointLight.h"
#include"RTCamera.h"

namespace gpurt {
    class Scene {
    private:
        RTCamera camera;

        vector<GeometryGroup*> _groups_;

        
        gpurt::BVH _merged_;

        mat4 _m_camera_;
        bool _camera_moved_;

        GLuint _ssbo_triangles_;
        GLuint _ssbo_bvh_;
        GLuint _ssbo_material_;
        GLuint _ssbo_nee_triangle_id_;

        int _bvh_size_;

        VirtualPointLightinitGemnerator _vplg_;

        void mergeBVH();
        void setOffset();
    public:
        Scene();
        static Scene* initScene(const char* filename);

        void init();

        void updateTopLevelBVH();

        int triangleCount();

        int geometryGroupCount()const { return _groups_.size(); }

        void upload();// alloc GPU memory

        void bind();// bind buffers

        void unload();// release GPU memory

        int getBVHSize()const { return _bvh_size_; };

        void frame();
        int cameraMoved()const { return _camera_moved_; }
        const float* mCamera()const { return &_m_camera_.cell[0]; }

        static Scene* current;
    };
}