#include "precomp.h"
#include "RTMaterial.h"
#include "Sampler.h"

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
	 pow_ = 2;
 }

const vec3 RTMaterial::getAlbedoAtPoint( const float s, const float t, float z ) const
{
	if ( albedoTexture == 0 )
		return color;
	else
		return color * albedoTexture->getTexel( s, t,z, textureScale );
}

const vec3 RTMaterial::brdf( const RTIntersection &intersection, const vec3 &out, vec3 &in, double &pdf, vec3 &eye_pos ) const
{
	auto &surfacePointData = intersection.surfacePointData;
	float distance = ( surfacePointData.position - eye_pos ).length();
	const vec3 &brdf = this->getAlbedoAtPoint( surfacePointData.textureCoordinates.x, surfacePointData.textureCoordinates.y, distance );

	switch ( shadingType )
	{
	case DIFFUSE:
		sampleDiffuse( surfacePointData.normal, out, in, pdf );
		break;
	default:
		break;
	}

	return brdf;
}

const Tmpl8::vec3 RTMaterial::evaluate( const RTIntersection &intersection, const vec3 &out, vec3 &in, vec3 &eye_pos ) const
{
	auto &surfacePointData = intersection.surfacePointData;
	float distance = ( surfacePointData.position - eye_pos ).length();
	const vec3 &color = this->getAlbedoAtPoint( surfacePointData.textureCoordinates.x, surfacePointData.textureCoordinates.y, distance );

	if ( shadingType == Phong )
	{
		vec3 mid( normalize( in - out ) );
		return color * dot( surfacePointData.normal, in ) +
			   vec3( 1.5 ) * std::pow( dot( surfacePointData.normal, mid ), pow_ );
	}
	else
	{
		return color * dot( surfacePointData.normal, in );
	}
}

void RTMaterial::sampleDiffuse( const vec3& normal, const vec3 &out, vec3 &in, double &pdf ) const
{
	in = sampleCosHemisphere( normal );
}
