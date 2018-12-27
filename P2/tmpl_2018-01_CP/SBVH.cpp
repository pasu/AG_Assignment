#include "precomp.h"
#include "SBVH.h"
#include <algorithm>
#include <iostream>
#include <unordered_set>
#include <tuple>

#include "BVH.h"
#include "Scene.h"

const float COST_TRAVERSAL = 0.125f;
const float COST_INTERSECTION = 1.0f;

struct CompareCentroid
{
	CompareCentroid( int dim ) : dim( dim ){};

	int dim;
	bool operator()( const PrimInfo node1, PrimInfo node2 ) const
	{
		return ( ( node1.bbox.min + node1.bbox.max ) / 2 )[dim] < ( ( node2.bbox.min + node2.bbox.max ) / 2 )[dim];
	}
};

 SBVH::SBVH( std::vector<RTPrimitive *> *objects, uint32_t leafSize /*= 4 */ )
	: build_prims( objects ), leafSize(leafSize)
{
	 m_maxDepth = 10;
	 m_spatialSplitBudget = 20;
	 m_spatialSplitCount = 0;
	 build();
 }

SBVH::~SBVH()
{
}

void SBVH::build()
{
	if ( ( *build_prims ).size() == 0 )
	{
		return;
	}

	std::vector<PrimInfo> primInfos( size_t( (*build_prims).size() * 1.2 ), {INVALID_ID, AABB()} ); // Pad 20% more space for spatial split fragments

	for ( size_t i = 0; i < (*build_prims).size(); i++ )
	{
		primInfos[i] = {i, ( *build_prims )[i]->getAABB()};
	}

	PrimID totalNodes = 0;
	std::vector<RTPrimitive*> orderedGeoms;
	PrimID first = 0;
	PrimID last = primInfos.size();
	m_root = BuildRecursive( first, last, totalNodes, primInfos, orderedGeoms, 0, true );
	Flatten();
}

RTIntersection SBVH::DoesIntersect( const RTRay &r )
{
	float nearestT = INFINITY;
	RTIntersection nearestIsx;

	// Update ray's traversal cost for visual debugging
	//r.m_traversalCost += COST_TRAVERSAL;

	if ( m_root->IsLeaf() )
	{
		SBVHLeaf *leaf = dynamic_cast<SBVHLeaf *>( m_root );
		//for (int i = 0; i < leaf->m_numGeoms; i++)
		//{
		//	r.m_traversalCost += COST_INTERSECTION;
		//
		//	std::shared_ptr<Geometry> geom = m_geoms[leaf->m_firstGeomOffset + i];
		//	Intersection isx = geom->GetIntersection(r);
		//	if (isx.t > 0 && isx.t < nearestT)
		//	{
		//		nearestT = isx.t;
		//		nearestIsx = isx;
		//	}
		//}
		for ( auto geomId : leaf->m_geomIds )
		{
			//r.m_traversalCost += COST_INTERSECTION;

			RTPrimitive* geom = (*build_prims)[geomId];
			RTIntersection isx = geom->intersect( r );
			if ( isx.isIntersecting() == true && isx.rayT < nearestT )
			{
				nearestT = isx.rayT;
				nearestIsx = isx;
			}
		}
		return nearestIsx;
	}

	// Traverse children
	GetIntersectionRecursive( r, m_root->m_nearChild, nearestT, nearestIsx );
	GetIntersectionRecursive( r, m_root->m_farChild, nearestT, nearestIsx );

	return nearestIsx;
}

