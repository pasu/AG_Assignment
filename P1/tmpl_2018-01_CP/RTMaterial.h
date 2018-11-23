#pragma once

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
  
  const ShadingType shadingType;

  const vec3 getAlbedoAtPoint( const float s, const float t ) const;

  float reflectionFactor;

private:
  const vec3 color;
};