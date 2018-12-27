#pragma once
#ifndef __TOP_LEVEL_BVH__
#define __TOP_LEVEL_BVH__

#include "RTObject.h"
#include "precomp.h"
#include "AABB.h"

class TopLevelBVHNode {
  public:
	AABB bounds;
	RTObject *object;// object!=nullptr if it's a leaf node. If object==nullptr, it's an inner node.
	int son;// index to the first son. second son = first son+1
};

// Top-Level-BVH is not promised to be balanced, can be very unbalanced
// But the number of nodes is promised to be 2n-1
// let nodes[0] = root
// then remove / add options can be done by swapping and decreasing the pointer.
class TopLevelBVH
{
  public:
	TopLevelBVH( const std::vector<RTObject *> &objects );
	~TopLevelBVH();

	void rebuild();
	bool getIntersection( const RTRay &ray, RTIntersection *intersection) const;
  private:
	TopLevelBVHNode *nodes;
	const std::vector<RTObject *> &objects;
	int tree_size;// the size of bvh tree: 2n-1
};

#endif