// void *SBVH::convertToBVHTree( void *scene, std::vector<RTPrimitive *> *objects )
// {
// 	int nCount = m_nodes.size();
// 
// 	BVHNode_32* bvhTree = new BVHNode_32[nCount];
// 	uint32_t stackptr = 0;
// 
// 	std::vector<RTPrimitive *> prims;
// 
// 	std::vector<SBVHNode*> buildnodes;
// 	buildnodes.push_back( m_root );
// 
// 	while (buildnodes.size())
// 	{
// 		SBVHNode *node = buildnodes.back();
// 		buildnodes.pop_back();
// 		if (node->IsLeaf())
// 		{
// 			SBVHLeaf *leaf = (SBVHLeaf *)node;
// 
// 			BVHNode_32& nodeBVH = bvhTree[stackptr++];
// 			
// 			nodeBVH.bounds = leaf->m_bbox;
// 			nodeBVH.count = leaf->m_geomIds.size();
// 			
// 			int start = prims.size();
// 
// 			nodeBVH.leftFirst = ( start << 1 ) + 0;
// 
// 			for ( auto geomId : leaf->m_geomIds )
// 			{
// 				RTPrimitive *prim = ( *build_prims )[geomId];
// 				prims.push_back( prim );
// 			}			
// 		}
// 		else
// 		{
// 			if ( node->m_nearChild != NULL)
// 			{
// 				BVHNode_32& nodeBVH = bvhTree[stackptr++];
// 				nodeBVH.bounds = node->m_bbox;
// 
// 				int nCount = getchildnum( node->m_nearChild );
// 				nodeBVH.leftFirst = ( nCount << 1 ) + 1;
// 
// 				buildnodes.push_back( node->m_farChild );
// 				buildnodes.push_back( node->m_nearChild );
// 			}
// 		}
// 	}
// 
// 	return tree;
// }

