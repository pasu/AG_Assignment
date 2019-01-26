#pragma once
#include"precomp.h"

#include"GPURT_Triangle.h"

namespace gpurt {
    class AABB {
    public:
        AABB();
        const AABB& operator=(const AABB& b);

        static AABB wrap(const Vertex& v);

        static AABB wrap(const Triangle& t);// calculate the bounding box of a triangle

        
    private:
        vec3 _min_;
        vec3 _max_;
        friend AABB operator+(const gpurt::AABB& a, const gpurt::AABB& b);
    };
    AABB operator+(const gpurt::AABB& a, const gpurt::AABB& b);
}