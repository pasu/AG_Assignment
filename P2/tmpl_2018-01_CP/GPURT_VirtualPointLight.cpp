// Don't believe the file name
// This is actually used for NEE

#include"precomp.h"
#include "GPURT_VirtualPointLight.h"


void gpurt::VirtualPointLightinitGemnerator::init(const gpurt::GeometryGroup & group) {
    using LightWeight = struct {
        const Geometry* g;
        float weight;
    };
   

    vector<LightWeight> lws;
    lws.clear();
    lws.reserve(1024);

    for (const Geometry* g : group.geometries()) {
        if (group.getMaterial(g->getMtlID()).isLightSource()) {
            lws.push_back({
                g,
                g->getBVH().aabb().area()*group.getMaterial(g->getMtlID()).getIll()
                });
        }
    }
    // normalize
    float weight_total = 0;
    for (auto &i : lws) {
        weight_total += i.weight;
    }
    for (auto &i : lws) {
        i.weight = i.weight / weight_total;
    }

    int start = 0;
    for (auto & i : lws) {
        int n = int(i.weight * 1024+0.5);
        int b = 0;
        while (start < 1024 && b < n) {
            _lightsources_[start] = i.g;
            start++;
            b++;
        }
    }

    while (start < 1024) {
        _lightsources_[start] = lws[start].g;
        start++;
    }
}


void gpurt::VirtualPointLightinitGemnerator::shuffel() {
    for (int i = 0; i < 1024; i++) {
        auto g = _lightsources_[i];

        int n = g->triangleCount();
        
        float x = rand() / float(RAND_MAX);
        
        int ii = int(x*n + 0.5);

        ii = (ii >= n) ? (n-1) : (ii);

        light_triangle_id[i] = ii + g->getTriangleOffset();
    }
}
