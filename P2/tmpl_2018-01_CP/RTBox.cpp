#include "precomp.h"
#include "RTBox.h"

 RTBox::RTBox( const vec3 &center, const vec3 &dimensions, const RTMaterial &material )
	:min( vec3( center.x - dimensions.x / 2.0f, center.y - dimensions.y / 2.0f, center.z - dimensions.z / 2.0f ) ),
	max( vec3( center.x + dimensions.x / 2.0f, center.y + dimensions.y / 2.0f, center.z + dimensions.z / 2.0f ) ),
	RTPrimitive( center, material )
{
	 computeAABBbounds();
 }

 RTBox::RTBox( const vec3 &center, const vec3 &min, const vec3 max, const RTMaterial &material )
	: min( min ), max( max ), RTPrimitive( center, material )
{
	 computeAABBbounds();
 }

RTBox::~RTBox()
{
}

const RTIntersection RTBox::intersect( const RTRay &ray ) const
{
	RTIntersection intersection( &ray, this, -1 );

	float tmin = ( min.x - ray.orig.x ) / ray.dir.x;
	float tmax = ( max.x - ray.orig.x ) / ray.dir.x;

	if ( tmin > tmax )
		swap( tmin, tmax );

	float tymin = ( min.y - ray.orig.y ) / ray.dir.y;
	float tymax = ( max.y - ray.orig.y ) / ray.dir.y;

	if ( tymin > tymax )
		swap( tymin, tymax );

	if ( ( tmin > tymax ) || ( tymin > tmax ) )
		return intersection;

	if ( tymin > tmin )
		tmin = tymin;

	if ( tymax < tmax )
		tmax = tymax;

	float tzmin = ( min.z - ray.orig.z ) / ray.dir.z;
	float tzmax = ( max.z - ray.orig.z ) / ray.dir.z;

	if ( tzmin > tzmax )
		swap( tzmin, tzmax );

	if ( ( tmin > tzmax ) || ( tzmin > tmax ) )
		return intersection;

	if ( tzmin > tmin )
		tmin = tzmin;

	if ( tzmax < tmax )
		tmax = tzmax;

	intersection.rayT = tmin > 0 ? tmin : tmax;
	return intersection;
}

const SurfacePointData RTBox::getSurfacePointData( const RTIntersection &intersection ) const
{
	SurfacePointData data;
	const vec3 &surfacePoint = intersection.getIntersectionPosition();

	vec2 texCoords;
	const float epsilon = 0.001f;

	float distanceX;
	float distanceY;
	float distanceZ;

	distanceX = max.x - min.x;
	distanceY = max.y - min.y;
	distanceZ = max.z - min.z;

	if ( Utils::floatEquals( surfacePoint.x, min.x, epsilon ) )
	{
		data.normal = {-1, 0, 0};
		data.tangent = {0, 0, 1};
		data.bitangent = {0, 1, 0};

		texCoords.x = ( surfacePoint.z - min.z ) / distanceZ;
		texCoords.y = ( surfacePoint.y - min.y ) / distanceY;
	}
	else if ( Utils::floatEquals( surfacePoint.x, max.x, epsilon ) )
	{
		data.normal = {1, 0, 0};
		data.tangent = {0, 0, 1};
		data.bitangent = {0, 1, 0};

		texCoords.x = ( surfacePoint.z - min.z ) / distanceZ;
		texCoords.y = ( surfacePoint.y - min.y ) / distanceY;
	}
	else if ( Utils::floatEquals( surfacePoint.y, min.y, epsilon ) )
	{
		data.normal = {0, -1, 0};
		data.tangent = {1, 0, 0};
		data.bitangent = {0, 0, 1};

		texCoords.x = ( surfacePoint.x - min.x ) / distanceX;
		texCoords.y = ( surfacePoint.z - min.z ) / distanceZ;
	}
	else if ( Utils::floatEquals( surfacePoint.y, max.y, epsilon ) )
	{
		data.normal = {0, 1, 0};
		data.tangent = {1, 0, 1};
		data.bitangent = {0, 0, 1};

		texCoords.x = ( surfacePoint.x - min.x ) / distanceX;
		texCoords.y = ( surfacePoint.z - min.z ) / distanceZ;
	}
	else if ( Utils::floatEquals( surfacePoint.z, min.z, epsilon ) )
	{
		data.normal = {0, 0, -1};
		data.tangent = {1, 0, 0};
		data.bitangent = {0, 1, 0};

		texCoords.x = ( surfacePoint.x - min.x ) / distanceX;
		texCoords.y = ( surfacePoint.y - min.y ) / distanceY;
	}
	else if ( Utils::floatEquals( surfacePoint.z, max.z, epsilon ) )
	{
		data.normal = {0, 0, 1};
		data.tangent = {1, 0, 0};
		data.bitangent = {0, 1, 0};

		texCoords.x = ( surfacePoint.x - min.x ) / distanceX;
		texCoords.y = ( surfacePoint.y - min.y ) / distanceY;
	}

	data.position = surfacePoint;
	data.textureCoordinates = texCoords;
	data.position = surfacePoint;
	return data;
}

void RTBox::computeAABBbounds()
{
	box = AABB(min,max);
}
