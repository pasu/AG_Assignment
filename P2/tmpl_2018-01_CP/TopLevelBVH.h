#pragma once
#ifndef __TOP_LEVEL_BVH__
#define __TOP_LEVEL_BVH__

#include "RTObject.h"
#include "precomp.h"
#include "AABB.h"
#include "RayPacket.h"

class TopLevelBVHNode {
  public:
	AABB bounds;
	RTObject *object;// object!=nullptr if it's a leaf node. If object==nullptr, it's an inner node.
	int son;// index to the first son. second son = first son+1
};

// Top-Level-BVH is not promised to be balanced, can be very unbalanced
// But the number of nodes is promised to be 2n-1
// let nodes[0] = root
// then remove / add options can be done by swapping and decreasing the pointer.
class TopLevelBVH
{
  public:
	TopLevelBVH( const std::vector<RTObject *> &objects );
	~TopLevelBVH();

	void rebuild();
	bool getIntersection( const RTRay &ray, RTIntersection *intersection) const;
	bool getIntersection( const RayPacket &ray, RTIntersection *intersection ) const;

	int getFirstHit( const RayPacket &raypacket, const AABB &box, int ia ) const
	{
		float bbhits[2];
		const RTRay &ray = raypacket.m_ray[ia];
		if ( box.intersect( ray, bbhits, bbhits + 1 ) )
		{
			return ia;
		}

		if ( !raypacket.m_Frustum.Intersect( box ) )
		{
			return RAYPACKET_RAYS_PER_PACKET;
		}

		for ( unsigned int i = ia + 1; i < RAYPACKET_RAYS_PER_PACKET; ++i )
		{
			if ( box.intersect( raypacket.m_ray[i], bbhits, bbhits + 1 ) )
				return i;
		}

		return RAYPACKET_RAYS_PER_PACKET;
	}

	int getLastHit( const RayPacket &raypacket, const AABB &box, int ia ) const
	{
		for ( unsigned int ie = ( RAYPACKET_RAYS_PER_PACKET - 1 ); ie > ia; --ie )
		{
			float bbhits[2];

			if ( box.intersect( raypacket.m_ray[ie], bbhits, bbhits + 1 ) )
				return ie + 1;
		}

		return ia + 1;
	}
  private:
	TopLevelBVHNode *nodes;
	const std::vector<RTObject *> &objects;
	int tree_size;// the size of bvh tree: 2n-1
};

#endif