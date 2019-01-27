#pragma once
#include"GPURT_Triangle.h"

namespace gpurt {
    class AABB {
    public:
        AABB();
        AABB(const AABB& b);
        const AABB& operator=(const gpurt::AABB& b);

        void operator+=(const gpurt::AABB& b);

        float area()const;

        float splitPlane(int dim, float frac);

        static AABB wrap(const gpurt::Vertex& v);

        static AABB wrap(const gpurt::Triangle& t);// calculate the bounding box of a triangle

        
    private:
        vec3 _min_;
        vec3 _max_;
        friend const AABB operator+(const AABB& a, const AABB& b);
    };

    const AABB operator+(const AABB& a, const AABB& b);

}