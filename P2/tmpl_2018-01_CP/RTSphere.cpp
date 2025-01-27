#include "precomp.h"
#include "RTSphere.h"
#include "Utils.h"

RTSphere::RTSphere( const vec3 &position, const float radius, RTMaterial &material )
	: RTPrimitive( position, material ), radius( radius )
{
	computeAABBbounds();
}

const RTIntersection RTSphere::intersect( const RTRay &ray ) const
{
	RTIntersection intersection( &ray, this, -1.0f );

	vec3 C = pos - ray.orig;
	float radiusSquare = radius * radius;

	if ( C.sqrLentgh() > radiusSquare )
	{
		float t = dot( C, ray.dir );
		vec3 Q = C - t * ray.dir;
		float p2 = dot( Q, Q );

		t -= sqrt( radiusSquare - p2 );
		if ( t > 0.0f )
		{
			intersection.rayT = t;
		}
	}
	else
	{
		// analytic solution
		vec3 L = -C;
		float a = dot( ray.dir, ray.dir );
		float b = 2 * dot( ray.dir, L );
		float c = dot( L, L ) - ( radiusSquare );

		float t0, t1;

		if ( !Utils::solveQuadratic( a, b, c, t0, t1 ) )
			return intersection;

		if ( t0 < 0 )
		{
			t0 = t1;
			if ( t0 < 0 )
				return intersection;
		}

		intersection.rayT = t0;
	}

	return intersection;
}

const SurfacePointData RTSphere::getSurfacePointData( const RTIntersection &intersection ) const
{
	const vec3 &surfacePoint = intersection.getIntersectionPosition();
	vec3 localCoord = surfacePoint - pos;
	vec3 normal = normalize( localCoord );

	float theta = atan2( localCoord.x, localCoord.z );
	float phi = acos( localCoord.y / radius );

	vec2 texCoords = {0.5f + theta / ( 2.0f * Utils::RT_PI ), 1.0f - ( phi / Utils::RT_PI )};

	vec3 tangent( cos( theta ) * cos( phi ), cos( theta ) * sin( phi ), -sin( theta ) );
	vec3 bitangent( -sin( phi ), cos( phi ), 0 );

	return {normal, texCoords, surfacePoint, tangent, bitangent};
}

void RTSphere::computeAABBbounds()
{
	vec3 min = pos - vec3( radius );
	vec3 max = pos + vec3( radius );

	box = AABB( min, max );
}

RTInnerSphere::RTInnerSphere( const vec3 &position, const float radius, RTMaterial &material )
	: RTSphere(position,radius,material)
{
}

const SurfacePointData RTInnerSphere::getSurfacePointData( const RTIntersection &intersection ) const
{
	const vec3 &surfacePoint = intersection.getIntersectionPosition();
	vec3 localCoord = surfacePoint - pos;
	vec3 normal = -normalize( localCoord );

	float theta = atan2( localCoord.x, localCoord.z );
	float phi = acos( localCoord.y / radius );

	vec2 texCoords = {0.5f + theta / ( 2.0f * Utils::RT_PI ), 1.0f - ( phi / Utils::RT_PI )};

	vec3 tangent( cos( theta ) * cos( phi ), cos( theta ) * sin( phi ), -sin( theta ) );
	vec3 bitangent( -sin( phi ), cos( phi ), 0 );

	return {normal, texCoords, surfacePoint, tangent, bitangent};
}