#include "precomp.h"
#include "GPURT_Geometry.h"
#include<assimp\mesh.h>

using namespace gpurt;

gpurt::Geometry::Geometry(const aiMesh & mesh, int geometry_id) {

    _material_id_ = mesh.mMaterialIndex;
    _id_ = geometry_id;

    _triangles_.reserve(mesh.mNumFaces);

    for (unsigned int faceI = 0; faceI < mesh.mNumFaces; ++faceI) {
        const aiFace & face = mesh.mFaces[faceI];

        unsigned int i1 = face.mIndices[0];
        unsigned int i2 = face.mIndices[1];
        unsigned int i3 = face.mIndices[2];

        Triangle t = {
            {
                mesh.mVertices[i1],0,
                mesh.mNormals[i1]
            },
            {   
                mesh.mVertices[i2],0,
                mesh.mNormals[i2]
            },
            {
                mesh.mVertices[i3],0,
                mesh.mNormals[i3]
            }
        };
        _triangles_.push_back(t);
    }

    constructBVH();
}

void gpurt::Geometry::constructBVH() {
    _bvh_.construct(_triangles_);
}

const void gpurt::Geometry::copyBVH(BVHNode * dst, int & bvh_offset) const {
    _bvh_.copy(dst, bvh_offset);
}

void gpurt::Geometry::setOffset(int & triangle_id_offset, const int geometry_id_offset, const int material_id_offset) {
    _triangle_id_offset_ = triangle_id_offset;

    for (int i = 0; i < bvhSize(); i++) {
        if (_bvh_[i].isLeaf()) {
            _bvh_[i].left_first += triangle_id_offset;
        }
    }
    triangle_id_offset += triangleCount();

    _id_offset_ = geometry_id_offset;
    _material_id_offset_ = material_id_offset;
    for (Triangle& t : _triangles_) {
        t.object_id = _id_ + geometry_id_offset;
        t.material_id = _material_id_ + _material_id_offset_;
    }

}

int gpurt::Geometry::bvhSize()const {
    return _bvh_.size();
}
