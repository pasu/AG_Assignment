#include "TopLevelBVH.h"
#include "precomp.h"

TopLevelBVH::TopLevelBVH( const std::vector<RTObject *> &objects ) : objects( objects ), tree_size( objects.size() * 2 - 1 )
{
	// alloc memory for the tree
	nodes = new TopLevelBVHNode[tree_size];

	rebuild();

	for ( unsigned int i = 0; i < RAYPACKET_RAYS_PER_PACKET; ++i )
	{
		m_I[i] = i;
	}
}

TopLevelBVH::~TopLevelBVH()
{
	delete[] nodes;
}

void TopLevelBVH::rebuild()
{
	for ( auto object : objects )
	{
		object->updateAABBbounds();
	}
	for ( int i = 0; i < objects.size(); i++ )
	{
		nodes[tree_size - i - 1].bounds = objects[i]->getAABBBounds();
		nodes[tree_size - i - 1].object = objects[i];
		nodes[tree_size - i - 1].son = 0; // not necessary, neither harmful
	}
	// nodes[2n-1]:
	// nodes={root,inner_node,...,inner_node,leaf_node,...,leaf_node}
	// root and (n-2) innernodes and n leafnodes

	int leaf_ptr = tree_size;					// index of the last leaf node that was added in th tree
	int inner_ptr = tree_size - objects.size(); // index of the last inner node that was added to the tree

	// Any inner node whose index is larger than inner_ptr has been added to the tree
	// Any leaf node whose index is less than leaf_ptr has not been added to the tree
	// So, any node whose index is in [inner_ptr, leaf_ptr) need to be checked.

	// function to find the best match of a node
	auto findBestMatch = [&]( int a ) {
		assert( inner_ptr > 0 );
		assert( inner_ptr <= a );
		assert( a < leaf_ptr );

		int least_index = 0;
		float least_surface = numeric_limits<float>::max();
		// check every one in [inner_ptr, leaf_ptr)
		for ( int b = inner_ptr; b < leaf_ptr; b++ )
		{
			if ( b == a ) continue;
			// aabb = i.aabb+b.aabb
			AABB aabb = nodes[a].bounds;
			aabb.expandToInclude( nodes[b].bounds );
			float sa = aabb.surfaceArea();

			if ( sa < least_surface )
			{
				least_surface = sa;
				least_index = b;
			}
		}
		return least_index;
	};

	int A = tree_size - 1; // init finding best pair with the last one
	int B = findBestMatch( A );
	while ( inner_ptr > 0 )
	{
		int C = findBestMatch( B );
		if ( A == C )
		{ // best pair is found
			// swap them to the "finished" segment
			leaf_ptr--;
			std::swap( nodes[A], nodes[leaf_ptr] );
			leaf_ptr--;
			std::swap( nodes[B], nodes[leaf_ptr] );
			// construct their father node
			inner_ptr--;
			nodes[inner_ptr].bounds = nodes[leaf_ptr].bounds;
			nodes[inner_ptr].bounds.expandToInclude( nodes[leaf_ptr + 1].bounds );
			nodes[inner_ptr].object = nullptr;
			nodes[inner_ptr].son = leaf_ptr;

			// exit strategy
			if ( inner_ptr == 0 ) break;

			// prepare for next pair
			A = inner_ptr;
			B = findBestMatch( A );
		}
		else
		{ // No best pair found
			A = B;
			B = C;
		}
	}
}

