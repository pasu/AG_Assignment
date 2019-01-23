// A group of static geometries
// But a Geometry group can move as a whole

// One geometry group is defined in one pair of (*.obj, *.mtl) files, and several textures

// Will load *.obj, *.mtl, and textures, then hand them to gpurt::Geometry.

#pragma once

#include <assimp/Importer.hpp>
#include <assimp/mesh.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

namespace gpurt {
    void loadGeometryGroup();
}