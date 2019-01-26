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
        int _triangle_offset_;
    public:
        std::vector<Geometry*> _geometries_;
        GeometryGroup(const char* file_name);
        int triangleCount();
    };

    // Object is a geometry group + a rigid motion
    // This gives ability of replication
    class Object {
    private:
        GeometryGroup* _gg_;
    public:

    };

}