#pragma once
#include "RTSurfacePointData.h"
#include "template.h"
#include"RTMaterial.h"
class RayTracer;
class RTLight
{
  private:
	vec3 color;
	float power;

  public:
	RTLight( vec3 _color, float _power );
	~RTLight();

	RTLight *createPointLight( vec3 _color, float _power, vec3 _pos );
	RTLight *createParralleLight( vec3 _color, float _power, vec3 _direction );

	virtual vec3 shade( const SurfacePointData & pd, const RayTracer &rt, const RTMaterial & material) = 0; // shade a diffused surface
};
