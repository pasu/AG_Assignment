#pragma once
#include "RTTexture.h"
typedef unsigned int ShadingType;
constexpr ShadingType DIFFUSE = 1;
constexpr ShadingType REFLECT = 2;
constexpr ShadingType REFRACT = 4;

class RTMaterial
{
  public:
	RTMaterial( const vec3 color, const ShadingType shadingType, const vec3 DRT_factors, const float fractionIndex = 1.0f );
	RTMaterial( const RTTexture *albedo, const ShadingType shadingType, const vec3 DRT_factors, vec2 textyreScale = vec2(1,1),const float fractionIndex = 1.0f );

	const ShadingType shadingType;

	const vec3 getAlbedoAtPoint( const float s, float z, const float t ) const;

	float diffuseFactor;
	float reflectionFactor;
	float refractionFactor;
	
	float indexOfRefraction;

	float highlight;

	vec2 textureScale;

  private:
	const vec3 color;
	const RTTexture *albedoTexture;
};