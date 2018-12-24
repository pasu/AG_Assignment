#pragma once
#include "RTMaterial.h"
#include "RTRay.h"
#include "RTSurfacePointData.h"

#include "AABB.h"

#define FLOAT_ZERO 0.000001f
#define MAX_DISTANCE_TO_INTERSECTON FLT_MAX

class RTIntersection;

class RTPrimitive
{
  public:
	RTPrimitive( const vec3 center, const RTMaterial &material );
	RTPrimitive( const RTMaterial &material );
	virtual const RTIntersection intersect( const RTRay &ray ) const = 0;
	virtual const SurfacePointData getSurfacePointData( const RTIntersection &intersection ) const = 0;

	virtual void computeAABBbounds() = 0;
// 	{
// 
// 	}

	void getBounds( Vector3& vmin, Vector3& vmax,Vector3 v1, Vector3 v2, Vector3 v3, Vector3 v4, 
		Vector3 v5, Vector3 v6, Vector3 v7, Vector3 v8);

	inline const RTMaterial &getMaterial() const { return material; }

	AABB getAABB() const { return box; }
	vec3 getCentroid() const 
	{
		Vector3 v = ( box.min + box.max ) * 0.5;
		return vec3( v.x, v.y, v.z); 
	};

  protected:
	vec3 pos;

	AABB box;
	const RTMaterial &material;
};
