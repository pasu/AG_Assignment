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

        int _id_;
        int _id_offset_;// object id offset

        int _material_id_;
        int _material_id_offset_;

        int _triangle_id_offset_;

        void constructBVH();

    public:

        Geometry(const aiMesh& mesh, int geometry_id);

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

        void setOffset(int& triangle_id_offset, const int geometry_id_offset,const int material_id_offset );

        int bvhSize()const;

        const int getMtlID()const { return _material_id_; }

        int getTriangleOffset()const { return _triangle_id_offset_; }

    };

}