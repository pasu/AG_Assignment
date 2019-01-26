#include "precomp.h"
#include "GPURT_Geometry.h"
#include<assimp\mesh.h>

gpurt::Geometry::Geometry(const aiMesh & mesh) {

    triangles.reserve(mesh.mNumFaces);

    _bvh_.reserve(mesh.mNumFaces);

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
        triangles.push_back(t);
    }
}

void gpurt::Geometry::constructBVH() {

}