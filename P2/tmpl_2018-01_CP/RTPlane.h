#pragma once
#include "RTPrimitive.h"
#include "Utils.h"
class RTPlane : public RTPrimitive
{
  public:
	vec3 normal;
	vec3 tangent;
	vec3 binormal;

  public:
	vec2 boundaryxy;
	bool bCircle;
	RTPlane( const vec3 &position, const vec3 &normal, const vec3 &tangent, const RTMaterial &material, vec2 boundary = vec2( Utils::MAX_FLOAT ) );

	const RTIntersection intersect( const RTRay &ray ) const;
	const SurfacePointData getSurfacePointData( const RTIntersection &intersection ) const;
	
	void computeAABBbounds();

	vec3 getRandomPnt();
};
