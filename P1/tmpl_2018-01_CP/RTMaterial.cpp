#include "precomp.h"
#include "RTMaterial.h"

 RTMaterial::RTMaterial( const vec3 &color, const ShadingType shadingType ) 
	 : RTMaterial( color, 0, vec2( 1 ), shadingType, 0.8f, 1.0f )
{
}

 RTMaterial::RTMaterial( const RTTexture *albedo, const ShadingType shadingType )
	: RTMaterial( vec3( 1.0f ), albedo, vec2( 1 ), shadingType, 0.8f, 1.0f )
{
}

 RTMaterial::RTMaterial( const vec3 &color, const RTTexture *albedo, const ShadingType shadingType )
	: RTMaterial( color, albedo, vec2( 1 ), shadingType, 0.8f, 1.0f )
{
}

 RTMaterial::RTMaterial( const vec3 &color, const RTTexture *albedo, const ShadingType shadingType, const float reflectionFactor )
	:RTMaterial( color, albedo, vec2( 1 ), shadingType, reflectionFactor, 1.0f )
{
}

 RTMaterial::RTMaterial( const vec3 &color, const RTTexture *albedo, const vec2 &textureScale, const ShadingType shadingType, const float reflectionFactor, const float indexOfRefraction )
	:color( color ),
	  albedoTexture( albedo ), shadingType( shadingType ),
	reflectionFactor( reflectionFactor ), indexOfRefraction( indexOfRefraction ), textureScale( textureScale )
{
}

const vec3 RTMaterial::getAlbedoAtPoint( const float s, const float t ) const
{
	if ( albedoTexture == 0 )
		return color;
	else
		return color * albedoTexture->getTexel( s, t, textureScale );
}