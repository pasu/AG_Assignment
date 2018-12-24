#pragma once
#include "RTPrimitive.h"
class RTCone : public RTPrimitive
{
  public:
	RTCone( const vec3 &top, const vec3 &bottomCenter, const float &radius, const RTMaterial& material );
	~RTCone();

	const RTIntersection intersect( const RTRay &ray ) const ;
	const SurfacePointData getSurfacePointData( const RTIntersection &intersection ) const;

	void computeAABBbounds();
  private:
	vec3 top;
	float radius;

	vec3 dir;
	vec3 tangent;
	vec3 binormal;
};
