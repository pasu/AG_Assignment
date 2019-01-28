#pragma once
#include"precomp.h"


#include <assimp/Importer.hpp>
#include <assimp/mesh.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include<array>

namespace gpurt {
    // material
#pragma pack(16)
    class Mtl {
    public:
        static constexpr int EMMISION = 1;
        static constexpr int DIFFUSE = 2;
        static constexpr int SPECULAR = 3;
        static constexpr int GLOSSY = 4;
        static constexpr int REFRACT = 5;

        Mtl(aiMaterial& aimtl);

    private:

        aiColor3D color_diffuse;// Kd or Ke
        float _ns;// glossy factor
        aiColor3D color_specular;// Ks or refract
        float _ni;// index of refraction

        float _alpha;// "d" in mtl file, ratio of refraction, alpha = 0: full transparent, alpha=1: not transparent

        // a_diffuse+a_reflact = _alpha = 1-refract
        // posiibility of refraction = 1-alpha
        // possibility of emmision = 1 if Ke > vec3(1) else 0
        // possibility of diffuse = Kd/(Kd+Ks)*_alpha
        // possibility of reflection = Ks/(Kd+Ks)*alpha
        // Kd and Ks should be the same color with different strength
        float _prefract;// possibility of refraction
        float _pdiffuse;// possibility of diffuse
        float _pspecular;// possibility of reflaction
        
        std::array<int,256> _shading_types_;// will be filled according to possibilities

    };
}