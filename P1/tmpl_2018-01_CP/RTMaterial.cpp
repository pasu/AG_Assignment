#include "precomp.h"
#include "RTMaterial.h"

 RTMaterial::RTMaterial( const vec3 &color, const ShadingType shadingType ) : color( color ), shadingType( shadingType )
{
}

const vec3 RTMaterial::getAlbedoAtPoint( const float s, const float t ) const
{
	 return color;
}