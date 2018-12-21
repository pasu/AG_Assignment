#pragma once
#include "RTBox.h"
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

  public:
	vec3 min;
	vec3 max;

};
