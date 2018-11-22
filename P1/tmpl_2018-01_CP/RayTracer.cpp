#include "RayTracer.h"
#include "precomp.h"

RayTracer::RayTracer( const Scene &scene, const RenderOptions &renderOptions ) : renderOptions( renderOptions ), scene( scene )
{
	size = renderOptions.width * renderOptions.height;
	pPixels = new unsigned int[size];
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
	scene.getCamera()->Update();
	for ( int y = 0; y < renderOptions.height; ++y )
	{
		for ( int x = 0; x < renderOptions.width; ++x )
		{
			RTRay r = generatePrimaryRay( x, y );
			vec3 color = castRay( r, 0 );
#define lmt( x ) ( ( x ) < 255 ? ( x ) : 255 )
			unsigned int colorf = 0xff000000 | lmt( (unsigned int)( color.z * 255 ) ) | lmt( (unsigned int)( color.y * 255 ) ) << 8 | lmt( (unsigned int)( color.x * 255 ) ) << 16;
#undef lmt
			pPixels[y * renderOptions.width + x] = colorf;
		}
	}

	memcpy( screen->GetBuffer(), pPixels, size * 4 );
}

const RTRay &RayTracer::generatePrimaryRay( const int x, const int y ) const
{
	vec3 origin = scene.getCamera()->getEye();
	vec2 ndcPixelCentre( ( 2.0f * x - renderOptions.width ) / renderOptions.width, ( renderOptions.height - 2.0f * y ) / renderOptions.height );
	vec3 dir = scene.getCamera()->rayDirFromNdc( ndcPixelCentre );
	return RTRay( origin, dir );
}

const vec3 RayTracer::castRay( const RTRay &ray, const int depth ) const
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

const vec3 RayTracer::shade( const RTRay &castedRay, const RTMaterial &material, const SurfacePointData &surfacePointData, const int depth ) const
{
	//return 0x00ff0000;
	vec3 color = vec3( .0f, .0f, .0f );
	if ( material.shadingType == DIFFUSE )
	{
		
		for ( RTLight *light : scene.getLights() )
		{
			color += light->shade( surfacePointData, *this, material );
		}
	}
	else if ( material.shadingType == REFLECTIVE )
	{
		vec3 nd = castedRay.dir - ( ( 2 * castedRay.dir.dot( surfacePointData.normal ) ) * surfacePointData.normal );
		RTRay refRay = RTRay( surfacePointData.position + 0.0001 * nd, nd );
		color = castRay( refRay, depth + 1 );
	}
	else if ( material.shadingType == TRANSMISSIVE_AND_REFLECTIVE )
	{
	}
	else if ( material.shadingType == DIFFUSE_AND_REFLECTIVE )
	{
	}
	return color;

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
