#include "precomp.h"
#include "RayTracer.h"

RayTracer::RayTracer( const Scene &scene, const RenderOptions &renderOptions ) : renderOptions( renderOptions ), scene( scene )
{
	size = renderOptions.width * renderOptions.height * 4;
	pPixels = new char[size];
}

 RayTracer::~RayTracer()
{
	 if (pPixels)
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

	return 0xffffffff;
}
