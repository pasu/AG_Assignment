#pragma once
#include "RTMaterial.h"
#include "RTRay.h"
#include "RTSurfacePointData.h"
class RTIntersection;

class RTPrimitive
{
  public:
	RTPrimitive( const vec3 center, const RTMaterial& material);
	virtual const RTIntersection intersect( const RTRay &ray ) const = 0;
	virtual const SurfacePointData getSurfacePointData( const RTIntersection &intersection ) const = 0;

	inline const RTMaterial &getMaterial() const { return material; }

  private:
	vec3 pos;
	const RTMaterial& material;
};
