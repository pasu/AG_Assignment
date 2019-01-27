#pragma once
#include"precomp.h"
#include"GPURT_AABB.h"
namespace gpurt {
    
    // size = 32
    class BVHNode {
    public:
        BVHNode();
        BVHNode(const BVHNode& b);
        BVHNode(gpurt::AABB aabb);
        
        const BVHNode& operator=(const BVHNode& b);



        union {
            gpurt::AABB aabb;
            struct {
                float _padding1_[3];
                unsigned int left_first;
                float _padding2_[3];
                unsigned int count;
            };
        };

        bool isLeaf()const { return count > 0; }
    };

    class BVH {

    private:

        int _depth_;

    public:

        vector<gpurt::BVHNode> _bvh_;

        const gpurt::AABB& aabb()const { return _bvh_[0].aabb; }

        void construct(vector<gpurt::Triangle>& t, const int leaf_size=4);

        void fastClaster(const vector<const BVH const*>& subs);

        const int getDepth()const { return _depth_; }

        int size()const { return _bvh_.size(); }
        int c_size()const { return size() * sizeof(BVHNode); }
        
        BVHNode& operator[](int i) { return _bvh_[i]; }

        void copy(gpurt::BVHNode* dst, int& inner_offset)const;

    };

}