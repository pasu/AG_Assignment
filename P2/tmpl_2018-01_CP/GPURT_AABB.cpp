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

const gpurt::AABB & gpurt::AABB::operator=(const gpurt::AABB & b) {
    _min_.x = b._min_.x;
    _min_.y = b._min_.y;
    _min_.z = b._min_.z;

    _max_.x = b._max_.x;
    _max_.y = b._max_.y;
    _max_.z = b._max_.z;
    return *this;
}

gpurt::AABB::AABB(const gpurt::AABB & b) {
    _min_.x = b._min_.x;
    _min_.y = b._min_.y;
    _min_.z = b._min_.z;

    _max_.x = b._max_.x;
    _max_.y = b._max_.y;
    _max_.z = b._max_.z;
}

void gpurt::AABB::operator+=(const AABB & b) {
    _min_.x = min(_min_.x, b._min_.x);
    _min_.y = min(_min_.y, b._min_.y);
    _min_.z = min(_min_.z, b._min_.z);

    _max_.x = max(_max_.x, b._max_.x);
    _max_.y = max(_max_.y, b._max_.y);
    _max_.z = max(_max_.z, b._max_.z);
}

float gpurt::AABB::area() const {
    if (_min_.x > _max_.x) {
        return 0;
    }
    vec3 l = _max_ - _min_;
    return l.x*l.y + l.x*l.z + l.y*l.z;
}

float gpurt::AABB::splitPlane(int dim, float frac) {
    return (_max_.cell[dim] - _min_.cell[dim])*frac + _min_.cell[dim];
}

gpurt::AABB gpurt::AABB::wrap(const Vertex& v) {
    AABB result;
    result._min_.x = v.pos.x;
    result._max_.x = v.pos.x;
    result._min_.y = v.pos.y;
    result._max_.y = v.pos.y;
    result._min_.z = v.pos.z;
    result._max_.z = v.pos.z;
    return result;
}

gpurt::AABB gpurt::AABB::wrap(const Triangle & t) {
    return wrap(t.v1) + wrap(t.v2) + wrap(t.v3);
}

const gpurt::AABB gpurt::operator+(const gpurt::AABB & a, const gpurt::AABB & b) {
    gpurt::AABB c;
    c._min_.x = min(a._min_.x, b._min_.x);
    c._min_.y = min(a._min_.y, b._min_.y);
    c._min_.z = min(a._min_.z, b._min_.z);

    c._max_.x = max(a._max_.x, b._max_.x);
    c._max_.y = max(a._max_.y, b._max_.y);
    c._max_.z = max(a._max_.z, b._max_.z);
    return c;
}