SBVHNode *SBVH::BuildRecursive( PrimID first, PrimID last, PrimID &nodeCount, std::vector<PrimInfo> &primInfos, std::vector<RTPrimitive *> &orderedGeoms, int depth, bool shouldInsertAtBack )
{
	if ( last <= first || last < 0 || first < 0 )
	{
		return nullptr;
	}

	// == COMPUTE BOUNDS OF ALL GEOMETRIES
	AABB bboxAllGeoms = primInfos[0].bbox;
	for ( int i = first; i < last; i++ )
	{
		if ( primInfos[i].primitiveId == INVALID_ID ) continue;
		bboxAllGeoms.expandToInclude( primInfos[i].bbox );
	}

	// Num primitive should only reflect unique IDs
	PrimID numPrimitives = 0;
	for ( auto prim : primInfos )
	{
		if ( prim.primitiveId != INVALID_ID )
		{
			++numPrimitives;
		}
		else
		{
			continue;
		}
	}

	// == GENERATE SINGLE GEOMETRY LEAF NODE
	if ( numPrimitives == 1 || depth >= m_maxDepth )
	{
		return CreateLeaf( nullptr, first, last, nodeCount, primInfos, orderedGeoms, bboxAllGeoms );
	}

	// COMPUTE BOUNDS OF ALL CENTROIDS
	AABB bboxCentroids = primInfos[0].bbox;
	for ( int i = first; i < last; i++ )
	{
		if ( primInfos[i].primitiveId == INVALID_ID ) continue;

		bboxCentroids.expandToInclude( ( primInfos[i].bbox.min + primInfos[i].bbox.max)/2 );
	}

	// Get maximum extent
	auto dim = bboxCentroids.maxDimension();

	// === GENERATE PLANAR LEAF NODE
	// If all centroids are the same, create leafe since there's no effective way to split the tree
	if ( bboxCentroids.max[dim] == bboxCentroids.min[dim] )
	{
		return CreateLeaf( nullptr, first, last, nodeCount, primInfos, orderedGeoms, bboxAllGeoms );
	}

	std::tuple<Cost, BucketID> objSplitCost;

	PrimID mid;
	if ( numPrimitives <= 4 )
	{
		PartitionEqualCounts( dim, first, last, mid, primInfos );
	}
	else
	{
		// Update maximum extend to be all bounding boxes, not just the centroids
		int allGeomsDim = bboxAllGeoms.maxDimension();

		const float RESTRICT_ALPHA = 1e-5;

		// === FIND OBJECT SPLIT CANDIDATE
		// For each primitive in range, determine which bucket it falls into
		objSplitCost =
			CalculateObjectSplitCost( dim, first, last, primInfos, bboxCentroids, bboxAllGeoms );

		bool isSpatialSplit = false;
		Cost minSplitCost = std::get<Cost>( objSplitCost );
		std::tuple<Cost, BucketID> spatialSplitCost;
		if ( m_spatialSplitBudget > 0 )
		{
			std::vector<BucketInfo> spatialBuckets;
			spatialBuckets.resize( NUM_BUCKET );
			spatialSplitCost =
				CalculateSpatialSplitCost( allGeomsDim, first, last, primInfos, bboxAllGeoms, spatialBuckets );

			// Get the cheapest cost between object split candidate and spatial split candidate

			if ( minSplitCost > std::get<Cost>( spatialSplitCost ) )
			{
				minSplitCost = std::get<Cost>( spatialSplitCost );
				isSpatialSplit = true;

				// Consume budget
				m_spatialSplitBudget--;

				std::vector<PrimInfo> origPrims;
				PrimID front = first;
				PrimID back = last - 1;
				BucketID spatialBucket = std::get<BucketID>( spatialSplitCost );
				for ( auto frag : spatialBuckets[spatialBucket].fragments )
				{
					PrimInfo origPrim = primInfos.at( frag.origPrimOffset );
					origPrims.push_back( origPrim );
				}

				// Create new fragments
				PrimID count = 0;
				for ( auto frag : spatialBuckets[std::get<BucketID>( spatialSplitCost )].fragments )
				{
					PrimInfo origPrim = origPrims.at( count );
					count++;

					PrimInfo left = {frag.primitiveId, frag.bbox};
					left.bbox.min[allGeomsDim] = origPrim.bbox.min[allGeomsDim];
					assert( left.bbox.min[allGeomsDim] <= left.bbox.max[allGeomsDim] );
					
					//left.bbox.m_transform = Transform( left.bbox.m_centroid, glm::vec3( 0 ), left.bbox.m_max - left.bbox.m_min );

					PrimInfo right = {frag.primitiveId, frag.bbox};
					right.bbox.max[allGeomsDim] = origPrim.bbox.max[allGeomsDim];
					assert( right.bbox.min[allGeomsDim] <= right.bbox.max[allGeomsDim] );
					//right.bbox.m_transform = Transform( right.bbox.m_centroid, glm::vec3( 0 ), right.bbox.m_max - right.bbox.m_min );

					// Test whether we can unfit this reference if it's overlapping is too small
					// Remember that spatial split cost is :
					// COST_TRAVERSAL + COST_INTERSECTION * (count0 * bbox0.GetSurfaceArea() + count1 * bbox1.GetSurfaceArea()) * invAllGeometriesSA;

					Cost csplit = ( std::get<Cost>( spatialSplitCost ) - COST_TRAVERSAL ) * bboxAllGeoms.surfaceArea() / COST_INTERSECTION;

					left.bbox.expandToInclude( origPrim.bbox );
					Cost c0 = left.bbox.surfaceArea() *
								  spatialBuckets[spatialBucket].enter +
							  right.bbox.surfaceArea() * ( spatialBuckets[spatialBucket].exit - 1 );
					right.bbox.expandToInclude( origPrim.bbox );
					Cost c1 = left.bbox.surfaceArea() *
								  ( spatialBuckets[spatialBucket].enter - 1 ) +
							  right.bbox.surfaceArea() * spatialBuckets[spatialBucket].exit;

					if ( csplit < c0 && csplit < c1 )
					{
						// Insert the new fragments into the priminfos list at both children
						primInfos.at( frag.origPrimOffset ) = left;
						if ( shouldInsertAtBack )
						{
							primInfos[back] = right;
							--back;
						}
						else
						{
							primInfos[front] = right;
							++front;
						}
					}
					else if ( c0 < c1 )
					{
					}
					else
					{
					}
				}
			}
		}

		// == CREATE LEAF OR SPLIT
		float leafCost = numPrimitives * COST_INTERSECTION;
		if ( numPrimitives > leafSize || minSplitCost < leafCost )
		{
			// Split node

			if ( isSpatialSplit )
			{
				PartitionSpatial( std::get<BucketID>( spatialSplitCost ), allGeomsDim, first, last, mid, primInfos, bboxAllGeoms );
				++m_spatialSplitCount;
			}
			else
			{
				PartitionObjects( std::get<BucketID>( objSplitCost ), dim, first, last, mid, primInfos, bboxCentroids );
			}
		}
		else
		{
			// == CREATE LEAF
			SBVHLeaf *leaf = CreateLeaf(
				nullptr, first, last, nodeCount, primInfos, orderedGeoms, bboxAllGeoms );
			return leaf;
		}
	}

	// Build near child
	if (mid == first)
	{
		mid++;
	}
	SBVHNode *nearChild = BuildRecursive( first, mid, nodeCount, primInfos, orderedGeoms, depth + 1, true );

	// Build far child
	if ( mid == last )
	{
		mid--;
	}
	SBVHNode *farChild = BuildRecursive( mid, last, nodeCount, primInfos, orderedGeoms, depth + 1, false );

	SBVHNode *node = new SBVHNode( nullptr, nearChild, farChild, nodeCount, dim );
	if ( nearChild )
		nearChild->m_parent = node;
	if ( farChild )
		farChild->m_parent = node;
	nodeCount++;
	return node;
}

