#include "RayTracer.h"
#include "precomp.h"
#include "fxaa.h"
RayTracer::RayTracer( const Scene &scene, const RenderOptions &renderOptions ) : renderOptions( renderOptions ), scene( scene )
{
	size = renderOptions.width * renderOptions.height;
	pPixels = new unsigned int[size];
	hdrPixels = new vec3[size];
}

RayTracer::~RayTracer()
{
	if ( pPixels )
	{
		delete[] pPixels;
		pPixels = NULL;
	}
}

void RayTracer::traceChunk( int x_min, int x_max, int y_min, int y_max )
{
#ifdef BVH_PARTITION_TRAVERSAL
	for ( int y = y_min; y <= y_max; y += RAYPACKET_DIM )
	{
		for ( int x = x_min; x <= x_max; x += RAYPACKET_DIM )
		{
			RayPacket rp(*this,vec2(x,y));

			vec3 colors[RAYPACKET_RAYS_PER_PACKET];
			this->castRayPacket( rp, colors );

			for ( unsigned int y0 = 0; y0 < RAYPACKET_DIM; ++y0 )
			{
				for ( unsigned int x0 = 0; x0 < RAYPACKET_DIM; ++x0 )
				{
					hdrPixels[( y + y0 ) * renderOptions.width + ( x + x0 )] = colors[y0 * RAYPACKET_DIM + x0];
				}
			}
			
		}
	}
#else
			for ( int y = y_min; y <= y_max; ++y )
			{
				for ( int x = x_min; x <= x_max; ++x )
				{
					RTRay r = generatePrimaryRay( x, y );
					RTIntersection intersection;
					hdrPixels[y * renderOptions.width + x] = castRay( r, 0, intersection );
				}
			}
#endif
}

