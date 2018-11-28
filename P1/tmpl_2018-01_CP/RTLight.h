#pragma once
#include "RTSurfacePointData.h"
#include "template.h"
#include"RTMaterial.h"
class RayTracer;
class RTLight
{
  protected:
	vec3 color;
	float power;
	float currentIndexOfRefraction;
  public:
	RTLight( vec3 _color, float _power );
	~RTLight();

	static RTLight *createPointLight( vec3 _color, float _power, vec3 _pos );
	static RTLight *createParralleLight( vec3 _color, float _power, vec3 _direction );

	virtual vec3 shade( const SurfacePointData & pd, const RayTracer &rt, const vec3& texture) = 0; // shade a diffused surface
};
