#pragma once
#include "RTPrimitive.h"
class RTBox : public RTPrimitive
{
  public:
	RTBox( const vec3 &center, const vec3 &dimension, const RTMaterial &material );
	RTBox( const vec3 &center, const vec3 &min, const vec3 max, const RTMaterial &material );
	~RTBox();

	const RTIntersection intersect( const RTRay &ray ) const;
	const SurfacePointData getSurfacePointData( const RTIntersection &intersection ) const;

  private:
	vec3 min;
	vec3 max;

	float distanceX;
	float distanceY;
	float distanceZ;
};
