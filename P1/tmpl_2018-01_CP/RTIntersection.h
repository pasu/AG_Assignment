#pragma once
#include "RTPrimitive.h"
#include "RTRay.h"
class RTIntersection
{
  public:
	RTIntersection();
	RTIntersection( const RTRay *ray, const RTPrimitive *object, const float rayT );

public:
	const RTRay *ray;
	const RTPrimitive *object;
	float rayT;

	inline bool isIntersecting() const { return rayT >= 0; }
	inline const vec3 getPos() const { return ray->orig + ray->dir * rayT; }
};
