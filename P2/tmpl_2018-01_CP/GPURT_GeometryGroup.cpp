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

    // GetTexture and material
    /*
    cout << scene.mNumTextures << endl;
    cout << scene.mMaterials[0]->GetTextureCount(aiTextureType_DIFFUSE)<< endl;
    cout << scene.mMaterials[1]->GetTextureCount(aiTextureType_DIFFUSE) << endl;
    
    aiString texPath;

    aiReturn tex = scene.mMaterials[1]->GetTexture(aiTextureType_DIFFUSE, 0, &texPath);

    aiMaterial*  mat = scene.mMaterials[1];

    aiColor3D sha;


    mat->Get(AI_MATKEY_COLOR_DIFFUSE,sha);

    cout << sha.r<<" "<<sha.g<<" "<<sha.b << endl;
    mat->Get(AI_MATKEY_COLOR_SPECULAR, sha);

    cout << sha.r << " " << sha.g << " " << sha.b << endl;

    mat->Get(AI_MATKEY_COLOR_AMBIENT, sha);

    cout << sha.r << " " << sha.g << " " << sha.b << endl;
    */

    for (unsigned int meshI = 0; meshI < scene.mNumMeshes; ++meshI)
    {
        aiMesh &mesh = *(scene.mMeshes[meshI]);
        
        _geometries_.push_back(new Geometry(mesh));
    }

}

int gpurt::GeometryGroup::triangleCount(){
    int number = 0;
    for (Geometry* g : _geometries_) {
        number += g->triangleCount();
    }
    return number;
}
