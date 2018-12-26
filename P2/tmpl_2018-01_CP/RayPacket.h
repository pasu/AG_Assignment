#pragma once
#include "precomp.h"
#include "RTFrustum.h"
#include "RTRay.h"

class RayPacket
{
  public:
	RayPacket( const RayTracer &raytracer, const vec2 &pos );
	~RayPacket();

  public:
	RTFrustum m_Frustum;
	RTRay m_ray[RAYPACKET_RAYS_PER_PACKET];

	const RayTracer &raytracer;
};
