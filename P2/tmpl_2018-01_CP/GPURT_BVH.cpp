#include"precomp.h"
#include"GPURT_BVH.h"


gpurt::BVHNode::BVHNode() {
    count = 0;
}

gpurt::BVHNode::BVHNode(const gpurt::BVHNode & b) {
    aabb = b.aabb;
    count = b.count;
    left_first = b.left_first;
}

gpurt::BVHNode::BVHNode(AABB aabb) {
    this->aabb = aabb;
    count = 0;
}

const gpurt::BVHNode & gpurt::BVHNode::operator=(const BVHNode & b) {
    aabb = b.aabb;
    left_first = b.left_first;
    count = b.count;
    return *this;
}


using TriangleWrapper = struct {
    gpurt::AABB aabb;
    vec3 c;// centroid
    gpurt::Triangle* triangle;
};
static vector<TriangleWrapper> tris;

gpurt::AABB boundingBox(int start, int end) {
    gpurt::AABB aabb;
    for (int i = start; i < end; i++) {
        aabb += tris[i].aabb;
    }
    return aabb;
};

// sort tris and return the mid index(_tris[mid] should be in right/up/front part)
int binnedSAHMid(int start, int end);


void gpurt::BVH::construct(vector<gpurt::Triangle>& t, const int leaf_size) {
    //init tris
    tris.clear();
    tris.reserve(t.size());
    for (int i = 0; i < t.size(); i++) {
        AABB aabb = AABB::wrap(t[i]);
        tris.push_back({
            aabb,
            t[i].centroid(),
            &t[i]
            }
        );
    }

    _bvh_.reserve(t.size());

    _bvh_.push_back(gpurt::BVHNode());// root
    // reuse the two integer entries as start and end
    _bvh_[0].left_first = 0;
    _bvh_[0].count = t.size();

    vector<BVHNode*>todo;
    todo.push_back(&_bvh_[0]);

    _depth_ = 0;

    while (!todo.empty()) {

        if (todo.size() > _depth_) {
            _depth_ = todo.size();
        }
        gpurt::BVHNode& node = **(todo.end() - 1);
        todo.pop_back();

        int start = node.left_first;
        int end = node.count;

        node.aabb = gpurt::AABB();
        for (int i = start; i < end; i++) {
            node.aabb += tris[i].aabb;
        }

        // leaf node
        if (end - start <= leaf_size || node.aabb.area()<0.001) {
            node.count = end - start;
            node.left_first = start;
        }
        // is not leaf node
        else {
            int mid = binnedSAHMid(start, end);

            node.count = 0;
            node.left_first = _bvh_.size();

            gpurt::BVHNode leftChild;
            leftChild.left_first = start;
            leftChild.count = mid;

            gpurt::BVHNode rightChild;
            rightChild.left_first = mid;
            rightChild.count = end;

            _bvh_.push_back(leftChild);
            todo.push_back(&*(_bvh_.end() - 1));

            _bvh_.push_back(rightChild);
            todo.push_back(&*(_bvh_.end() - 1));
        }
    }
    tris.clear();
}
int binnedSAHMid(int start, int end) {
    // cost function of a specific reference value and dim (x,y,z)
    auto cost_func = [&](float ref, int dim) {
        gpurt::AABB left;
        int leftCount = 0;
        gpurt::AABB right;
        int rightCount = 0;
        for (int i = start; i < end; i++) {
            if (tris[i].c.cell[dim] < ref) {// triangls centroid lies left/down/behind of the reference plane (splitting plane)
                left += tris[i].aabb;
                leftCount++;
            }
            else {
                right += tris[i].aabb;
                rightCount++;
            }
        }
        return left.area()*leftCount + right.area()*rightCount;
    };

    float candidate_cost = numeric_limits<float>::max();
    float candidate_ref;
    int candidate_dim;

    if (end - start <= BIN_NUM) {
        for (int dim = 0; dim <= 2; dim++) for (int mid = start; mid < end; mid++) {
            float cost = cost_func(tris[mid].c.cell[dim], dim);
            if (cost < candidate_cost) {
                candidate_cost = cost;
                candidate_dim = dim;
                candidate_ref = tris[mid].c.cell[dim];
            }
        }

    }
    else {
        gpurt::AABB aabb = boundingBox(start, end);
        for (int dim = 0; dim <= 2; dim++) for (int sep = 1; sep < BIN_NUM; sep++) {
            float ref = aabb.splitPlane(dim, sep / (float)BIN_NUM);
            float cost = cost_func(ref, dim);
            if (cost < candidate_cost) {
                candidate_cost = cost;
                candidate_dim = dim;
                candidate_ref = ref;
            }
        }
    }

    int head = start;
    int tail = end - 1;
    while (head < tail) {
        if (tris[head].c.cell[candidate_dim] >= candidate_ref) {
            while (tris[tail].c.cell[candidate_dim] >= candidate_ref && head < tail) {
                tail--;
            }
            std::swap(tris[tail], tris[head]);
            std::swap(*(tris[tail].triangle), *(tris[head].triangle));
            std::swap((tris[tail].triangle), (tris[head].triangle));
        }
        head++;
    }
    return tail;
};