void SBVH::Flatten()
{
	FlattenRecursive( m_root );
}

void SBVH::FlattenRecursive( SBVHNode *node )
{
	if ( node == nullptr )
	{
		return;
	}

	m_nodes.push_back( node );
	FlattenRecursive( node->m_nearChild );
	FlattenRecursive( node->m_farChild );
}

void SBVH::GetIntersectionRecursive( const RTRay &r, SBVHNode *node, float &nearestT, RTIntersection &nearestIsx )
{
	// Update ray's traversal cost for visual debugging
	//r.m_traversalCost += COST_TRAVERSAL;

	if ( node == nullptr )
	{
		return;
	}

	float fn[2];

	if ( node->IsLeaf() )
	{
		SBVHLeaf *leaf = dynamic_cast<SBVHLeaf *>( node );
		
		if ( leaf->m_numGeoms < 4 || node->m_bbox.intersect( r,fn,fn+1 ) )
		{
			// Return nearest primitive
			//for (int i = 0; i < leaf->m_numGeoms; i++)
			//{
			//	r.m_traversalCost += COST_INTERSECTION;

			//	std::shared_ptr<Geometry> geom = m_geoms[leaf->m_firstGeomOffset + i];
			//	Intersection isx = geom->GetIntersection(r);
			//	if (isx.t > 0 && isx.t < nearestT)
			//	{
			//		r.m_traversalCost = COST_INTERSECTION;
			//		nearestT = isx.t;
			//		nearestIsx = isx;
			//	}
			//}
			for ( auto geomId : leaf->m_geomIds )
			{
				//r.m_traversalCost += COST_INTERSECTION;

				RTPrimitive* geom = (*build_prims)[geomId];
				RTIntersection isx = geom->intersect( r );
				if ( isx.isIntersecting() == true && isx.rayT < nearestT )
				{
					nearestT = isx.rayT;
					nearestIsx = isx;
				}
			}
		}
		return;
	}

	if ( node->m_bbox.intersect( r,fn,fn+1 ) )
	{
		// Traverse children
		GetIntersectionRecursive( r, node->m_nearChild, nearestT, nearestIsx );
		GetIntersectionRecursive( r, node->m_farChild, nearestT, nearestIsx );
	}
}

SBVHLeaf *SBVH::CreateLeaf( SBVHNode *parent, PrimID first, PrimID last, PrimID &nodeCount, std::vector<PrimInfo> &primInfos, std::vector<RTPrimitive *> &orderedGeoms, AABB &bboxAllGeoms )
{
	PrimID numPrimitives = 0;
	for ( auto prim : primInfos )
	{
		if ( prim.primitiveId != INVALID_ID )
		{
			++numPrimitives;
		}
		else
		{
			continue;
		}
	}

	size_t firstGeomOffset = orderedGeoms.size();
	std::unordered_set<PrimID> geomIds;
	for ( int i = first; i < last; i++ )
	{
		PrimID primID = primInfos.at( i ).primitiveId;
		if ( primID == INVALID_ID ) continue;

		orderedGeoms.push_back( (*build_prims)[primID] );
		geomIds.insert( primID );
	}
	SBVHLeaf *leaf = new SBVHLeaf( parent, nodeCount, firstGeomOffset, numPrimitives, bboxAllGeoms );
	nodeCount++;
	leaf->m_geomIds = geomIds;
	return leaf;
}

