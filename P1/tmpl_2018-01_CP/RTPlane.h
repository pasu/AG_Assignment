#pragma once
#include "RTPrimitive.h"
class RTPlane : public RTPrimitive
{
  private:
	vec3 normal;
	vec3 tangent;
	vec3 binormal;

	

  public:
	vec2 boundaryxy;
	RTPlane( const vec3 &position, const vec3 &normal, const vec3 &tangent, const RTMaterial &material );

	const RTIntersection intersect( const RTRay &ray ) const;
	const SurfacePointData getSurfacePointData( const RTIntersection &intersection ) const;
	
};
