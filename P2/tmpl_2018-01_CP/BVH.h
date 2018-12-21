#pragma once
#include "AABB.h"
#include "RTPrimitive.h"
#include "RTRay.h"
#include "RTIntersection.h"

struct BVHNode
{
	AABB bounds;
	uint32_t leftFirst, count;
};

class BVH
{
  public:
	BVH( std::vector<RTPrimitive *> *objects, uint32_t leafSize = 4 );
	~BVH();

	std::vector<RTPrimitive *> *build_prims;

	//! Build the BVH tree out of build_prims
	void build();

	// Fast Traversal System
	BVHNode *bvhTree;

	bool getIntersection( const RTRay &ray, RTIntersection *intersection, bool occlusion ) const;

	~BVH();

  private:
	uint32_t nNodes, nLeafs, leafSize;
};
