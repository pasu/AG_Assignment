#include "precomp.h"

#include "RTPlane.h"
#include "Utils.h"

RTPlane::RTPlane( const vec3 &position, const vec3 &normal, const vec3 &tangent, const RTMaterial &material )
	: RTPrimitive( position, material ), normal( normalize( normal ) ), tangent( normalize( tangent ) ), binormal( normalize( cross( normal, tangent ) ) )
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
// 		if (num < 0 && denominator < 0) {// anti - seam
// 			intersection.rayT = 0;
// 		}
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
