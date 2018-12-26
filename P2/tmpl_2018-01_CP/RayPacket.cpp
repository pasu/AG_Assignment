#include "precomp.h"
#include "RayPacket.h"


RayPacket::RayPacket( const RayTracer &tracer, const vec2 &pos )
	: raytracer(tracer)
{
	unsigned int i = 0;

	for ( unsigned int y = 0; y < RAYPACKET_DIM; ++y )
	{
		for ( unsigned int x = 0; x < RAYPACKET_DIM; ++x )
		{
			m_ray[i++] = raytracer.generatePrimaryRay( pos.x + x, pos.y + y );
		}
	}

	m_Frustum.GenerateFrustum(
		m_ray[0 * RAYPACKET_DIM + 0],
		m_ray[0 * RAYPACKET_DIM + ( RAYPACKET_DIM - 1 )],
		m_ray[( RAYPACKET_DIM - 1 ) * RAYPACKET_DIM + 0],
		m_ray[( RAYPACKET_DIM - 1 ) * RAYPACKET_DIM + ( RAYPACKET_DIM - 1 )] );
}

RayPacket::~RayPacket()
{
}
