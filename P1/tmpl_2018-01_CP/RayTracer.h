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
	const vec3 shade( const RTRay &castedRay, const RTIntersection &intersection, const int depth ) const;

	RTIntersection findNearestObjectIntersection( const RTRay &ray ) const;

	const RenderOptions &getRenderOptions() const { return renderOptions; }

  private:
	const vec3 shade_diffuse( const RTRay &castedRay, const RTIntersection &intersection, const int depth ) const;
	const vec3 shade_reflective( const RTRay &castedRay, const RTIntersection &intersection, const int depth ) const;
	const vec3 shade_transmissive( const RTRay &castedRay, const RTIntersection &intersection, const int depth ) const;

	float fresnel( const vec3 &I, const vec3 &N, const float refractionIndex ) const; //return ratio of reflected light
	const vec3 refract( const vec3 &I, const vec3 &N, const float refractionIndex ) const;



  private:
	const RenderOptions &renderOptions;
	const Scene &scene;
	int size;
	unsigned  int *pPixels;
};
