#include "precomp.h"
#include "RTPrimitive.h"

RTPrimitive::RTPrimitive( vec3 center, const RTMaterial &material )
	: pos( center ), material(material)
{
}

RTPrimitive::RTPrimitive( const RTMaterial &material ) : pos(vec3(0.0f)),material( material )
{
}

AABB RTPrimitive::getAABB()
{
	return box;
}

vec3 RTPrimitive::getCentroid()
{
	return pos;
}
