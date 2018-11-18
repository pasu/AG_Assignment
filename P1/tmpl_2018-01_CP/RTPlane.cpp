#include "precomp.h"

#include "RTPlane.h"
#include "Utils.h"

RTPlane::RTPlane(const vec3 &position, const vec3 &normal, const RTMaterial &material)
	: RTPrimitive( position, material ), normal( normalize( normal ) )
{
}

const RTIntersection RTPlane::intersect( const RTRay &ray ) const
{
	RTIntersection intersection( &ray, this, -1.0f );

	float denominator = dot( normal, ray.dir );

	//if denominator is almost 0 then Ray is pratically parallel to Plane
	if ( abs( denominator ) > Utils::EPSILON_FLOAT )
	{
		float num = dot( normal, ray.orig - pos );
		intersection.rayT = -num / denominator;
	}
	return intersection;
}

const SurfacePointData RTPlane::getSurfacePointData( const RTIntersection &intersection ) const
{
	const vec3 &surfacePoint = intersection.getIntersectionPosition();
	vec3 localCoords = surfacePoint - pos;
	vec2 texCoords = {dot( localCoords, tangent ), dot( localCoords, binormal )};
	return {normal, texCoords, surfacePoint, tangent, binormal};
}
