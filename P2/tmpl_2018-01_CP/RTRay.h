#pragma once
class RTRay
{
  public:
	vec3 orig;
	vec3 dir;

	float distance_traveled;// for looking up in mip map table

	RTRay();
	RTRay( const vec3 &o, const vec3 &d, float distance_start =.0f );

	vec3 getPointAt( float distance ) const
	{
		return orig + dir * distance;
	}
};
