#include "precomp.h"
#include "GPURT_Triangle.h"

const vec3 gpurt::Triangle::centroid() const
{
    aiVector3D c = v1.pos + v2.pos + v3.pos;
    return {
        c.x/3,
        c.y/3,
        c.z/3
    };
}