void gpurt::BVH::fastClaster(const vector<const BVH const*>& objects) {

    int tree_size = objects.size() * 2 - 1;

    _bvh_.clear();
    _bvh_.resize(tree_size);

    vector<BVHNode>& nodes = _bvh_;

    vector<int>depth(tree_size);

    for (int i = 0; i < objects.size() - 1;i++) {
        nodes[i].count = 0;// inner nodes
    }
    for (int i = 0; i < objects.size(); i++) {
        nodes[tree_size - i - 1].aabb = objects[i]->aabb();
        nodes[tree_size - i - 1].left_first = i;
        nodes[tree_size - i - 1].count = 1;// leaf nodes
        depth[tree_size - i - 1] = objects[i]->_depth_;
    }
    // nodes[2n-1]:
    // nodes={root,inner_node,...,inner_node,leaf_node,...,leaf_node}
    // root and (n-2) innernodes and n leafnodes

    int leaf_ptr = tree_size;					// index of the last leaf node that was added in th tree
    int inner_ptr = tree_size - objects.size(); // index of the last inner node that was added to the tree

    // Any inner node whose index is larger than inner_ptr has been added to the tree
    // Any leaf node whose index is less than leaf_ptr has not been added to the tree
    // So, any node whose index is in [inner_ptr, leaf_ptr) need to be checked.

    // function to find the best match of a node
    auto findBestMatch = [&](int a) {
        // 		assert( inner_ptr > 0 );
        // 		assert( inner_ptr <= a );
        // 		assert( a < leaf_ptr );

        int least_index = 0;
        float least_surface = numeric_limits<float>::max();
        // check every one in [inner_ptr, leaf_ptr)
        for (int b = inner_ptr; b < leaf_ptr; b++)
        {
            if (b == a) continue;
            // aabb = i.aabb+b.aabb
            AABB aabb = nodes[a].aabb;
            aabb += nodes[b].aabb;
            float sa = aabb.area();

            if (sa < least_surface)
            {
                least_surface = sa;
                least_index = b;
            }
        }
        return least_index;
    };

    int A = tree_size - 1; // init finding best pair with the last one
    int B = findBestMatch(A);
    while (inner_ptr > 0)
    {
        int C = findBestMatch(B);
        if (A == C)
        { // best pair is found
            // swap them to the "finished" segment
            leaf_ptr--;
            std::swap(nodes[A], nodes[leaf_ptr]);
            std::swap(depth[A], depth[leaf_ptr]);
            leaf_ptr--;
            std::swap(nodes[B], nodes[leaf_ptr]);
            std::swap(depth[B], depth[leaf_ptr]);

            // construct their father node
            inner_ptr--;
            nodes[inner_ptr].aabb = nodes[leaf_ptr].aabb + nodes[leaf_ptr + 1].aabb;
            
            nodes[inner_ptr].left_first = leaf_ptr;

            depth[inner_ptr] = max(depth[leaf_ptr], depth[leaf_ptr + 1]) + 1;

            // exit strategy
            if (inner_ptr == 0) break;

            // prepare for next pair
            A = inner_ptr;
            B = findBestMatch(A);
        }
        else
        { // No best pair found
            A = B;
            B = C;
        }
    }
    _depth_ = depth[0];
}

void gpurt::BVH::copy(gpurt::BVHNode * dst, int & offset) const{
    for (int i = 0; i < size(); i++) {
        dst[i + offset] = _bvh_[i];
        if (!dst[i + offset].isLeaf()) {
            dst[i + offset].left_first += offset;
        }
        
    }
    offset += size();
}