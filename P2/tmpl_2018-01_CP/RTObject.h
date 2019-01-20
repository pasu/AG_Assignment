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

	bool getIntersection( const RTRay &ray, RTIntersection &nearestIntersection, bool occlusion = false, const float& distance = FLT_MAX ) const;

	const RTGeometry *getGeometry() const { return pGeometry; }
	const AABB &getAABBBounds() const { return bounds; }

	typedef void ( *AnimateFunc )( RTObject * );
	void animate();													// invoke animate callback
	void setAnimateFunc( AnimateFunc func ) { animateFunc = func; } // set animate callback

	// rigid body transform
	void resetTransform();
	void translateGlobal( const vec3 v );
	void rotateLocal( vec3 axis, float angle );

	mat4 mModelRotate; // rotation part of view matrix
	mat4 mViewRotate;
	vec3 pos; // the coordinate of origin
	vec3 speed;
	vec3 rotateAxis;
	float rotateSpeed;
  private:
	RTGeometry *pGeometry;

	AABB bounds;

	AnimateFunc animateFunc; // animate callback
};

#endif