#pragma once
#include "RTPrimitive.h"

class RTSphere : public RTPrimitive
{
  public:
	RTSphere( const vec3 &position, const float radius, RTMaterial &material );

	const RTIntersection intersect( const RTRay &ray ) const;
	const SurfacePointData getSurfacePointData( const RTIntersection &intersection ) const;
  private:
	float radius;
};
