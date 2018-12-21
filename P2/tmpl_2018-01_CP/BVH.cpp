#include "precomp.h"
#include "BVH.h"

BVH::BVH( std::vector<RTPrimitive *> *objects, uint32_t leafSize /*= 4 */ )
: build_prims( objects ), leafSize( leafSize ), nNodes( 0 ), nLeafs( 0 ), bvhTree( NULL )
{
	build();
}

BVH::~BVH()
{
}

bool BVH::getIntersection( const RTRay &ray, RTIntersection *intersection, bool occlusion ) const
{
	return false;
}

struct BVHBuildEntry
{
	// If non-zero then this is the index of the parent. (used in offsets)
	uint32_t parent;
	// The range of objects in the object list covered by this node.
	uint32_t start, end;
};

void BVH::build()
{
	BVHBuildEntry todo[128];
	uint32_t stackptr = 0;
	const uint32_t Untouched = 0xffffffff;
	const uint32_t TouchedTwice = 0xfffffffd;

	// Push the root
	todo[stackptr].start = 0;
	todo[stackptr].end = build_prims->size();
	todo[stackptr].parent = 0xfffffffc;
	stackptr++;

	BVHNode node;
	std::vector<BVHNode> buildnodes;
	buildnodes.reserve( build_prims->size() * 2 );
}
