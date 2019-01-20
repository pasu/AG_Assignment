#pragma once
#include "RTTexture.h"

enum ShadingType
{
	DIFFUSE = 1,
	REFLECTIVE = 2,
	TRANSMISSIVE = 4,
	Phong = DIFFUSE | 8,
	Glossy = DIFFUSE | 16,
	Retro = DIFFUSE | 32,
	Halton = 64,
	TRANSMISSIVE_AND_REFLECTIVE = 128,
	DIFFUSE_AND_REFLECTIVE = 256,
	EMITTANCE = 512
};

class RTIntersection;

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
  const vec3 brdf( const RTIntersection &intersection, const vec3 &out, vec3 &in, double &pdf, vec3& eye_pos)const;
  const vec3 evaluate( const RTIntersection &intersection, const vec3 &out, vec3 &in, vec3 &eye_pos ) const;

  float reflectionFactor;
  float indexOfRefraction;

  vec2 textureScale;

private:
  void sampleDiffuse( const vec3 &normal, const vec3 &out, vec3 &in, double &pdf ) const;

private:
  const vec3 color;
  const RTTexture *albedoTexture;

  int pow_;
};