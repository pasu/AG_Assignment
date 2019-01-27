/* One Static Geometry

The container of:
Texture,
Material(Diffuse, Glossy, ...)
Triangles in local memorys

*/
#pragma once

#include"assimp/mesh.h"

#include<glad/glad.h>

namespace gpurt {
    typedef  struct Vertex {
        aiVector3D pos;// use aiVector3D because it has no padding, sizeof(aiVector3D) = 12, thus the padding bytes can be used explicitly
        uint _padding1;
        aiVector3D normal;
        uint __padding2;
    }Vertex;

    typedef struct Triangle {
        Vertex v1, v2, v3;

        const vec3 centroid()const;

    }Triangle;
}