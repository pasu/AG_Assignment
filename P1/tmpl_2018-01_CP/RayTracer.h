#pragma once
#include "RenderOptions.h"
#include "Scene.h"
#include "RTRay.h"

class RayTracer
{
  public:
	RayTracer( const Scene &scene, const RenderOptions &renderOptions );
	~RayTracer();
	void render( Surface *screen ) const;
	
	const RTRay generatePrimaryRay( const int x, const int y ) const;

	const int castRay( const RTRay &ray, const int depth ) const; 

  private:
	const RenderOptions &renderOptions;
	const Scene &scene;
	int size;
	char *pPixels;
};
