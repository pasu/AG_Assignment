#include "precomp.h"
#include "RTTorus.h"
#include "quarticsolver.h"

RTTorus::RTTorus( const vec3 &center, const vec3 &axis, float innerRadius, float outerRadius, const RTMaterial &material )
	: RTPrimitive( center, material ), mAxis( normalize( axis ) ), mInnerRadius( innerRadius ), mOuterRadius( outerRadius )
{
	computeAABBbounds();
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

	float theta = atan2( A.x, A.z );
	float theta2 = atan2( sqrtf( A.x * A.x + A.z * A.z ) - mOuterRadius, A.y );

	vec2 texCoords = {0.5f + theta / ( 2.0f * Utils::RT_PI ), 0.5f + theta2 / ( 2.0f * Utils::RT_PI )};

	return {normalize( N ), texCoords , point};
}

void RTTorus::computeAABBbounds()
{
	vec3 binormal = 1.0f;
	if ( fabs( mAxis.x ) > FLOAT_ZERO )
	{
		binormal.x = 0;
	}
	else if ( fabs( mAxis.y ) > FLOAT_ZERO )
	{
		binormal.y = 0;
	}
	else if ( fabs( mAxis.z ) > FLOAT_ZERO )
	{
		binormal.z = 0;
	}

	vec3 tangent = normalize( cross( mAxis, binormal ) );
	binormal = normalize( cross( mAxis, tangent ) );

	float radius = mInnerRadius + mOuterRadius;
	vec3 x_dir = tangent * radius;
	vec3 y_dir = binormal * radius;

	vec3 bottomcorner[8];

	vec3 bottom = pos - mAxis*mInnerRadius;
	bottomcorner[0] = bottom + x_dir + y_dir;
	bottomcorner[1] = bottom + x_dir - y_dir;
	bottomcorner[2] = bottom - x_dir + y_dir;
	bottomcorner[3] = bottom - x_dir - y_dir;

	vec3 top = pos + mAxis * mInnerRadius;
	bottomcorner[4] = top + x_dir + y_dir;
	bottomcorner[5] = top + x_dir - y_dir;
	bottomcorner[6] = top - x_dir + y_dir;
	bottomcorner[7] = top - x_dir - y_dir;

	Vector3 min, max;
	getBounds( min, max,
			   bottomcorner[0], bottomcorner[1], bottomcorner[2], bottomcorner[3],
			   bottomcorner[4], bottomcorner[5], bottomcorner[6], bottomcorner[7] );
	box = AABB( min, max );
}
