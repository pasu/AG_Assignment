#include "RayTracer.h"
#include "precomp.h"

RayTracer::RayTracer( const Scene &scene, const RenderOptions &renderOptions ) : renderOptions( renderOptions ), scene( scene )
{
	size = renderOptions.width * renderOptions.height;
	pPixels = new int[size];
}

RayTracer::~RayTracer()
{
	if ( pPixels )
	{
		delete[] pPixels;
		pPixels = NULL;
	}
}

void RayTracer::render( Surface *screen ) const
{
	for ( int y = 0; y < renderOptions.height; ++y )
	{
		for ( int x = 0; x < renderOptions.width; ++x )
		{
			RTRay r = generatePrimaryRay( x, y );
			int color = castRay( r, 0 );

			pPixels[y * renderOptions.width + x] = color;
		}
	}

	memcpy( screen->GetBuffer(), pPixels, size );
}

const RTRay RayTracer::generatePrimaryRay( const int x, const int y ) const
{
	vec3 origin = scene.getCamera()->getEye();
	vec2 ndcPixelCentre( ( 2.0f * x - renderOptions.width ) / renderOptions.width, ( 2.0f * y - renderOptions.height ) / renderOptions.height );
	vec3 dir = scene.getCamera()->rayDirFromNdc( ndcPixelCentre );
	return RTRay( origin, dir );
}

const int RayTracer::castRay( const RTRay &ray, const int depth ) const
{
	if ( depth > renderOptions.maxRecursionDepth )
		return 0; // black

	const RTIntersection &intersection = findNearestObjectIntersection( ray );

	if ( intersection.isIntersecting() )
	{
		const SurfacePointData &surfacePointData = intersection.object->getSurfacePointData( intersection );
		const RTMaterial &material = intersection.object->getMaterial();

		return shade( ray, material, surfacePointData, depth );
	}
	else
		return scene.backgroundColorPixel;
}

const int RayTracer::shade( const RTRay &castedRay, const RTMaterial &material, const SurfacePointData &surfacePointData, const int depth ) const
{
	if ( material.shadingType == DIFFUSE )
	{
	}
	else if ( material.shadingType == REFLECTIVE )
	{
	}
	else if ( material.shadingType == TRANSMISSIVE_AND_REFLECTIVE )
	{
	}
	else if ( material.shadingType == DIFFUSE_AND_REFLECTIVE )
	{
	}
	
	return 0;
}

const RTIntersection RayTracer::findNearestObjectIntersection( const RTRay &ray ) const
{
	auto objects = scene.getObjects();

	RTIntersection nearestIntersection;

	for ( auto it = objects.begin(); it != objects.end(); ++it )
	{
		const RTIntersection &intersection = ( *it )->intersect( ray );

		if ( intersection.isIntersecting() &&
			 ( !nearestIntersection.isIntersecting() || intersection.rayT < nearestIntersection.rayT ) )
		{
			nearestIntersection = intersection;
		}
	}
	return nearestIntersection;
}