SBVHNode *SBVH::CreateNode( SBVHNode *parent, SBVHNode *nearChild, SBVHNode *farChild, PrimID &nodeCount, int dim )
{
	SBVHNode *node = new SBVHNode( parent, nearChild, farChild, nodeCount, dim );
	nodeCount++;
	return node;
}

void SBVH::PartitionEqualCounts( int dim, PrimID first, PrimID last, PrimID &mid, std::vector<PrimInfo> &geomInfos ) const
{
	mid = ( first + last ) / 2;
	std::nth_element( &geomInfos[first], &geomInfos[mid], &geomInfos[last - 1] + 1, CompareCentroid( dim ) );
}

void SBVH::PartitionObjects( Cost minCostBucket, int dim, PrimID first, PrimID last, PrimID &mid, std::vector<PrimInfo> &primInfos, AABB &bboxCentroids )
{
	std::vector<PrimInfo> temp;
	for ( PrimID i = first; i < last; i++ )
	{
		if ( primInfos[i].primitiveId == INVALID_ID ) continue;

		temp.push_back( primInfos.at( i ) );
		// Clear out that memory
		primInfos.at( i ) = {INVALID_ID, AABB()};
	}

	PrimID front = first;
	PrimID back = last - 1;
	for ( PrimInfo info : temp )
	{
		// Partition geometry into two halves, before and after the split, and leave the middle empty
		int whichBucket = NUM_BUCKET * bboxCentroids.Offset( ( info.bbox.min + info.bbox.max)/2 )[dim];
		assert( whichBucket <= NUM_BUCKET );
		if ( whichBucket == NUM_BUCKET ) whichBucket = NUM_BUCKET - 1;
		if ( whichBucket <= minCostBucket )
		{
			primInfos.at( front ) = info;
			front++;
		}
		else
		{
			primInfos.at( back ) = info;
			back--;
		}
		assert( front <= back + 1 );
	}

	mid = ( front + back ) / 2;
}

std::tuple<Cost, BucketID> SBVH::CalculateObjectSplitCost( int dim, PrimID first, PrimID last, std::vector<PrimInfo> &primInfos, AABB &bboxCentroids, AABB &bboxAllGeoms ) const
{
	BucketInfo buckets[NUM_BUCKET];
	Cost costs[NUM_BUCKET - 1];
	float invAllGeometriesSA = 1.0f / bboxAllGeoms.surfaceArea();

	// For each primitive in range, determine which bucket it falls into
	for ( int i = first; i < last; i++ )
	{
		if ( primInfos[i].primitiveId == INVALID_ID ) continue;

		Vector3 centre = ( primInfos.at( i ).bbox.min + primInfos.at( i ).bbox.max ) / 2;
		int whichBucket = NUM_BUCKET * bboxCentroids.Offset( centre )[dim];
		assert( whichBucket <= NUM_BUCKET );
		if ( whichBucket == NUM_BUCKET ) whichBucket = NUM_BUCKET - 1;

		buckets[whichBucket].count++;
		if ( buckets[whichBucket].bbox.bValid==false)
		{
			buckets[whichBucket].bbox = primInfos.at( i ).bbox;
			buckets[whichBucket].bbox.bValid = true;
		}
		else
		{
			buckets[whichBucket].bbox.expandToInclude( primInfos.at( i ).bbox );
		}
		
	}

	// Compute cost for splitting after each bucket
	for ( int i = 0; i < NUM_BUCKET - 1; i++ )
	{
		AABB bbox0, bbox1;
		int count0 = 0, count1 = 0;

		// Compute cost for buckets before split candidate
		for ( int j = 0; j <= i; j++ )
		{
			if (j==0)
			{
				bbox0 = buckets[j].bbox;
			}
			else
			{
				bbox0.expandToInclude( buckets[j].bbox );
			}
			count0 += buckets[j].count;
		}

		bbox1 = buckets[0].bbox;
		// Compute cost for buckets after split candidate
		for ( int j = i + 1; j < NUM_BUCKET; j++ )
		{
			if (j==i+1)
			{
				bbox1 = ( buckets[j].bbox );
			}
			else
			{
				bbox1.expandToInclude( buckets[j].bbox );
			}
			count1 += buckets[j].count;
		}

		costs[i] = COST_TRAVERSAL + COST_INTERSECTION * ( count0 * bbox0.surfaceArea() + count1 * bbox1.surfaceArea() ) * invAllGeometriesSA;
	}

	// Now that we have the costs, we can loop through our buckets and find
	// which bucket has the lowest cost
	Cost minCost = costs[0];
	BucketID minCostBucket = 0;
	for ( int i = 1; i < NUM_BUCKET - 1; i++ )
	{
		if ( costs[i] < minCost )
		{
			minCost = costs[i];
			minCostBucket = i;
		}
	}

	return std::tuple<Cost, BucketID>( minCost, minCostBucket );
}

