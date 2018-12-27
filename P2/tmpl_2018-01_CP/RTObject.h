// Dynamic geometry
// container of a static object and a rigid body transform
#pragma once
#ifndef __RT_RIGID_BODY__
#define __RT_RIGID_BODY__
#include "RTGeometry.h"
#include "RTObject.h"

class RTObject
{
  private:
	static void animateFuncDefault( RTObject * );

  public:
	RTObject( RTGeometry *g );

	void updateAABBbounds(); // update AABB bounds after rotation/translation

	bool getIntersection( const RTRay &ray, RTIntersection &nearestIntersection ) const;

	const RTGeometry *getGeometry() const { return pGeometry; }
	const AABB &getAABBBounds() const { return bounds; }

	// rigid body transform
	void resetTransform();
	void translateGlobal( const vec3 v );
	void rotateLocal( vec3 axis, float angle );

	typedef void ( *AnimateFunc )( RTObject* );
    void animate();
    void setAnimateFunc(AnimateFunc func) {
		animateFunc = func;
    }

	mat4 mModelRotate; // rotation part of view matrix
	vec3 pos;		   // the coordinate of origin
	vec3 speed;
  private:
	RTGeometry *pGeometry;

	
	AABB bounds;

	AnimateFunc animateFunc;
};

#endif