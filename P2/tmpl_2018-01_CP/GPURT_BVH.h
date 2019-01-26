#pragma once
#include"precomp.h"
#include"GPURT_AABB.h"
namespace gpurt {
    
    // size = 32
    class BVHNode {
    public:
        BVHNode();

        union {
            AABB aabb;
            struct {
                float _padding1_[3];
                unsigned int left_first;
                float _padding2_[3];
                unsigned int count;
            };
        };
    };

}