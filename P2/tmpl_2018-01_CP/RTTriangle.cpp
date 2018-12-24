#include "precomp.h"
#include "RTTriangle.h"

 RTTriangle::RTTriangle( vec3 v1, vec3 v2, vec3 v3, vec3 n1, vec3 n2, vec3 n3, vec2 t1, vec2 t2, vec2 t3, const RTMaterial &material )
	: RTPrimitive( material )
{
	vertices[0] = v1;
	vertices[1] = v2;
	vertices[2] = v3;
 
	normals[0] = n1;
	normals[1] = n2;
	normals[2] = n3;

	textures[0] = t1;
	textures[1] = t2;
	textures[2] = t3;

	computeAABBbounds();
}

RTTriangle::~RTTriangle()
{
}

void RTTriangle::computeAABBbounds()
{
	Vector3 min, max;
	min = max = vertices[0];

	min = ::min( min, vertices[1] );
	max = ::max( max, vertices[1] );

	min = ::min( min, vertices[2] );
	max = ::max( max, vertices[2] );

	box = AABB( min, max );

	pos = ( vertices[0] + vertices[1] + vertices[2] ) * (1.0/3.0);
}

const RTIntersection RTTriangle::intersect( const RTRay &ray ) const
{
	return intersectTriangle( ray, vertices[0], vertices[1], vertices[2] );
}

const RTIntersection RTTriangle::intersectTriangle( const RTRay &ray, const vec3 &a, const vec3 &b, const vec3 &c ) const
{
	RTIntersection intersection( &ray, this, -1 );

	vec3 AB = b - a;
	vec3 AC = c - a;

	vec3 P = cross( ray.dir, AC );
	float denominator = dot( P, AB ); //if negative triangle is backfacing (Cull here)

	if ( Utils::floatEquals( denominator, 0.0f ) ) //ray parallel to triangle
		return intersection;

	float inverseDenominator = 1.0f / denominator;
	vec3 T = ray.orig - a;
	float u = inverseDenominator * dot( P, T );
	if ( u < 0 || u > 1 )
		return intersection;

	vec3 Q = cross( T, AB );
	float v = inverseDenominator * dot( Q, ray.dir );
	if ( v < 0 || u + v > 1 )
		return intersection;

	intersection.rayT = inverseDenominator * dot( Q, AC );
	intersection.u = u;
	intersection.v = v;
	return intersection;
}

const SurfacePointData RTTriangle::getSurfacePointData( const RTIntersection &intersection ) const
{
	const vec3 &point = intersection.getIntersectionPosition();

	vec3 normal( normals[0].x + intersection.u * ( normals[1].x - normals[0].x ) + intersection.v * ( normals[2].x - normals[0].x ),
				 normals[0].y + intersection.u * ( normals[1].y - normals[0].y ) + intersection.v * ( normals[2].y - normals[0].y ),
				 normals[0].z + intersection.u * ( normals[1].z - normals[0].z ) + intersection.v * ( normals[2].z - normals[0].z ) );

	// The area of a triangle is
	float areaABC, areaPBC, areaPCA;
	Barycentric( point, vertices[0], vertices[1], vertices[2],
				 areaABC, areaPBC, areaPCA );

	vec2 texCoords = vec2( textures[0].x, textures[0].y ) * areaABC + vec2( textures[1].x, textures[1].y ) * areaPBC + vec2( textures[2].x, textures[2].y ) * areaPCA;

	return {normalize( normal ), texCoords, point};
}
