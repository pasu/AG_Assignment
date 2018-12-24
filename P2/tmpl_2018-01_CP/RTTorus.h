#pragma once
#include "RTPrimitive.h"
class RTTorus : public RTPrimitive
{
  public:
	RTTorus( const vec3 &center, const vec3 &axis, float innerRadius, float outerRadius, const RTMaterial& material );
	~RTTorus();

	const RTIntersection intersect( const RTRay &ray )const;
	const SurfacePointData getSurfacePointData( const RTIntersection &intersection )const;

	void computeAABBbounds();
  private:
	vec3 mAxis;
	float mInnerRadius;
	float mOuterRadius;

};
