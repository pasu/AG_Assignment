#include "precomp.h"
#include "GPURT_GeometryGroup.h"

using namespace gpurt;



gpurt::GeometryGroup::GeometryGroup(const char* file_name) {
    Assimp::Importer* importer = new Assimp::Importer();

    importer->ReadFile(file_name, aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace | aiProcess_Triangulate |
        aiProcess_JoinIdenticalVertices| aiProcess_FlipUVs);
    if (!importer->GetScene())
    {
        cout << "Error loading mesh: " << file_name << ". " << importer->GetErrorString();
        return;
    }
    const aiScene &scene = *(importer->GetScene());

    if (scene.mFlags == AI_SCENE_FLAGS_INCOMPLETE) {
        cout << "Scene incomplete : " << file_name <<endl;
    }

    // load material

    _mtls_.clear();
    _mtls_.reserve(scene.mNumMaterials);
    for (int i = 0; i < scene.mNumMaterials; i++) {
        aiMaterial*  mat = scene.mMaterials[i];
        _mtls_.push_back(Mtl(*mat));
    }
    // texture
    /*
    cout << scene.mNumTextures << endl;
    cout << scene.mMaterials[0]->GetTextureCount(aiTextureType_DIFFUSE)<< endl;
    cout << scene.mMaterials[1]->GetTextureCount(aiTextureType_DIFFUSE) << endl;
    
    aiString texPath;

    aiReturn tex = scene.mMaterials[1]->GetTexture(aiTextureType_DIFFUSE, 0, &texPath);
    */
    
    for (unsigned int meshI = 0; meshI < scene.mNumMeshes; ++meshI)
    {
        
        aiMesh &mesh = *(scene.mMeshes[meshI]);
        
        _geometries_.push_back(new Geometry(mesh, meshI));
        
    }

    constructBVH();
}

const int gpurt::GeometryGroup::triangleCount()const{
    int number = 0;
    for (Geometry* g : _geometries_) {
        number += g->triangleCount();
    }
    return number;
}

void gpurt::GeometryGroup::constructBVH() {
    
    _sub_bvh_list_.clear();
    _sub_bvh_list_.reserve(geometryCount());

    for (Geometry* g:_geometries_) {
        _sub_bvh_list_.push_back(&g->getBVH());
    }

    _bvh_.fastClaster(_sub_bvh_list_);
    std::cout << "BVH Depth:" << _bvh_.getDepth() << endl;
}

void gpurt::GeometryGroup::copyBVH(BVHNode * dst, int & offset) {

    int start_leaf = offset;

    _bvh_.copy(dst, offset);

    for (int i = 0; i < _bvh_.size(); i++) {
        gpurt::BVHNode& node = dst[start_leaf + i];
        if (node.isLeaf()) {// found a leaf node

            const gpurt::Geometry& left_child= *_geometries_[node.left_first];

            if (left_child.bvhSize() > 1) {
                node.count = 0;// it is nolonger a leaf
                node.left_first = offset + 1;// set left child, don't forget to jump over root

                left_child.copyBVH(dst, offset);

            }
            else {
                node.count = left_child.triangleCount();
            }
        }
        
    }
}

void gpurt::GeometryGroup::setOffset(int & offset, int& geometry_id_offset, int& material_id_offset) {
    for (Geometry* g : _geometries_) {
        g->setOffset(offset, geometry_id_offset, material_id_offset);
    }
    material_id_offset += mtlCount();
    geometry_id_offset += geometryCount();
}

int gpurt::GeometryGroup::bvhSize() {
    int s = _bvh_.size();
    for (gpurt::Geometry* g : _geometries_) {
        s += g->bvhSize();
    }
    return s;
}
