// Dynamic geometry
// container of a static object and a rigid body transform
#pragma once
#ifndef __RT_RIGID_BODY__
#define __RT_RIGID_BODY__
#include "RTGeometry.h"
#include "RTObject.h"
class RTObject
{
  public:
	RTObject( RTGeometry *g );

	

	void updateAABBbounds(); // update AABB bounds after rotation/translation
	
    bool getIntersection( const RTRay &ray, RTIntersection &nearestIntersection ) const;

    const RTGeometry *getGeometry() const { return pGeometry; }
    const AABB &getAABBBounds() const { return bounds; }


    // rigid body transform
	void resetTransform();
	void translate(const vec3 v);

  private:
	RTGeometry *pGeometry;

	mat4 mViewRotate; // rotation part of view matrix
	vec3 pos;		  // the coordinate of origin
	AABB bounds;
};

#endif