#include "precomp.h"
#include "RTTorus.h"
#include "quarticsolver.h"

RTTorus::RTTorus( const vec3 &center, const vec3 &axis, float innerRadius, float outerRadius, const RTMaterial &material )
	: RTPrimitive( center, material ), mAxis( normalize( axis ) ), mInnerRadius( innerRadius ), mOuterRadius( outerRadius )
{
}

RTTorus::~RTTorus()
{
}

const RTIntersection RTTorus::intersect( const RTRay &ray )const
{
	RTIntersection nearestIntersection(&ray, this, -1.0f);

	vec3 rayOriginPosition = ray.orig;
	vec3 rayDirection = ray.dir;

	vec3 centerToRayOrigin = rayOriginPosition - pos;
	const float centerToRayOriginDotDirection = rayDirection.dot( centerToRayOrigin );
	float centerToRayOriginDotDirectionSquared = centerToRayOrigin.dot( centerToRayOrigin );
	float innerRadiusSquared = mInnerRadius * mInnerRadius;
	float outerRadiusSquared = mOuterRadius * mOuterRadius;

	float axisDotCenterToRayOrigin = mAxis.dot( centerToRayOrigin );
	float axisDotRayDirection = mAxis.dot( rayDirection );
	float a = 1 - axisDotRayDirection * axisDotRayDirection;
	float b = 2 * ( centerToRayOrigin.dot( rayDirection ) - axisDotCenterToRayOrigin * axisDotRayDirection );
	float c = centerToRayOriginDotDirectionSquared - axisDotCenterToRayOrigin * axisDotCenterToRayOrigin;
	float d = centerToRayOriginDotDirectionSquared + outerRadiusSquared - innerRadiusSquared;

	// Solve quartic equation with coefficients A, B, C, D and E
	float A = 1;
	float B = 4 * centerToRayOriginDotDirection;
	float C = 2 * d + B * B * 0.25f - 4 * outerRadiusSquared * a;
	float D = B * d - 4 * outerRadiusSquared * b;
	float E = d * d - 4 * outerRadiusSquared * c;

	// Maximum number of roots is 4
	QuarticEquation equation( A, B, C, D, E );
	const int maxRootsCount = 4;
	double roots[maxRootsCount] = {-1.0, -1.0, -1.0, -1.0};
	int rootsCount = equation.Solve( roots );

	if ( rootsCount == 0 )
	{
		return nearestIntersection;
	}

	// Find closest to zero positive solution
	float closestRoot = MAX_DISTANCE_TO_INTERSECTON;
	std::vector<float> intersectionDistances;
	for ( int idx = 0; idx < maxRootsCount; ++idx )
	{
		float root = roots[idx];
		if ( root > FLOAT_ZERO && root < closestRoot )
		{
			closestRoot = root;
			intersectionDistances.push_back( root );
		}
	}

	if ( closestRoot != MAX_DISTANCE_TO_INTERSECTON )
	{
		nearestIntersection.rayT = closestRoot;
	}

	return nearestIntersection;
}

const SurfacePointData RTTorus::getSurfacePointData( const RTIntersection &intersection )const
{
// 	const vec3 &point = intersection.getIntersectionPosition();
// 	vec3 centerToPoint = point - pos;
// 	float centerToPointDotAxis = centerToPoint.dot( mAxis );
// 	vec3 direction = centerToPoint - mAxis * centerToPointDotAxis;
// 	direction.normalize();
// 	vec3 normal = point - pos + direction * mOuterRadius;

	const vec3 &point = intersection.getIntersectionPosition();
	vec3 centerToPoint = point - pos;
	float k = centerToPoint.dot( mAxis );
	vec3 A = point - mAxis * k;
	float m = sqrtf( mInnerRadius * mInnerRadius - k * k );
	vec3 N = point - A - ( pos - A ) * ( m / ( mOuterRadius + m ) );

	return {normalize( N ), {0, 0}, point};
}
