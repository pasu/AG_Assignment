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

#include"GPURT_Material.h"

namespace gpurt {

    class GeometryGroup {
    private:

        gpurt::BVH _bvh_;// mid-level merged bvh tree, size = geometryCount()*2-1

        vector<const gpurt::BVH*> _sub_bvh_list_;// a collection of the subbvhs 

        std::vector<Geometry*> _geometries_;

        vector<Mtl> _mtls_;

        void constructBVH();

        

    public:
        GeometryGroup(const char* file_name);
        
        const int triangleCount()const;
        
        const int geometryCount()const { return _geometries_.size(); }

        const std::vector<Geometry*>& geometries() { return _geometries_; }

        const int mtlCount()const { return _mtls_.size(); }

        const Mtl* mtlPointer()const { return &_mtls_[0]; }


        void copyBVH(BVHNode* dst, int& bvh_offset);

        void setOffset(int& triangle_id_offset, int& geometry_id_offset, int & material_id_offset);

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