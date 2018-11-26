#pragma once
class RTRay
{
  public:
	vec3 orig;
	vec3 dir;

	RTRay();
	RTRay( const vec3 &o, const vec3 &d );

	vec3 getPointAt( float distance ) const
	{
		return orig + dir * distance;
	}
};
