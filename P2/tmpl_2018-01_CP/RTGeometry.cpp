#include "precomp.h"
#include "RTGeometry.h"

void RTGeometry::BuildBVHTree()
{
	bvhTree = new BVH( &primitivecollection );
}

bool RTGeometry::getIntersection( const RTRay &ray, RTIntersection &nearestIntersection ) const
{
	return bvhTree->getIntersection( ray, &nearestIntersection, false );
}


void RTGeometry::addObject( RTPrimitive *object )
{
	primitivecollection.push_back( object );
}