std::tuple<Cost, BucketID> SBVH::CalculateSpatialSplitCost( int dim, PrimID first, PrimID last, std::vector<PrimInfo> &primInfos, AABB &bboxAllGeoms, std::vector<BucketInfo> &buckets ) const
{

	Cost costs[NUM_BUCKET - 1];
	float invAllGeometriesSA = 1.0f / bboxAllGeoms.surfaceArea();
	float bucketSize = ( bboxAllGeoms.max[dim] - bboxAllGeoms.min[dim] ) / NUM_BUCKET;

	// For each primitive in range, determine which bucket it falls into
	for ( int i = first; i < last; i++ )
	{
		if ( primInfos[i].primitiveId == INVALID_ID ) continue;

		BucketID minBucket = ( NUM_BUCKET * bboxAllGeoms.Offset( primInfos.at( i ).bbox.min )[dim] );
		assert( minBucket <= NUM_BUCKET );
		if ( minBucket == NUM_BUCKET ) minBucket = NUM_BUCKET - 1;

		BucketID maxBucket = ( NUM_BUCKET * bboxAllGeoms.Offset( primInfos.at( i ).bbox.max )[dim] );
		assert( maxBucket <= NUM_BUCKET );
		if ( maxBucket == NUM_BUCKET ) maxBucket = NUM_BUCKET - 1;

		buckets[minBucket].enter++;
		buckets[maxBucket].exit++;

		if ( minBucket != maxBucket )
		{

			// Naively split into equal sized bboxes
			for ( BucketID b = minBucket; b <= maxBucket; b++ )
			{
				AABB bbox = primInfos.at( i ).bbox;
				float nearSplitPlane = b * bucketSize + bboxAllGeoms.min[dim];
				float farSplitPlane = nearSplitPlane + bucketSize;
				if ( ( bbox.min[dim] < nearSplitPlane && bbox.max[dim] < nearSplitPlane ) || bbox.min[dim] > farSplitPlane && bbox.max[dim] > farSplitPlane )
				{
					continue;
				}
				bbox.min[dim] = std::max( bbox.min[dim], nearSplitPlane );
				bbox.max[dim] = std::min( bbox.max[dim], farSplitPlane );
				assert( bbox.min[dim] <= bbox.max[dim] );
				//bbox.m_transform = Transform( bbox.m_centroid, glm::vec3( 0 ), bbox.m_max - bbox.m_min );
				// Create a new geom info to hold the splitted geometry
				PrimInfo fragInfo;
				fragInfo.bbox = bbox;
				fragInfo.primitiveId = primInfos.at( i ).primitiveId;
				fragInfo.origPrimOffset = i;
				buckets[b].fragments.push_back( fragInfo );

				if ( buckets[b].bbox.bValid==false )
				{
					buckets[b].bbox = bbox;
					buckets[b].bbox.bValid = true;
				}
				else
				{
					buckets[b].bbox.expandToInclude( bbox );
				}
				
			}
		}
		else
		{
			buckets[minBucket].bbox = primInfos.at( i ).bbox;
		}
	}

	// Compute cost for splitting after each bucket
	for ( int i = 0; i < NUM_BUCKET - 1; i++ )
	{
		AABB bbox0, bbox1;
		int count0 = 0, count1 = 0;


		// Compute cost for buckets before split candidate
		for ( BucketID j = 0; j <= i; j++ )
		{
			if (j==0)
			{
				bbox0 = ( buckets[j].bbox );
			}
			else
			{
				bbox0.expandToInclude( buckets[j].bbox );
			}
			count0 += buckets[j].enter;
		}

		// Compute cost for buckets after split candidate
		for ( BucketID j = i + 1; j < NUM_BUCKET; j++ )
		{
			if (j==i+1)
			{
				bbox1 = ( buckets[j].bbox );
			}
			else
			{
				bbox1.expandToInclude( buckets[j].bbox );
			}			
			count1 += buckets[j].exit;
		}

		costs[i] = COST_TRAVERSAL + COST_INTERSECTION * ( count0 * bbox0.surfaceArea() + count1 * bbox1.surfaceArea() ) * invAllGeometriesSA;
	}

	// Now that we have the costs, we can loop through our buckets and find
	// which bucket has the lowest cost
	Cost minCost = costs[0];
	BucketID minCostBucket = 0;
	for ( int i = 1; i < NUM_BUCKET - 1; i++ )
	{
		if ( costs[i] < minCost )
		{
			minCost = costs[i];
			minCostBucket = i;
		}
	}

	return std::tuple<Cost, BucketID>( minCost, minCostBucket );
}

