#pragma once
#include "AABB.h"
#include "RTPrimitive.h"
#include "RTRay.h"
#include "RTIntersection.h"
#include "RayPacket.h"

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

	int getFirstHit( const RayPacket &raypacket, const AABB &box, int ia )const;

#ifdef BVH_RANGED_TRAVERSAL
	int getLastHit( const RayPacket &raypacket, const AABB &box, int ia )const;
	bool getIntersection( const RayPacket &raypacket, RTIntersection *intersections );
#endif

#ifdef BVH_PARTITION_TRAVERSAL
	int getLastHit( const RayPacket &raypacket, const AABB &box, int ia, const unsigned int *aRayIndex) const;
	int partRays( const RayPacket &raypacket, const AABB &box, int ia, unsigned int *aRayIndex ) const;
	bool getIntersection( const RayPacket &raypacket, RTIntersection *intersections );
#endif
	void getSplitDimAndCoordBySAH( uint32_t &split_dim, float &split_coord, uint32_t binnedNum, AABB &bc, uint32_t &start, uint32_t &end );

  private:
	uint32_t nNodes, nLeafs, leafSize;
	unsigned int m_I[RAYPACKET_RAYS_PER_PACKET];
};
