#pragma once
#include "RenderOptions.h"
#include "Scene.h"

class RayTracer
{
  public:
	RayTracer( const Scene &scene, const RenderOptions &renderOptions );
	void render( Surface *screen ) const;

  private:
	const RenderOptions &renderOptions;
	const Scene &scene;
};