void SBVH::PartitionSpatial( BucketID minCostBucket, int dim, PrimID first, PrimID last, PrimID &mid, std::vector<PrimInfo> &primInfos, AABB &bboxAllGeoms )
{
	std::vector<PrimInfo> temp;
	for ( PrimID i = first; i < last; i++ )
	{
		if ( primInfos[i].primitiveId == INVALID_ID ) continue;

		temp.push_back( primInfos.at( i ) );
		// Clear out that memory
		primInfos.at( i ) = {INVALID_ID, AABB()};
	}

	PrimID front = first;
	PrimID back = last - 1;
	for ( PrimInfo info : temp )
	{
		// Partition geometry into two halves, before and after the split, and leave the middle empty
		BucketID startEdgeBucket = NUM_BUCKET * bboxAllGeoms.Offset( info.bbox.min )[dim];
		assert( startEdgeBucket <= NUM_BUCKET );
		if ( startEdgeBucket == NUM_BUCKET ) startEdgeBucket = NUM_BUCKET - 1;
		if ( startEdgeBucket <= minCostBucket )
		{
			primInfos.at( front ) = info;
			front++;
		}
		else
		{
			primInfos.at( back ) = info;
			back--;
		}
		assert( front <= back+1 );
		if ( front > back )
		{
			int a = 100;
		}
	}

	mid = ( front + back ) / 2;

	//PrimInfo *pmid = std::partition(
	//	&primInfos[first], &primInfos[last - 1] + 1,
	//	[=](const PrimInfo& gi)
	//{
	//	BucketID startEdgeBucket = NUM_BUCKET * bboxAllGeoms.Offset(gi.bbox.m_min)[dim];;
	//	assert(startEdgeBucket <= NUM_BUCKET);
	//	if (startEdgeBucket == NUM_BUCKET) startEdgeBucket = NUM_BUCKET - 1;

	//	return startEdgeBucket <= minCostBucket;
	//});
	//mid = pmid - &primInfos[0];
}

int SBVH::getchildnum( SBVHNode *node )
{
	int nLeft, nRight;
	nLeft = nRight = 0;
	if (node->m_nearChild != NULL)
	{
		nLeft = getchildnum( node->m_nearChild );
	}

	if ( node->m_farChild != NULL )
	{
		nRight = getchildnum( node->m_farChild );
	}

	return nLeft + nRight + 1;
}
