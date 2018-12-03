#include "precomp.h"
#include "RTCone.h"

 RTCone::RTCone( const vec3 &top, const vec3 &bottomCenter, const float &radius, const RTMaterial &material )
	: RTPrimitive( bottomCenter, material ), top( top ), radius(radius)
{
	dir = pos - top;

	binormal = 1.0f;
	if ( fabs( dir.x ) > FLOAT_ZERO )
	{
		binormal.x = 0;
	}
	else if ( fabs( dir.y ) > FLOAT_ZERO )
	{
		binormal.y = 0;
	}
	else if ( fabs( dir.z ) > FLOAT_ZERO )
	{
		binormal.z = 0;
	}

	tangent = normalize( cross( dir, binormal ) );
	binormal = normalize( cross( dir, tangent ) );
}

RTCone::~RTCone()
{
}

const RTIntersection RTCone::intersect( const RTRay &ray ) const
{
	RTIntersection nearestIntersection( &ray, this, -1.0f );

	vec3 coneAxis = ( pos - top );
	coneAxis.normalize();

	vec3 rayOriginPosition = ray.orig;
	vec3 rayDirection = ray.dir;

	vec3 bottomCenterToRayOrigin = rayOriginPosition - top;

	float rayDirectionDotAxis = rayDirection.dot( coneAxis );
	float bottomCenterToRayOriginDotAxis = bottomCenterToRayOrigin.dot( coneAxis );
	float radiansPerHeight = radius / ( pos - top ).length();

	vec3 u = rayDirection + coneAxis * ( -rayDirectionDotAxis );
	vec3 v = bottomCenterToRayOrigin + coneAxis * ( -bottomCenterToRayOriginDotAxis );
	float w = bottomCenterToRayOriginDotAxis * radiansPerHeight;

	float radiansPerDirection = rayDirectionDotAxis * radiansPerHeight;

	// Solve square equation a * x^2 + b * x + c = 0
	float a = u.dot( u ) - radiansPerDirection * radiansPerDirection;
	float closestRoot = -1.f;
	float rayExit = -1.f;
	float root = 0.f;
	std::vector<float> intersectionDistances;

	if ( fabs( a ) > FLOAT_ZERO )
	{
		float b = 2 * ( u.dot( v ) - w * radiansPerDirection );
		float c = v.dot( v ) - w * w;
		float discriminant = b * b - 4 * a * c;

		if ( discriminant < 0.0 )
		{
			return nearestIntersection;
		}

		discriminant = sqrtf( discriminant );
		float denominator = 1.0f / ( 2.0f * a );

		root = ( -b - discriminant ) * denominator;
		if ( root > 0.0 )
		{
			vec3 point = ray.getPointAt( root );
			vec3 bottomCenterToPoint = point - top;
			vec3 topToPoint = point - pos;
			if ( coneAxis.dot( bottomCenterToPoint ) > 0.0 && ( -coneAxis ).dot( topToPoint ) > 0.0 )
			{
				intersectionDistances.push_back( root );
				closestRoot = root;
			}
		}

		root = ( -b + discriminant ) * denominator;
		if ( root > 0.0 )
		{
			vec3 point = ray.getPointAt( root );
			vec3 bottomCenterToPoint = point - top;
			vec3 topToPoint = point - pos;
			if ( coneAxis.dot( bottomCenterToPoint ) > 0.0 && ( -coneAxis ).dot( topToPoint ) > 0.0 )
			{
				intersectionDistances.push_back( root );
				if ( closestRoot < 0.0 )
				{
					closestRoot = root;
				}
				else if ( root < closestRoot )
				{
					rayExit = closestRoot;
					closestRoot = root;
				}
				else
				{
					rayExit = root;
				}
			}
		}
	}

	// Intersection with bottom
	if ( fabs( rayDirectionDotAxis ) < FLOAT_ZERO )
	{
		if ( closestRoot > 0.0 )
		{
			if ( rayExit < 0.f )
			{
				intersectionDistances.insert( intersectionDistances.begin(), 0.f );
			}
			nearestIntersection.rayT = closestRoot;
		}

		return nearestIntersection;
	}

	// Intersection with top and bottom points
	root = ( -coneAxis ).dot( rayOriginPosition - pos ) / rayDirectionDotAxis;
	if ( root > 0.0 )
	{
		vec3 topToPoint = ray.getPointAt( root ) - pos;
		if ( topToPoint.dot( topToPoint ) < radius * radius )
		{
			intersectionDistances.push_back( root );
			if ( closestRoot < 0.0 )
			{
				closestRoot = root;
				rayExit = root;
			}
			else if ( root < closestRoot )
			{
				rayExit = closestRoot;
				closestRoot = root;
			}
			else
			{
				rayExit = root;
			}
		}
	}

	if ( closestRoot > 0.0 )
	{
		if ( rayExit < 0.0 )
		{
			intersectionDistances.insert( intersectionDistances.begin(), 0.f );
		}
		
		nearestIntersection.rayT = closestRoot;
	}

	return nearestIntersection;
}

const SurfacePointData RTCone::getSurfacePointData( const RTIntersection &intersection ) const
{
	vec3 coneAxis = ( pos - top );
	coneAxis.normalize();

	const vec3 &point = intersection.getIntersectionPosition();

	// If point is lying at bottom
	vec3 pointPositionRelativelyToBottom = point - pos;
	if ( fabs( coneAxis.dot( pointPositionRelativelyToBottom ) ) < FLOAT_ZERO &&
		 pointPositionRelativelyToBottom.dot( pointPositionRelativelyToBottom ) < radius * radius )
	{
		vec2 texCoords = {dot( pointPositionRelativelyToBottom, tangent ), dot( pointPositionRelativelyToBottom, binormal )};

		return {coneAxis, texCoords, point};
	}

	// Otherwise, if point is lying at side surface
	vec3 approximatedNormal = point - ( coneAxis * ( point - pos ).dot( coneAxis ) + pos );
	float radiansPerHeight = radius / ( pos - top ).length();
	vec3 normal = approximatedNormal + coneAxis * ( -radiansPerHeight * approximatedNormal.length() );
	normal.normalize();

	float v = pointPositionRelativelyToBottom.dot( coneAxis );
	vec3 A = normalize( pointPositionRelativelyToBottom - v * coneAxis );
	float value = std::max( -1.0f, std::min( 1.0f, dot( A, tangent ) ) );

	float phi = acos( value );

	vec2 texCoords = {phi * 2 * Utils::RT_PI, v};

	return {normal, texCoords, point};
}
