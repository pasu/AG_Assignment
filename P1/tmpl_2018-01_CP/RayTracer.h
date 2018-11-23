#pragma once
#include "RenderOptions.h"
#include "Scene.h"
#include "RTRay.h"
#include "RTIntersection.h"

class RayTracer
{
  public:
	RayTracer( const Scene &scene, const RenderOptions &renderOptions );
	~RayTracer();
	void render( Surface *screen ) const;
	
	const RTRay& generatePrimaryRay( const int x, const int y ) const;

	const vec3 castRay( const RTRay &ray, const int depth ) const; 
	const vec3 shade( const RTRay &castedRay, const RTMaterial &material, const SurfacePointData &surfacePointData, const int depth ) const;

	const RTIntersection findNearestObjectIntersection( const RTRay &ray ) const;

  private:
	const vec3 shade_diffuse( const RTRay &castedRay, const RTMaterial &material, const SurfacePointData &surfacePointData, const int depth ) const;
	const vec3 shade_reflective( const RTRay &castedRay, const RTMaterial &material, const SurfacePointData &surfacePointData, const int depth ) const;


  private:
	const RenderOptions &renderOptions;
	const Scene &scene;
	int size;
	unsigned  int *pPixels;
};
