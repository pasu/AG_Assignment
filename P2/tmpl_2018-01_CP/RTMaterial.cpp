#include "precomp.h"
#include "RTMaterial.h"
#include "Sampler.h"
#include "Utils.h"

 RTMaterial::RTMaterial( const vec3 &color, const ShadingType shadingType ) 
	 : RTMaterial( color, vec3( 0 ), 0, vec2( 1 ), shadingType, 0.8f, 1.0f )
{
}

 RTMaterial::RTMaterial( const vec3 &color, const vec3 &emission, const ShadingType shadingType )
  : RTMaterial( color,emission, 0, vec2( 1 ), shadingType, 0.8f, 1.0f )
  {
	 
  }
 RTMaterial::RTMaterial( const RTTexture *albedo, const ShadingType shadingType )
	  : RTMaterial( vec3( 1.0f ), vec3( 0 ), albedo, vec2( 1 ), shadingType, 0.8f, 1.0f )
  {
}

 RTMaterial::RTMaterial( const vec3 &color, const RTTexture *albedo, const ShadingType shadingType )
	: RTMaterial( color, vec3( 0 ), albedo, vec2( 1 ), shadingType, 0.8f, 1.0f )
{
}

 RTMaterial::RTMaterial( const vec3 &color, const RTTexture *albedo, const ShadingType shadingType, const float reflectionFactor )
	: RTMaterial( color, vec3( 0 ), albedo, vec2( 1 ), shadingType, reflectionFactor, 1.0f )
{
}

 RTMaterial::RTMaterial( const vec3 &color, const vec3 &emission, const RTTexture *albedo, const vec2 &textureScale, const ShadingType shadingType, const float reflectionFactor, const float indexOfRefraction )
	: color( color ), emission( emission ),
	  albedoTexture( albedo ), shadingType( shadingType ),
	reflectionFactor( reflectionFactor ), indexOfRefraction( indexOfRefraction ), textureScale( textureScale )
{
	 pow_ = 2;	
	 bLight = false;

	 this->emission = emission;
	 if ( !Utils::floatEquals( this->emission.x, 0.0f ) ||
		  !Utils::floatEquals( this->emission.y, 0.0f ) ||
		  !Utils::floatEquals( this->emission.z, 0.0f ) )
	 {
		 bLight = true;
	 }
 }

const vec3 RTMaterial::getAlbedoAtPoint( const float s, const float t, float z ) const
{
	if ( albedoTexture == 0 )
		return color;
	else
		return color * albedoTexture->getTexel( s, t,z, textureScale );
}

float RTMaterial::brdf( const RTRay &ray, const SurfacePointData &hitPnt, const RTRay &ray_random ) const
{
	float cosine = 0;
	switch ( shadingType )
	{
	case DIFFUSE:
		cosine = hitPnt.normal.dot( ray_random.dir );
		cosine = cosine < 0.0 ? 0.0f : cosine;
		break;
	}

	return cosine * Utils::INV_PI;
}

// const Tmpl8::vec3 RTMaterial::evaluate( const RTIntersection &intersection, const vec3 &out, vec3 &in, vec3 &eye_pos ) const
// {
// 	auto &surfacePointData = intersection.surfacePointData;
// 	float distance = ( surfacePointData.position - eye_pos ).length();
// 	const vec3 &color = this->getAlbedoAtPoint( surfacePointData.textureCoordinates.x, surfacePointData.textureCoordinates.y, distance );
// 
// 	if ( shadingType == Phong )
// 	{
// 		vec3 mid( normalize( in - out ) );
// 		return color * dot( surfacePointData.normal, in ) +
// 			   vec3( 1.5 ) * std::pow( dot( surfacePointData.normal, mid ), pow_ );
// 	}
// 	else
// 	{
// 		return color * dot( surfacePointData.normal, in );
// 	}
// }

bool RTMaterial::evaluate( const RTRay &ray, const SurfacePointData &hitPnt, vec3 &eye_pos, vec3 &random_dir, float &pdf, vec3 &albedo ) const
{
	float distance = ( hitPnt.position - eye_pos ).length();
	albedo = this->getAlbedoAtPoint( hitPnt.textureCoordinates.x, hitPnt.textureCoordinates.y, distance );
		
	switch ( shadingType )
	{
	case DIFFUSE:
		sampleDiffuse( hitPnt.normal, ray.dir, random_dir, pdf );
		return true;
		break;
	default:
		break;
	}
}

const Tmpl8::vec3 RTMaterial::getEmission()const
{
	return emission;
}

bool RTMaterial::isLight()const
{
	return bLight;
}

void RTMaterial::sampleDiffuse( const vec3& normal, const vec3 &out, vec3 &in, float &pdf ) const
{
	in = sampleCosHemisphere( normal );
	pdf = in.dot( normal ) * Utils::INV_PI;
}
