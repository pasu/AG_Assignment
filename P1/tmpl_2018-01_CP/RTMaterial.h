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

private:
  const vec3 color;
};