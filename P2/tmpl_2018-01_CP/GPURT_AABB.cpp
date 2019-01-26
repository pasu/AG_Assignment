#include"precomp.h"
#include"GPURT_AABB.h"


using namespace gpurt;

gpurt::AABB::AABB() {
    _min_.x = 999999.0f;
    _min_.y = 999999.0f;
    _min_.z = 999999.0f;

    _max_.x = -999999.0f;
    _max_.y = -999999.0f;
    _max_.z = -999999.0f;
}

const gpurt::AABB & gpurt::AABB::operator=(const AABB & b) {
    _min_.x = b._min_.x;
    _min_.y = b._min_.y;
    _min_.z = b._min_.z;
    _max_.x = b._max_.x;
    _max_.y = b._max_.y;
    _max_.z = b._max_.z;
}

gpurt::AABB gpurt::AABB::wrap(const Vertex& v) {
    AABB result;
    result._min_.x = result._max_.x = v.pos.x;
    result._min_.y = result._max_.y = v.pos.y;
    result._min_.z = result._max_.z = v.pos.z;
    return result;
}

gpurt::AABB gpurt::AABB::wrap(const Triangle & t) {
    return wrap(t.v1) + wrap(t.v2) + wrap(t.v3);
}

gpurt::AABB gpurt::operator+(const gpurt::AABB& a, const gpurt::AABB& b) {
    AABB result;
    result._min_.x = min(a._min_.x, b._min_.x);
    result._min_.y = min(a._min_.y, b._min_.y);
    result._min_.z = min(a._min_.z, b._min_.z);

    result._max_.x = min(a._max_.x, b._max_.x);
    result._max_.y = min(a._max_.y, b._max_.y);
    result._max_.z = min(a._max_.z, b._max_.z);

    return result;
}