bool TopLevelBVH::getIntersection( const RTRay &ray, RTIntersection *intersection ) const
{
	assert( intersection->object == nullptr );
	assert( !intersection->isIntersecting() );

	// Working set
	std::vector<int> todo;
	todo.reserve( 100 );

	// recursive code:
	// intersection visitNode(node,ray){
	//     static intersection;
	//     intersect, fnear, ffar = findIntersection(node.aabb,ray)
	//     if(!intersect) return;
	//     if(fnear > intersection.rayT) return;
	//     if(is leaf)
	//         intersect = intersectleave(node, ray)
	//     if(is not leaf)
	//         visitNode(son,ray);
	//         visitNode(son+1,ray);
	// }
	//
	// "Push" on the root node to the working set
	todo.push_back( 0 );

	while ( !todo.empty() )
	{
		// Pop off the next node to work on.
		TopLevelBVHNode current = nodes[todo.back()];
		todo.pop_back();

		float fNear, fFar;
		bool intersect;
		intersect = current.bounds.intersect( ray, &fNear, &fFar );

		// early exit when not intersecting aabb
		if ( !intersect ) continue;

		// If this node is further than the closest found intersection, continue
		if ( intersection->isIntersecting() && ( fNear > intersection->rayT ) )
			continue;

		if ( current.object ) // current is leaf node
		{
			RTIntersection currentIntersection; // find the intersection with subBVH
			if ( current.object->getIntersection( ray, currentIntersection ) )
			{
				if ( !( intersection->isIntersecting() ) || ( intersection->isIntersecting() && ( currentIntersection.rayT < intersection->rayT ) ) )
					*intersection = currentIntersection;
			}
		}
		else // current is inner node
		{
			todo.push_back( current.son );
			todo.push_back( current.son + 1 );
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

#ifdef BVH_RANGED_TRAVERSAL
bool TopLevelBVH::getIntersection( const RayPacket &raypacket, RTIntersection *intersection ) const
{
// 	assert( intersection->object == nullptr );
// 	assert( !intersection->isIntersecting() );

	// Working set
	StackNode todo[64];

	bool anyHitted = false;
	int todoOffset = 0, nodeNum = 0;
	unsigned int ia = 0;

	while ( true )
	{
		// Pop off the next node to work on.
		TopLevelBVHNode currentN = nodes[nodeNum];
		
		ia = 0;
		ia = getFirstHit( raypacket, currentN.bounds, ia );

		if ( ia < RAYPACKET_RAYS_PER_PACKET )
		{
			if ( currentN.object ) // current is leaf node
			{
				const unsigned int ie = getLastHit( raypacket,
													currentN.bounds,
													ia );

				if ( raypacket.m_Frustum.Intersect( currentN.object->getAABBBounds() ) )
				{
					for ( unsigned int i = ia; i < ie; ++i )
					{
						RTIntersection current;

						currentN.object->getIntersection( raypacket.m_ray[i], current );

						if ( current.isIntersecting() )
						{
							anyHitted |= true;

							if ( intersection[i].rayT < 0.0f || current.rayT < intersection[i].rayT )
							{
								( intersection[i] ) = current;
							}
						}
					}
				}
			}
			else // current is inner node
			{
				StackNode &node = todo[todoOffset++];

				node.cell = currentN.son+1;
				node.ia = ia;
				nodeNum = currentN.son;
				continue;
			}
		}

		if ( todoOffset == 0 )
			break;

		const StackNode &node = todo[--todoOffset];

		nodeNum = node.cell;
		ia = node.ia;
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
#endif

#ifdef BVH_PARTITION_TRAVERSAL
bool TopLevelBVH::getIntersection( const RayPacket &raypacket, RTIntersection *intersection ) const
{
	// 	assert( intersection->object == nullptr );
	// 	assert( !intersection->isIntersecting() );

	// Working set
	StackNode todo[64];

	bool anyHitted = false;
	int todoOffset = 0, nodeNum = 0;
	unsigned int ia = 0;

	unsigned int I[RAYPACKET_RAYS_PER_PACKET];
	memcpy( I, m_I, RAYPACKET_RAYS_PER_PACKET * sizeof( unsigned int ) );

	while ( true )
	{
		// Pop off the next node to work on.
		TopLevelBVHNode currentN = nodes[nodeNum];

		ia = partRays( raypacket, currentN.bounds, ia, I );

		if ( ia < RAYPACKET_RAYS_PER_PACKET )
		{
			if ( currentN.object ) // current is leaf node
			{
				const unsigned int ie = getLastHit( raypacket,
													currentN.bounds,
													ia, I );

				if ( raypacket.m_Frustum.Intersect( currentN.object->getAABBBounds() ) )
				{
					for ( unsigned int i = ia; i < ie; ++i )
					{
						RTIntersection current;

						currentN.object->getIntersection( raypacket.m_ray[i], current );

						if ( current.isIntersecting() )
						{
							anyHitted |= true;

							if ( intersection[i].rayT < 0.0f || current.rayT < intersection[i].rayT )
							{
								( intersection[i] ) = current;
							}
						}
					}
				}
			}
			else // current is inner node
			{
				StackNode &node = todo[todoOffset++];

				node.cell = currentN.son + 1;
				node.ia = ia;
				nodeNum = currentN.son;
				continue;
			}
		}

		if ( todoOffset == 0 )
			break;

		const StackNode &node = todo[--todoOffset];

		nodeNum = node.cell;
		ia = node.ia;
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

int TopLevelBVH::getLastHit( const RayPacket &raypacket, const AABB &box, int ia, const unsigned int *aRayIndex ) const
{
	for ( unsigned int ie = ( RAYPACKET_RAYS_PER_PACKET - 1 ); ie > ia; --ie )
	{
		float bbhits[2];

		if ( box.intersect( raypacket.m_ray[aRayIndex[ie]], bbhits, bbhits + 1 ) )
			return ie + 1;
	}

	return ia + 1;
}

int TopLevelBVH::partRays( const RayPacket &raypacket, const AABB &box, int ia, unsigned int *aRayIndex ) const
{
	if ( !raypacket.m_Frustum.Intersect( box ) )
		return RAYPACKET_RAYS_PER_PACKET;

	unsigned int ie = 0;

	for ( unsigned int i = 0; i < ia; ++i )
	{
		float bbhits[2];

		if ( box.intersect( raypacket.m_ray[aRayIndex[i]], bbhits, bbhits + 1 ) )
			std::swap( aRayIndex[ie++], aRayIndex[i] );
	}

	return ie;
}
#endif
