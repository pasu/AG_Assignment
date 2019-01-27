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
        vector<Triangle> _triangles_;
        gpurt::BVH _bvh_;

        int _material_group_;

        void constructBVH();

    public:

        Geometry(const aiMesh& mesh);

        const int triangleCount() const{
            return _triangles_.size();
        }


        const void const* c_pointer() const {
            return &_triangles_[0];
        }

        const int c_size()const {
            return _triangles_.size() * sizeof(Triangle);
        }

        const gpurt::BVH& getBVH()const { return _bvh_; }

        const void copyBVH(BVHNode* dst, int& bvh_offset)const;

        void setTriangleOffset(int& offset);

        int bvhSize()const;

        void setMaterialGroup(int materialgroup);

    };

}