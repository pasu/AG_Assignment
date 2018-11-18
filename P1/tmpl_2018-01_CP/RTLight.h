#pragma once
#include "RTSurfacePointData.h"
#include "template.h"
class Scene;

class RTLight
{
  private:
	int color;
	float power;

  public:
	RTLight( int _color, float _power );
	~RTLight();

	virtual vec3 shade(const SurfacePointData &, Scene & )const = 0; // shade a diffused surface
};
