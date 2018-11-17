#pragma once
#include "precomp.h"
class RTCamera
{
public:
	vec3 getEye()
	{
		return eye;
	}
	void setEye( vec3 e )
	{
		eye = e;
	}

	RTCamera()
	{
		eye = vec3( 0, 0, 5 );
		ahead = vec3( 0, 0, -1 );
		right = vec3( 1, 0, 0 );
		up = vec3( 0, 1, 0 );
		tanFovHalf = vec2( 1, 1 );
	}

	vec3 rayDirFromNdc( const vec2 ndc )
	{
		return vec3::normalize( ahead + right * ndc.x * tanFovHalf.x + up * ndc.y * tanFovHalf.y );
	}

private:

	vec3 eye;
	vec3 ahead;
	vec3 right;
	vec3 up;

	vec2 tanFovHalf;
};
