#pragma once
#include "RTRay.h"
#include "AABB.h"
class RTFrustum
{
  public:
	RTFrustum();
	~RTFrustum();

	void GenerateFrustum( const RTRay &topLeft,
						  const RTRay &topRight,
						  const RTRay &bottomLeft,
						  const RTRay &bottomRight );

	bool Intersect( const AABB &box ) const;

  private:
	vec3 m_normals[4];
	vec3 m_points[4];
};
