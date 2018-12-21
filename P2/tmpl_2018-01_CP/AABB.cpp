#include "precomp.h"
#include "AABB.h"


AABB::AABB()
{
}

 AABB::AABB( const vec3 &min, const vec3 &max )
	: min( min ), max( max )
{
}

 AABB::AABB( const vec3 &p )
	: min( p ), max( p )
{
}

 AABB::AABB( const RTBox &box )
	: min( box.min ), max( box.max )
{
}

void AABB::expandToInclude( const vec3 &p )
{
	min = ::min( min, p );
	max = ::max( max, p );
}

void AABB::expandToInclude( const AABB &b )
{
	min = ::min( min, b.min );
	max = ::max( max, b.max );
}

AABB::~AABB()
{
}
