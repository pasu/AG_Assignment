/* One Static Geometry

The container of:
Texture,
Material(Diffuse, Glossy, ...)
Triangles in local memorys

*/
#pragma once

#include"assimp/mesh.h"

#include<glad/glad.h>
#include"GPURT_BVH.h"

namespace gpurt {
    typedef  struct Vertex {
        aiVector3D pos;
        uint _padding1;
        aiVector3D normal;
        uint __padding2;
    }Vertex;

    typedef struct Triangle {
        Vertex v1, v2, v3;
    }Triangle;
}