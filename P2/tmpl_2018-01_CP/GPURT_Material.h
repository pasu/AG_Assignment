#pragma once
#include"precomp.h"
namespace gpurt {
    // material
    class Mtl {
    public:
        static constexpr int LIGHTSOURCE = 1;
        static constexpr int DIFFUSE = 2;
        static constexpr int SPECULAR = 3;
        static constexpr int GLOSSY = 4;
        static constexpr int TRANS = 5;
    private:

        vec3 color_d_e;// diffuse or emmisive
        float _p1;
        vec3 color_s_g_t;// specular or glossy reflection or refraction color
        float _p2;
        vec3 color_tf; // transmission filter color
        float _p3;
    };
}