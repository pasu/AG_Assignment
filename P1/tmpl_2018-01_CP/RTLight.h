#pragma once
#include "RTSurfacePointData.h"
#include "template.h"
#include"RTMaterial.h"
class RayTracer;
class RTLight
{
  private:
	int color;
	float power;

  public:
	RTLight( int _color, float _power );
	~RTLight();

	virtual vec3 shade( const SurfacePointData & pd, const RayTracer &rt, const RTMaterial & material) = 0; // shade a diffused surface
};
