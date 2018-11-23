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
	if ( material.shadingType == DIFFUSE )
	{
		return shade_diffuse( castedRay, material, surfacePointData, depth );
	}
	else if ( material.shadingType == REFLECTIVE )
	{
		return material.reflectionFactor * shade_reflective( castedRay, material, surfacePointData, depth );
	}
	else if ( material.shadingType == TRANSMISSIVE_AND_REFLECTIVE )
	{
		vec3 refractionColor = vec3( 0 );
		vec3 reflectionColor = vec3( 0 );

		const vec3 &normal = surfacePointData.normal;

		const float reflectionFactor = fresnel( castedRay.dir, normal, material.indexOfRefraction );
		// compute refraction if it is not a case of total internal reflection
		if ( reflectionFactor < 1.0f )
			refractionColor = shade_transmissive( castedRay, material, surfacePointData, depth );
		reflectionColor = shade_reflective( castedRay, material, surfacePointData, depth );
		return reflectionColor * reflectionFactor + refractionColor * ( 1.0f - reflectionFactor );
	}
	else if ( material.shadingType == DIFFUSE_AND_REFLECTIVE )
	{
		vec3 reflectionColor = shade_reflective( castedRay, material, surfacePointData, depth );
		vec3 diffuseColor = shade_diffuse( castedRay, material, surfacePointData, depth );
		return material.reflectionFactor * reflectionColor + ( 1.0f - material.reflectionFactor ) * diffuseColor;
	}

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

const Tmpl8::vec3 RayTracer::shade_diffuse( const RTRay &castedRay, const RTMaterial &material, const SurfacePointData &surfacePointData, const int depth ) const
{
	vec3 color( 0.0f );

	const vec3 &albedo = material.getAlbedoAtPoint( surfacePointData.textureCoordinates.x, surfacePointData.textureCoordinates.y );
	//color = scene.ambientLight * albedo;

	for ( RTLight *light : scene.getLights() )
	{
		color += light->shade( surfacePointData, *this, material );
	}

	return color;
}

const Tmpl8::vec3 RayTracer::shade_reflective( const RTRay &castedRay, const RTMaterial &material, const SurfacePointData &surfacePointData, const int depth ) const
{
	vec3 nd = castedRay.dir - ( ( 2 * castedRay.dir.dot( surfacePointData.normal ) ) * surfacePointData.normal );
	RTRay refRay = RTRay( surfacePointData.position + 0.0001 * nd, nd );
	return castRay( refRay, depth + 1 );
}

const Tmpl8::vec3 RayTracer::shade_transmissive( const RTRay &castedRay, const RTMaterial &material, const SurfacePointData &surfacePointData, const int depth ) const
{
	const vec3 &normal = surfacePointData.normal; // normal texture later
	const vec3 &albedo = material.getAlbedoAtPoint( surfacePointData.textureCoordinates.x, surfacePointData.textureCoordinates.y );
	const bool outside = dot( castedRay.dir, normal ) < 0;
	const vec3 bias = renderOptions.shadowBias * normal;
	const vec3 &refractionDirection = refract( castedRay.dir, normal, material.indexOfRefraction );
	const vec3 refractionRayOrig = outside ? surfacePointData.position - bias : surfacePointData.position + bias;
	const RTRay refractionRay( refractionRayOrig, refractionDirection );
	return castRay( refractionRay, depth + 1 ) * albedo;
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
