#pragma once
#include "precomp.h"

class LocalSpace
{
  public:
	LocalSpace();
	~LocalSpace();

	vec3 u,v,w;

	void init(const vec3 normal)
	{
		w = normal;
		w.normalize();
		vec3 a = ( fabs( normal.x ) > 0.99f ) ? vec3( 0, 1, 0 ) : vec3( 1, 0, 0 );
		u = w.cross( a );
		v = u.cross(w);
		u.normalize();
		v.normalize();
	}

	vec3 local( float a, float b, float c )
	{
		return a * u + b * v + c * w; 
	}

	vec3 local( const vec3 &vec ) 
	{ 
		return vec.x * u + vec.y * v + vec.z * w; 
	}
	vec3 world(const vec3 &vec)
	{
		return vec3( u.dot( vec ), v.dot( vec ), w.dot( vec ) );
	}
};
