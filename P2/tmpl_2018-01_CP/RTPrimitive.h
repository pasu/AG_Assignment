#pragma once
#include "RTMaterial.h"
#include "RTRay.h"
#include "RTSurfacePointData.h"

#include "AABB.h"

#define FLOAT_ZERO 0.000001f
#define MAX_DISTANCE_TO_INTERSECTON FLT_MAX

class RTIntersection;

class RTPrimitive
{
  public:
	RTPrimitive( const vec3 center, const RTMaterial& material);
	RTPrimitive( const RTMaterial &material );
	virtual const RTIntersection intersect( const RTRay &ray ) const = 0;
	virtual const SurfacePointData getSurfacePointData( const RTIntersection &intersection ) const = 0;

	inline const RTMaterial &getMaterial() const { return material; }

	AABB getAABB();
	vec3 getCentroid();

  protected:
	vec3 pos;
	AABB box;
	const RTMaterial& material;
};
