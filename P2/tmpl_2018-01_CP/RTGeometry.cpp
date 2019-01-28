#include "precomp.h"
#include "RTGeometry.h"

void RTGeometry::BuildBVHTree()
{
	bvhTree = new BVH( &primitivecollection );
}

void RTGeometry::BuildSBVHTree()
{
	sbvhTree = new SBVH( &primitivecollection );
}

bool RTGeometry::getIntersection( const RTRay &ray, RTIntersection &nearestIntersection, bool occlusion, const float &distance ) const
{
	if (sbvhTree)
	{
		nearestIntersection = sbvhTree->DoesIntersect( ray );
		return nearestIntersection.isIntersecting();
	}

	return bvhTree->getIntersection( ray, &nearestIntersection, occlusion, distance );
}


void RTGeometry::addObject( RTPrimitive *object )
{
	primitivecollection.push_back( object );
}