void RayTracer::render( Surface *screen )
{
	scene.getCamera()->Update();
	
#if NUMBER_THREAD==1
    traceChunk( 0, renderOptions.width - 1, 0, renderOptions.height - 1 );
#else
	int chunkWidth = renderOptions.width / NUMBER_THREAD;
	for (int i = 0; i < NUMBER_THREAD; i++)
	{
		int x_min = chunkWidth * i;
		int x_max = std::min(x_min + chunkWidth,renderOptions.width) - 1;
		startRenderThread( x_min, x_max, 0, renderOptions.height - 1 );
	}
	
	waitRenderThreads();
#endif
	//runFXAA( hdrPixels, renderOptions.width, renderOptions.height );


	for (int y = 0; y < renderOptions.height; ++y) {
		for (int x = 0; x < renderOptions.width; ++x) {
			auto &color = hdrPixels[y * renderOptions.width + x];
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

const vec3 RayTracer::castRay( const RTRay &ray, const int depth, RTIntersection &intersection ) const
{
	if ( depth > renderOptions.maxRecursionDepth )
		return scene.backgroundColor;

	intersection = findNearestObjectIntersection( ray );

	if ( intersection.isIntersecting() )
	{
		

		return shade( ray, intersection, depth );
	}
	else
		return scene.backgroundColor;
}

void RayTracer::castRayPacket( const RayPacket &raypacket, vec3* colors ) const
{
	int depth = 0;
	RTIntersection intersections[RAYPACKET_RAYS_PER_PACKET];

	scene.findNearestObjectIntersection( raypacket,intersections);

	for ( int i = 0; i < RAYPACKET_RAYS_PER_PACKET ;i++)
	{
		RTIntersection &current = intersections[i];

		if ( current.isIntersecting() )
		{
			current.surfacePointData = current.object->getSurfacePointData( current );

			colors[i] = shade( raypacket.m_ray[i], current, depth );
		}
		else
			colors[i] = scene.backgroundColor;
	}
}


const vec3 RayTracer::shade( const RTRay &castedRay, const RTIntersection &intersection, const int depth ) const
{
	auto &surfacePointData = intersection.surfacePointData;
	const RTMaterial &material = intersection.object->getMaterial();
	float distance = ( surfacePointData.position - scene.getCamera()->getEye() ).length();
	const vec3 &albedo = material.getAlbedoAtPoint( surfacePointData.textureCoordinates.x, surfacePointData.textureCoordinates.y, distance );

	//return 0x00ff0000;
	if ( material.shadingType == DIFFUSE )
	{
		return shade_diffuse( castedRay, intersection, depth );
	}
	else if ( material.shadingType == REFLECTIVE )
	{
		return material.reflectionFactor * shade_reflective( castedRay, intersection, depth );
	}
	else if ( material.shadingType == TRANSMISSIVE_AND_REFLECTIVE )
	{
		vec3 refractionColor = vec3( 0 );
		vec3 reflectionColor = vec3( 0 );

		const vec3 &normal = surfacePointData.normal;

		float reflectionFactor;
		reflectionFactor = fresnel( castedRay.dir, normal, material.indexOfRefraction );
		//reflectionFactor = material.reflectionFactor;
		// compute refraction if it is not a case of total internal reflection
		if (reflectionFactor < 1.0f)
		{
			RTIntersection intersectionObj;
			refractionColor = shade_transmissive( castedRay, intersection, depth, intersectionObj );
			//refractionColor = vec3( 0, 0, 0 );

			if (intersectionObj.isInSideObj())
			{
				// Burger-Lambert-Beer law
				vec3 absorbance = albedo * 0.15f * ( -intersectionObj.rayT );
				vec3 transparency = vec3( expf( absorbance.x ), expf( absorbance.y ), expf( absorbance.z ) );
				refractionColor = refractionColor * transparency;
			}
		}
		reflectionColor = shade_reflective( castedRay, intersection, depth );
		//reflectionColor = vec3( 1, 1, 1 );
		return reflectionColor * reflectionFactor + refractionColor * ( 1.0f - reflectionFactor );
	}
	else if ( material.shadingType == DIFFUSE_AND_REFLECTIVE )
	{
		vec3 reflectionColor = shade_reflective( castedRay, intersection, depth );
		vec3 diffuseColor = shade_diffuse( castedRay, intersection, depth );
		return material.reflectionFactor * reflectionColor + ( 1.0f - material.reflectionFactor ) * diffuseColor;
	}

}

RTIntersection RayTracer::findNearestObjectIntersection( const RTRay &ray ) const
{
	return scene.findNearestObjectIntersection( ray );
}

const Tmpl8::vec3 RayTracer::shade_diffuse( const RTRay &castedRay, const RTIntersection &intersection, const int depth ) const
{
	auto &surfacePointData = intersection.surfacePointData;
	const RTMaterial &material = intersection.object->getMaterial();
	vec3 color( 0.0f );
	if ( castedRay.dir.dot( surfacePointData.normal ) > 0 )
	{
		return color;
	}
	const vec3 albedo = material.getAlbedoAtPoint( surfacePointData.textureCoordinates.x, surfacePointData.textureCoordinates.y,castedRay.distance_traveled+intersection.rayT );
	color = scene.ambientLight * albedo;
	static auto &light_list = scene.getLights();
	for ( RTLight *light : light_list )
	{
		color += light->shade( surfacePointData, *this, albedo );
	}

	return color;
}

const Tmpl8::vec3 RayTracer::shade_reflective( const RTRay &castedRay, const RTIntersection &intersection, const int depth ) const
{
	auto &surfacePointData = intersection.surfacePointData;
	const RTMaterial &material = intersection.object->getMaterial();
	vec3 nd = castedRay.dir - ( ( 2 * castedRay.dir.dot( surfacePointData.normal ) ) * surfacePointData.normal );
	RTRay refRay = RTRay( surfacePointData.position + renderOptions.shadowBias * nd, nd, castedRay.distance_traveled+intersection.rayT );

	RTIntersection intersection2;
	return castRay( refRay, depth + 1, intersection2 );
}

const Tmpl8::vec3 RayTracer::shade_transmissive( const RTRay &castedRay, const RTIntersection &intersection, const int depth, RTIntersection &intersectionObj ) const
{
	auto &surfacePointData = intersection.surfacePointData;
	const RTMaterial &material = intersection.object->getMaterial();
	const vec3 &normal = surfacePointData.normal; // normal texture later

	float distance = ( surfacePointData.position - scene.getCamera()->getEye() ).length();
	const vec3 &albedo = material.getAlbedoAtPoint( surfacePointData.textureCoordinates.x, surfacePointData.textureCoordinates.y,distance );
	const bool outside = dot( castedRay.dir, normal ) < 0;
	const vec3 bias = renderOptions.shadowBias * normal;
	const vec3 &refractionDirection = refract( castedRay.dir, normal, material.indexOfRefraction );
	const vec3 refractionRayOrig = outside ? surfacePointData.position - bias : surfacePointData.position + bias;
	const RTRay refractionRay( refractionRayOrig, refractionDirection,castedRay.distance_traveled + intersection.rayT );

	return castRay( refractionRay, depth + 1, intersectionObj ) * albedo;
}

float RayTracer::fresnel( const vec3 &I, const vec3 &N, const float refractionIndex ) const
{
	float cosi = Utils::clamp_rt( dot( I, N ), -1.0f, 1.0f );
	float etai = 1, etat = refractionIndex;
	if ( cosi > 0 )
		std::swap( etai, etat );

	//Snell's law
	float sint = etai / etat * sqrtf( std::max( 0.0f, 1.0f - cosi * cosi ) );

	// Total internal reflection
	if ( sint >= 1 )
	{
		return 1.0f;
	}
	else
	{
		float cost = sqrtf( std::max( 0.0f, 1.0f - sint * sint ) );
		cosi = fabsf( cosi );
		float Rs = ( ( etat * cosi ) - ( etai * cost ) ) / ( ( etat * cosi ) + ( etai * cost ) );
		float Rp = ( ( etai * cosi ) - ( etat * cost ) ) / ( ( etai * cosi ) + ( etat * cost ) );
		return ( Rs * Rs + Rp * Rp ) / 2.0f;
	}

}

const Tmpl8::vec3 RayTracer::refract( const vec3 &I, const vec3 &N, const float refractionIndex ) const
{
	float cosi = Utils::clamp_rt( dot( I, N ), -1.0f, 1.0f );
	float etai = 1, etat = refractionIndex;
	vec3 n = N;
	if ( cosi < 0.0f )
		cosi = -cosi;
	else
	{
		std::swap( etai, etat );
		n = -N;
	}
	float eta = etai / etat;
	float k = 1.0f - eta * eta * ( 1 - cosi * cosi );

	//k < 0 = total internal reflection
	return k < 0.0f ? vec3( 0.0f ) : ( eta * I ) + ( eta * cosi - sqrtf( k ) ) * n;
}

