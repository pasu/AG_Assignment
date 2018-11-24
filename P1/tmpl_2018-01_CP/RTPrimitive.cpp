#include "precomp.h"
#include "RTPrimitive.h"

RTPrimitive::RTPrimitive( vec3 center, const RTMaterial &material )
	: pos( center ), material(material)
{
}

RTPrimitive::RTPrimitive( const RTMaterial &material ) : material( material )
{
}
