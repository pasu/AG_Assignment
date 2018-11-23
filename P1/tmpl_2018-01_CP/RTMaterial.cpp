#include "precomp.h"
#include "RTMaterial.h"

 RTMaterial::RTMaterial( const vec3 &color, const ShadingType shadingType ) : color( color ), shadingType( shadingType ), reflectionFactor( 0.8f ), indexOfRefraction(1.0f)
{
}

const vec3 RTMaterial::getAlbedoAtPoint( const float s, const float t ) const
{
	 return color;
}