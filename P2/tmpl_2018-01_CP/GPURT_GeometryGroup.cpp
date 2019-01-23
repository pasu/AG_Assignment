#include "precomp.h"
#include "GPURT_GeometryGroup.h"

using namespace gpurt;

void gpurt::loadGeometryGroup() {
    Assimp::Importer* importer = new Assimp::Importer();

    importer->ReadFile("assets\\GPURT\\house_interior\\untitled.obj", aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace | aiProcess_Triangulate |
        aiProcess_JoinIdenticalVertices| aiProcess_FlipUVs);
    if (!importer->GetScene())
    {
        cout << "Error loading mesh: " << "assets\\GPURT\\house_interior\\untitled.obj" << ". " << importer->GetErrorString();
        return;
    }
    const aiScene &scene = *(importer->GetScene());

    cout << scene.mFlags << std::endl;
    AI_SCENE_FLAGS_INCOMPLETE;
    cout << scene.mNumMaterials << endl;

    cout << scene.mNumMeshes << endl;

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

}
