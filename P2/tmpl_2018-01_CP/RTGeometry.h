// static geometries
// container of subbvh and primitives

#pragma once
#ifndef __RT_OBJECT__
#define __RT_OBJECT__

#include "precomp.h"
#include"RTPrimitive.h"
#include"BVH.h"
class RTGeometry
{
  public:
	void BuildBVHTree();
	bool getIntersection( const RTRay &ray, RTIntersection &nearestIntersection ) const;
	void addObject( RTPrimitive *object );
	const AABB &getAABBBounds() const{
		return ( *bvhTree ).bvhTree[0].bounds;
    }
  private:
	vector<RTPrimitive *> primitivecollection;

	BVH *bvhTree;

};

#endif