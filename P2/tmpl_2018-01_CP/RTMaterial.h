#pragma once
#include "RTTexture.h"
#include "RTRay.h"
#include "RTSurfacePointData.h"

enum ShadingType
{
	DIFFUSE = 1,
	REFLECTIVE = 2,
	TRANSMISSIVE = 4,
	Phong = DIFFUSE | 8,
	Glossy = DIFFUSE | 16,
	TRANSMISSIVE_AND_REFLECTIVE = 32,
	DIFFUSE_AND_REFLECTIVE = 64,
	MICROFACET = 128
};

class RTIntersection;
class RTRay;

class RTMaterial
{
public:
  RTMaterial( const vec3 &color, const ShadingType shadingType );
  RTMaterial( const vec3 &color, const vec3 &emission, const ShadingType shadingType );
  RTMaterial( const RTTexture *albedo, const ShadingType shadingType );
  RTMaterial( const vec3 &color, const RTTexture *albedo, const ShadingType shadingType );
  RTMaterial( const vec3 &color, const RTTexture *albedo, const ShadingType shadingType, const float reflectionFactor );
  RTMaterial( const vec3 &color, const vec3 &emission, const RTTexture *albedo, const vec2 &textureScale, const ShadingType shadingType, const float reflectionFactor, const float indexOfRefraction );
  
  const ShadingType shadingType;

  const vec3 getAlbedoAtPoint( const float s,float z, const float t ) const;
  float brdf( const RTRay &ray, const SurfacePointData &hitPnt, const RTRay &ray_random ) const;
  bool evaluate( const RTRay &ray, const SurfacePointData &hitPnt, vec3 &eye_pos, vec3 &random_dir, float &pdf, vec3 &albedo ) const;
  const vec3 getEmission()const;
  bool isLight()const;

  float reflectionFactor;
  float indexOfRefraction;

  vec2 textureScale;

private:
  void sampleDiffuse( const vec3 &normal, const vec3 &out, vec3 &in, float &pdf ) const;

private:
  const vec3 color;
  const RTTexture *albedoTexture;
  vec3 emission;
  bool bLight;

public:

  float pow_;
  float k_;

};