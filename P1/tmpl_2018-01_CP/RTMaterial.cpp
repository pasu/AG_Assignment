#include "precomp.h"
#include "RTMaterial.h"

 RTMaterial::RTMaterial( const vec3 color, const ShadingType shadingType, const vec3 DRT_factors, const float fractionIndex )
	: shadingType( shadingType ), color( color ),
	  diffuseFactor( DRT_factors.x ),
	  reflectionFactor( DRT_factors.y ),
	  refractionFactor( DRT_factors.z ),
	  indexOfRefraction( fractionIndex ),
	  albedoTexture( nullptr )
{
 }

 RTMaterial::RTMaterial( const RTTexture *albedo, const ShadingType shadingType, const vec3 DRT_factors, vec2 textyreScale,const float fractionIndex )
	 :
	 shadingType( shadingType ),
	 color( vec3(1.0f) ),
	 diffuseFactor( DRT_factors.x ),
	 reflectionFactor( DRT_factors.y ),
	 refractionFactor( DRT_factors.z ),
	 indexOfRefraction( fractionIndex ),
	   textureScale( textyreScale ),
	   albedoTexture( albedo )
 {
 }



const vec3 RTMaterial::getAlbedoAtPoint( const float s, const float t, float z ) const
{
	if ( albedoTexture )
		return albedoTexture->getTexel( s, t, z, textureScale );
	else
		return color;
}