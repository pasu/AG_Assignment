// Don't believe the file name
// This is actually used for NEE

#pragma once
#include"precomp.h"
#include"GPURT_GeometryGroup.h"
namespace gpurt {
    class VirtualPointLightinitGemnerator {
    private:
        array<const gpurt::Geometry*,1024> _lightsources_;
        
    public:
        
        void init(const gpurt::GeometryGroup& group);

        void shuffel();

        array<int,1024> light_triangle_id;
    };

}