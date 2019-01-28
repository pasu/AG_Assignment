#include "BVH.h"
#include "precomp.h"
#include "Utils.h"
#include "RayPacket.h"

BVH::BVH( std::vector<RTPrimitive *> *objects, uint32_t leafSize /*= 4 */ )
	: build_prims( objects ), leafSize( leafSize ), nNodes( 0 ), nLeafs( 0 ), bvhTree( NULL )
{
	build();

	// Initialize the indexes of ray packet for partition traversal
	for ( unsigned int i = 0; i < RAYPACKET_RAYS_PER_PACKET; ++i )
	{
		m_I[i] = i;
	}
}

BVH::~BVH()
{
	delete[] bvhTree;
}

struct BVHTraversal
{
	uint32_t i; // Node
	float mint; // Minimum hit time for this node.
	BVHTraversal() {}
	BVHTraversal( int _i, float _mint ) : i( _i ), mint( _mint ) {}
};

//! - Compute the nearest intersection of all objects within the tree.
//! - Return true if hit was found, false otherwise.
//! - In the case where we want to find out of there is _ANY_ intersection at all,
//!   set occlusion == true, in which case we exit on the first hit, rather
//!   than find the closest.
bool BVH::getIntersection( const RTRay &ray, RTIntersection *intersection, bool occlusion, const float &distance ) const
{
	//intersection->rayT = 999999999.f;
	intersection->object = NULL;
	float bbhits[4];
	int32_t closer, other;

	// Working set
	BVHTraversal todo[6400];
	int32_t stackptr = 0;

	// "Push" on the root node to the working set
	todo[stackptr].i = 0;
	todo[stackptr].mint = -9999999.f;

	while ( stackptr >= 0 )
	{
		// Pop off the next node to work on.
		int ni = todo[stackptr].i;
		float fnear = todo[stackptr].mint;
		stackptr--;
		const BVHNode_32 &node( bvhTree[ni] );

		// If this node is further than the closest found intersection, continue
		if ( intersection->rayT > 0.0 && fnear > intersection->rayT )
			continue;

		// Is leaf -> Intersect
		if ( ( node.leftFirst & 0x1 ) == 0 )
		{
			uint32_t start = ( node.leftFirst >> 1 );
			for ( uint32_t o = 0; o < node.count; ++o )
			{
				bool hit = false;

				const RTPrimitive *obj = ( *build_prims )[start + o];
				const RTIntersection &current = obj->intersect( ray );

				if ( current.isIntersecting() )
				{
					// If we're only looking for occlusion, then any hit is good enough
					if ( occlusion && current.rayT<distance )
					{
						*intersection = current;
						return true;
					}

					// Otherwise, keep the closest intersection only
					if ( intersection->rayT < 0.0f || current.rayT < intersection->rayT )
					{
						*intersection = current;
					}
				}
			}
		}
		else
		{ // Not a leaf

			uint32_t rightOffset = ( node.leftFirst >> 1 );
			bool hitc0 = bvhTree[ni + 1].bounds.intersect( ray, bbhits, bbhits + 1 );
			bool hitc1 = bvhTree[ni + rightOffset].bounds.intersect( ray, bbhits + 2, bbhits + 3 );

			// Did we hit both nodes?
			if ( hitc0 && hitc1 )
			{

				// We assume that the left child is a closer hit...
				closer = ni + 1;
				other = ni + rightOffset;

				// ... If the right child was actually closer, swap the relavent values.
				if ( bbhits[2] < bbhits[0] )
				{
					std::swap( bbhits[0], bbhits[2] );
					std::swap( bbhits[1], bbhits[3] );
					std::swap( closer, other );
				}

				// It's possible that the nearest object is still in the other side, but we'll
				// check the further-awar node later...

				// Push the farther first
				todo[++stackptr] = BVHTraversal( other, bbhits[2] );

				// And now the closer (with overlap test)
				todo[++stackptr] = BVHTraversal( closer, bbhits[0] );
			}

			else if ( hitc0 )
			{
				todo[++stackptr] = BVHTraversal( ni + 1, bbhits[0] );
			}

			else if ( hitc1 )
			{
				todo[++stackptr] = BVHTraversal( ni + rightOffset, bbhits[2] );
			}
		}
	}

	// If we hit something,
	// 	if ( intersection->object != NULL )
	// 		intersection->hit = ray.o + ray.d * intersection->t;
	if ( intersection->object == nullptr )
	{
		intersection->rayT = -1;
	}
	return intersection->object != NULL;
}

struct StackNode
{
	int cell;
	unsigned int ia; // Index to the first alive ray
};

