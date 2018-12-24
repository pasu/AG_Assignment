#pragma once
#include "RTPrimitive.h"
class RTTriangle : public RTPrimitive
{
  public:
	RTTriangle( vec3 v1, vec3 v2, vec3 v3, vec3 n1, vec3 n2, vec3 n3, vec2 t1, vec2 t2, vec2 t3, const RTMaterial &material );
	~RTTriangle();

	void computeAABBbounds();

	const RTIntersection intersect( const RTRay &ray ) const;
	const SurfacePointData getSurfacePointData( const RTIntersection &intersection ) const;

  private:
	const RTIntersection intersectTriangle( const RTRay &ray, const vec3 &a, const vec3 &b, const vec3 &c ) const;

	void Barycentric( const vec3 &p, const vec3 &a, const vec3 &b, const vec3 &c, float &u, float &v, float &w ) const
	{
		vec3 v0 = b - a, v1 = c - a, v2 = p - a;
		float d00 = dot( v0, v0 );
		float d01 = dot( v0, v1 );
		float d11 = dot( v1, v1 );
		float d20 = dot( v2, v0 );
		float d21 = dot( v2, v1 );
		float denom = d00 * d11 - d01 * d01;
		v = ( d11 * d20 - d01 * d21 ) / denom;
		w = ( d00 * d21 - d01 * d20 ) / denom;
		u = 1.0f - v - w;
	}
  private:

		vec3 vertices[3];
		vec3 normals[3];
		vec2 textures[3];
};
