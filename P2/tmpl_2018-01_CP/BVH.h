#pragma once
#include "AABB.h"
#include "RTPrimitive.h"
#include "RTRay.h"
#include "RTIntersection.h"

struct BVHNode_32
{
	AABB bounds;
	//merge start and rightOffset as leftFirst;
	uint32_t leftFirst, count;
};
struct BVHNode
{
	AABB bounds;
	uint32_t start, nPrims, rightOffset;
	//uint32_t leftFirst, count;
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
	BVHNode_32 *bvhTree;

	bool getIntersection( const RTRay &ray, RTIntersection *intersection, bool occlusion ) const;

  private:
	uint32_t nNodes, nLeafs, leafSize;
};
