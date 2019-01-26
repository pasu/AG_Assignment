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
#include"GPURT_Triangle.h"

namespace gpurt {

    class Geometry {
    private:
        vector<Triangle> triangles;
        vector<gpurt::BVHNode> _bvh_;

    public:

        Geometry(const aiMesh& mesh);

        void constructBVH();

        const int triangleCount() {
            return triangles.size();
        }

        const void const* c_pointer() const {
            return &triangles[0];
        }

        const int c_size()const {
            return triangles.size() * sizeof(Triangle);
        }



    };

}