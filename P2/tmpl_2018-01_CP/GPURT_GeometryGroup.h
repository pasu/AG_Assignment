// A group of static geometries
// A Geometry group can move as a whole

// One geometry group is defined in one pair of (*.obj, *.mtl) files, and several textures

// Will load *.obj, *.mtl, and textures, then hand them to gpurt::Geometry.

#pragma once

#include <assimp/Importer.hpp>
#include <assimp/mesh.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include"GPURT_Geometry.h"
#include<vector>

namespace gpurt {

    class GeometryGroup {
    private:

        gpurt::BVH _bvh_;// mid-level merged bvh tree, size = geometryCount()*2-1

        vector<const gpurt::BVH*> _sub_bvh_list_;

        std::vector<Geometry*> _geometries_;

        void constructBVH();

    public:
        GeometryGroup(const char* file_name);
        int triangleCount();
        const int geometryCount()const { return _geometries_.size(); }

        const std::vector<Geometry*>& geometries() { return _geometries_; }

        void copyBVH(BVHNode* dst, int& bvh_offset);

        void setTriangleOffset(int& offset);

        int bvhSize();

    };

    // Object is a geometry group + a rigid motion
    // This gives ability of replication
    class Object {
    private:
        GeometryGroup* _gg_;
    public:

    };

}