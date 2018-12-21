#pragma once
#include "precomp.h"
#include "RTRay.h"

class RTBox;

class AABB
{
  public:
	AABB();
	~AABB();

	AABB( const vec3 &min, const vec3 &max );
	AABB( const vec3 &p );
	AABB( const RTBox &box );

	void expandToInclude( const vec3 &p );
	void expandToInclude( const AABB &b );
	uint32_t maxDimension() const;

	bool intersect( const RTRay &ray, float *tnear, float *tfar ) const;

  public:
	vec3 min;
	vec3 max;

};
