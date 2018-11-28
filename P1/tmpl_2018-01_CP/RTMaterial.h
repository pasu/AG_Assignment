#pragma once
#include "RTTexture.h"

enum ShadingType
{
	DIFFUSE,
	REFLECTIVE,
	TRANSMISSIVE_AND_REFLECTIVE,
	DIFFUSE_AND_REFLECTIVE
};

class RTMaterial
{
public:
  RTMaterial( const vec3 &color, const ShadingType shadingType );
  RTMaterial( const RTTexture *albedo, const ShadingType shadingType );
  RTMaterial( const vec3 &color, const RTTexture *albedo, const ShadingType shadingType );
  RTMaterial( const vec3 &color, const RTTexture *albedo, const ShadingType shadingType, const float reflectionFactor );
  RTMaterial( const vec3 &color, const RTTexture *albedo, const vec2 &textureScale, const ShadingType shadingType, const float reflectionFactor, const float indexOfRefraction );
  
  const ShadingType shadingType;

  const vec3 getAlbedoAtPoint( const float s,float z, const float t ) const;

  float reflectionFactor;
  float indexOfRefraction;

  vec2 textureScale;

private:
  const vec3 color;
  const RTTexture *albedoTexture;
};