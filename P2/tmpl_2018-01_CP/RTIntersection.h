#pragma once
#include "RTPrimitive.h"
#include "RTRay.h"
class RTIntersection
{
  public:
	RTIntersection();
	RTIntersection( const RTRay *ray, const RTPrimitive *object, const float rayT );

  public:
	const RTRay *ray;
	const RTPrimitive *object;
	float rayT;

	SurfacePointData surfacePointData;
	const RTMaterial *material;

	//Triangle Only
	int triangleIndex;
	float u, v; //barycentric coords

	inline bool isIntersecting() const { return rayT > 0.0f; }
	inline bool isInSideObj() const
	{
		bool bResult = false;
		if ( isIntersecting() )
		{
			bResult = dot( ray->dir, surfacePointData.normal ) > 0;
		}

		return bResult;
	}
	inline const vec3 getIntersectionPosition() const { return ray->orig + ray->dir * rayT; }
};