int BVH::getFirstHit( const RayPacket &raypacket, const AABB &box, int ia )const
{
	float bbhits[2];
	const RTRay &ray = raypacket.m_ray[ia];
	if ( box.intersect( ray, bbhits, bbhits +1) )
	{
		return ia;
	}

	if (!raypacket.m_Frustum.Intersect(box))
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

#ifdef BVH_RANGED_TRAVERSAL
int BVH::getLastHit( const RayPacket &raypacket, const AABB &box, int ia )const
{
	for ( unsigned int ie = ( RAYPACKET_RAYS_PER_PACKET - 1 ); ie > ia; --ie )
	{
		float bbhits[2];

		if ( box.intersect( raypacket.m_ray[ie], bbhits, bbhits + 1 ) )
				return ie + 1;
	}

	return ia + 1;
}

bool BVH::getIntersection( const RayPacket &raypacket, RTIntersection *intersections )
{
	if (false)
	{
		unsigned int ia = 0;
		ia = getFirstHit( raypacket, bvhTree[0].bounds, ia );
		if ( ia < RAYPACKET_RAYS_PER_PACKET )
		{
			const unsigned int ie = getLastHit( raypacket,
												bvhTree[0].bounds,
												ia );

			for ( unsigned int i = ia; i < ie; ++i )
			{
				getIntersection( raypacket.m_ray[i], &intersections[i], false );
			}
		}

		return true;
	}
	else if (false)
	{
		unsigned int ia = 0;
		ia = getFirstHit( raypacket, bvhTree[0].bounds, ia );
		if ( ia < RAYPACKET_RAYS_PER_PACKET )
		{
			const unsigned int ie = getLastHit( raypacket,
												bvhTree[0].bounds,
												ia );

			for ( unsigned int i = ia; i < ie; ++i )
			{
				StackNode todo[64];

				bool anyHitted = false;
				int todoOffset = 0, nodeNum = 0;
				unsigned int ia = 0;

				while ( true )
				{
					const BVHNode_32 *curCell = &bvhTree[nodeNum];
					
					if ( ( curCell->leftFirst & 0x1 ) == 1 )
					{
						StackNode &node = todo[todoOffset++];

						uint32_t rightOffset = ( curCell->leftFirst >> 1 );
						node.cell = nodeNum+rightOffset;
						node.ia = ia;
						nodeNum = nodeNum + 1;
						continue;
					}
					else
					{
						uint32_t start = ( curCell->leftFirst >> 1 );
						for ( uint32_t o = 0; o < curCell->count; ++o )
						{
							const RTPrimitive *obj = ( *build_prims )[start + o];

							const RTIntersection &current = obj->intersect( raypacket.m_ray[i] );

							if ( current.isIntersecting() )
							{
								anyHitted |= true;

								if ( intersections[i].rayT < 0.0f || current.rayT < intersections[i].rayT )
								{
									( intersections[i] ) = current;
								}
							}
						}
					}

					if ( todoOffset == 0 )
						break;

					const StackNode &node = todo[--todoOffset];

					nodeNum = node.cell;
					ia = node.ia;
				}
			}
		}

		return true;
	}
	else
	{
		StackNode todo[64];

		bool anyHitted = false;
		int todoOffset = 0, nodeNum = 0;
		unsigned int ia = 0;

		while ( true )
		{
			const BVHNode_32 *curCell = &bvhTree[nodeNum];
			ia = getFirstHit( raypacket, curCell->bounds, ia );

			if ( ia < RAYPACKET_RAYS_PER_PACKET )
			{
				// nodes
				if ( ( curCell->leftFirst & 0x1 ) == 1 )
				{
					StackNode &node = todo[todoOffset++];

					uint32_t rightOffset = ( curCell->leftFirst >> 1 );
					node.cell = nodeNum + rightOffset;
					node.ia = ia;
					nodeNum = nodeNum + 1;
					continue;
				}
				else
				{
					const unsigned int ie = getLastHit( raypacket,
														curCell->bounds,
														ia );

					uint32_t start = ( curCell->leftFirst >> 1 );
					for ( uint32_t o = 0; o < curCell->count; ++o )
					{
						const RTPrimitive *obj = ( *build_prims )[start + o];

						if ( raypacket.m_Frustum.Intersect( obj->getAABB() ) )
						{
							for ( unsigned int i = ia; i < ie; ++i )
							{
								const RTIntersection &current = obj->intersect( raypacket.m_ray[i] );

								if ( current.isIntersecting() )
								{
									anyHitted |= true;

									if ( intersections[i].rayT < 0.0f || current.rayT < intersections[i].rayT )
									{
										( intersections[i] ) = current;
									}
								}
							}
						}
					}
				}
			}

			if ( todoOffset == 0 )
				break;

			const StackNode &node = todo[--todoOffset];

			nodeNum = node.cell;
			ia = node.ia;
		}

		return anyHitted;
	}
	
}
#endif

#ifdef BVH_PARTITION_TRAVERSAL
int BVH::getLastHit( const RayPacket &raypacket, const AABB &box, int ia, const unsigned int *aRayIndex) const
{
	for ( unsigned int ie = ( RAYPACKET_RAYS_PER_PACKET - 1 ); ie > ia; --ie )
	{
		float bbhits[2];

		if ( box.intersect( raypacket.m_ray[aRayIndex[ie]], bbhits, bbhits+1 ) )
				return ie + 1;
	}

	return ia + 1;
}

int BVH::partRays( const RayPacket &raypacket, const AABB &box, int ia, unsigned int *aRayIndex ) const
{
	if ( !raypacket.m_Frustum.Intersect( box ) )
		return RAYPACKET_RAYS_PER_PACKET;

	unsigned int ie = 0;

	for ( unsigned int i = 0; i < ia; ++i )
	{
		float bbhits[2];
		
		if ( box.intersect( raypacket.m_ray[aRayIndex[i]], bbhits, bbhits+1 ) )
				std::swap( aRayIndex[ie++], aRayIndex[i] );
	}

	return ie;
}

bool BVH::getIntersection( const RayPacket &raypacket, RTIntersection *intersections )
{
		StackNode todo[64];

		bool anyHitted = false;
		int todoOffset = 0, nodeNum = 0;
		unsigned int ia = 0;

		unsigned int I[RAYPACKET_RAYS_PER_PACKET];
		memcpy( I, m_I, RAYPACKET_RAYS_PER_PACKET * sizeof( unsigned int ) );

		while ( true )
		{
			const BVHNode_32 *curCell = &bvhTree[nodeNum];
			ia = partRays( raypacket, curCell->bounds, ia, I );

			if ( ia < RAYPACKET_RAYS_PER_PACKET )
			{
				// nodes
				if ( ( curCell->leftFirst & 0x1 ) == 1 )
				{
					StackNode &node = todo[todoOffset++];

					uint32_t rightOffset = ( curCell->leftFirst >> 1 );
					node.cell = nodeNum + rightOffset;
					node.ia = ia;
					nodeNum = nodeNum + 1;
					continue;
				}
				else
				{
					const unsigned int ie = getLastHit( raypacket,
														curCell->bounds,
														ia,
														I);

					uint32_t start = ( curCell->leftFirst >> 1 );
					for ( uint32_t o = 0; o < curCell->count; ++o )
					{
						const RTPrimitive *obj = ( *build_prims )[start + o];

						if ( raypacket.m_Frustum.Intersect( obj->getAABB() ) )
						{
							for ( unsigned int i = ia; i < ie; ++i )
							{
								const RTIntersection &current = obj->intersect( raypacket.m_ray[i] );

								if ( current.isIntersecting() )
								{
									anyHitted |= true;

									if ( intersections[i].rayT < 0.0f || current.rayT < intersections[i].rayT )
									{
										( intersections[i] ) = current;
									}
								}
							}
						}
					}
				}
			}

			if ( todoOffset == 0 )
				break;

			const StackNode &node = todo[--todoOffset];

			nodeNum = node.cell;
			ia = node.ia;
		}

		return anyHitted;
}
#endif // BVH_PARTITION_TRAVERSAL

struct BVHBuildEntry
{
	// If non-zero then this is the index of the parent. (used in offsets)
	uint32_t parent;
	// The range of objects in the object list covered by this node.
	uint32_t start, end;
};

void BVH::build()
{
	BVHBuildEntry todo[128];
	uint32_t stackptr = 0;
	const uint32_t Untouched = 0xffffffff;
	const uint32_t TouchedTwice = 0xfffffffd;

	// Push the root
	todo[stackptr].start = 0;
	todo[stackptr].end = build_prims->size();
	todo[stackptr].parent = 0xfffffffc;
	stackptr++;

	BVHNode node;
	std::vector<BVHNode> buildnodes;
	buildnodes.reserve( build_prims->size() * 2 );

	while ( stackptr > 0 )
	{
		// Pop the next item off of the stack
		BVHBuildEntry &bnode( todo[--stackptr] );
		uint32_t start = bnode.start;
		uint32_t end = bnode.end;
		uint32_t nPrims = end - start;

		nNodes++;
		node.start = start;
		node.nPrims = nPrims;
		node.rightOffset = Untouched;

		// Calculate the bounding box for this node
		AABB bb( ( *build_prims )[start]->getAABB() );
		AABB bc( ( *build_prims )[start]->getCentroid() );
		for ( uint32_t p = start + 1; p < end; ++p )
		{
			bb.expandToInclude( ( *build_prims )[p]->getAABB() );
			bc.expandToInclude( ( *build_prims )[p]->getCentroid() );
		}
		node.bounds = bb;

		// If the number of primitives at this point is less than the leaf
		// size, then this will become a leaf. (Signified by rightOffset == 0)
		if ( nPrims <= leafSize )
		{
			node.rightOffset = 0;
			nLeafs++;
		}

		buildnodes.push_back( node );

		// Child touches parent...
		// Special case: Don't do this for the root.
		if ( bnode.parent != 0xfffffffc )
		{
			buildnodes[bnode.parent].rightOffset--;

			// When this is the second touch, this is the right child.
			// The right child sets up the offset for the flat tree.
			if ( buildnodes[bnode.parent].rightOffset == TouchedTwice )
			{
				buildnodes[bnode.parent].rightOffset = nNodes - 1 - bnode.parent;
			}
		}

		// If this is a leaf, no need to subdivide.
		if ( node.rightOffset == 0 )
			continue;

		// Set the split dimensions
		uint32_t split_dim = bc.maxDimension();
		// Split on the center of the longest axis
		float split_coord = .5f * ( bc.min[split_dim] + bc.max[split_dim] );

#ifdef SAH_ON
		getSplitDimAndCoordBySAH( split_dim, split_coord, BIN_NUM, bc, start, end );
#endif

		// Partition the list of objects on this split
		uint32_t mid = start;
		for ( uint32_t i = start; i < end; ++i )
		{
			if ( ( *build_prims )[i]->getCentroid()[split_dim] < split_coord )
			{
				std::swap( ( *build_prims )[i], ( *build_prims )[mid] );
				++mid;
			}
		}

		// If we get a bad split, just choose the center...
		if ( mid == start || mid == end )
		{
			mid = start + ( end - start ) / 2;
		}

		// Push right child
		todo[stackptr].start = mid;
		todo[stackptr].end = end;
		todo[stackptr].parent = nNodes - 1;
		stackptr++;

		// Push left child
		todo[stackptr].start = start;
		todo[stackptr].end = mid;
		todo[stackptr].parent = nNodes - 1;
		stackptr++;
	}

	// Copy the temp node data to a flat array
	bvhTree = new BVHNode_32[nNodes];
	for ( uint32_t n = 0; n < nNodes; ++n )
	{
		const BVHNode &node = buildnodes[n];

		bvhTree[n].bounds = node.bounds;
		bvhTree[n].count = node.nPrims;
		// leaf
		if (node.rightOffset == 0)
		{
			bvhTree[n].leftFirst = ( node.start << 1 ) + 0;
		}
		else
		{
			bvhTree[n].leftFirst = ( node.rightOffset << 1 ) + 1;
		}
	}
}

void BVH::getSplitDimAndCoordBySAH( uint32_t &split_dim, float &split_coord, uint32_t binnedNum, AABB &bc, uint32_t &start, uint32_t &end )
{
	float fCurrentCost = Utils::MAX_FLOAT;

	for (uint32_t dim=0;dim<3;dim++)
	{
		uint32_t split_dim_current = dim;

		float split_inc_step = ( bc.max[split_dim_current] - bc.min[split_dim_current] ) / binnedNum;
		for ( uint32_t j = 1; j < binnedNum; j++ )
		{
			float split_coord_current = bc.min[split_dim_current] + j * split_inc_step;
			float leftArea = 0;
			float rightArea = 0;

			float leftNumber = 0;
			float rightNumber = 0;

			for ( uint32_t i = start; i < end; ++i )
			{
				const AABB& box = ( *build_prims )[i]->getAABB();
				float area = box.surfaceArea();

				if ( box.min[split_dim_current] < split_coord_current && box.max[split_dim_current] > split_coord_current )
				{
					float ratio = ( split_coord_current - box.min[split_dim_current] ) / ( box.max[split_dim_current] - box.min[split_dim_current] );
					
					leftArea += area;// * ratio;
					rightArea += area; // * ( 1 - ratio );

					leftNumber++;
					rightNumber++;
				}
				else if ( ( *build_prims )[i]->getCentroid()[split_dim_current] < split_coord_current )
				{
					leftArea += area;
					leftNumber++;
				}
				else
				{
					rightArea += area;
					rightNumber++;
				}
			}

			float fCost = leftArea * leftNumber + rightArea * rightNumber;
			if ( fCurrentCost > fCost )
			{
				split_dim = split_dim_current;
				split_coord = split_coord_current;

				fCurrentCost = fCost;
			}
		}
	}
}