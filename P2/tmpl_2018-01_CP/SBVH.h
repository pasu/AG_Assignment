#pragma once
#include "AABB.h"
#include "RTPrimitive.h"
#include "RTRay.h"
#include <unordered_set>

typedef size_t PrimID;
typedef size_t SBVHNodeId;
typedef size_t BucketID;
typedef float Cost;

const PrimID INVALID_ID = std::numeric_limits<size_t>::max();
const BucketID NUM_BUCKET = 12;

struct PrimInfo
{
	PrimID primitiveId;
	AABB bbox;
	PrimID origPrimOffset;
};

class BucketInfo
{
  public:
	BucketInfo() : count( 0 ), bbox{AABB()}, enter( 0 ), exit( 0 ){};

	int count = 0;
	AABB bbox;
	int enter = 0; // Number of entering references
	int exit = 0;  // Number of exiting references
	std::vector<PrimInfo> fragments;
};

class SBVHNode
{
  public:
	SBVHNode(
		SBVHNode *parent,
		SBVHNode *nearChild,
		SBVHNode *farChild,
		SBVHNodeId nodeIdx,
		int m_dim ) : m_parent( parent ),
					  m_nearChild( nearChild ),
					  m_farChild( farChild ),
					  m_nodeIdx( nodeIdx ),
					  m_dim( m_dim )
	{
		if ( nearChild )
		{
			if (m_bbox.bValid == false)
			{
				m_bbox = nearChild->m_bbox;
			}
			else
			{
				m_bbox.expandToInclude( nearChild->m_bbox );
			}
		}

		if ( farChild )
		{
			if ( m_bbox.bValid == false )
			{
				m_bbox = farChild->m_bbox;
			}
			else
			{
				m_bbox.expandToInclude( farChild->m_bbox );
			}
		}
	};

	virtual ~SBVHNode()
	{
		if ( m_nearChild )
		{
			delete m_nearChild;
		}

		if ( m_farChild )
		{
			delete m_farChild;
		}
	};

	virtual bool IsLeaf()
	{
		return false;
	}

	SBVHNode *m_parent;
	SBVHNode *m_nearChild;
	SBVHNode *m_farChild;
	AABB m_bbox;
	size_t m_nodeIdx;
	int m_dim;
};

class SBVHLeaf : public SBVHNode
{
  public:
	SBVHLeaf(
		SBVHNode *parent,
		size_t nodeIdx,
		size_t firstGeomOffset,
		size_t numGeoms,
		const AABB &bbox ) : SBVHNode( parent, nullptr, nullptr, nodeIdx, 0 ),
							 m_firstGeomOffset( firstGeomOffset ),
							 m_numGeoms( numGeoms )
	{
		m_bbox = bbox;
	}

	bool IsLeaf() final
	{
		return true;
	}

	size_t m_firstGeomOffset;
	size_t m_numGeoms;
	std::unordered_set<PrimID> m_geomIds;
};

class SBVH
{
  public:
	SBVH( std::vector<RTPrimitive *> *objects, uint32_t leafSize = 4 );
	~SBVH();

	std::vector<RTPrimitive *> *build_prims;
	SBVHNode *m_root;
	//! Build the BVH tree out of build_prims
	void build();

	RTIntersection DoesIntersect( const RTRay &r );

	//void *convertToBVHTree( void *scene, std::vector<RTPrimitive *> *objects );
  protected:
	SBVHNode *	BuildRecursive(		PrimID first,
		PrimID last,
		PrimID &nodeCount,
		std::vector<PrimInfo> &geomInfos,
		std::vector<RTPrimitive *> &orderedGeoms,
		int depth,
		bool shouldInsertAtBack );

	void Flatten();

	void FlattenRecursive( SBVHNode *node );

	void
	GetIntersectionRecursive(
		const RTRay &r,
		SBVHNode *node,
		float &nearestT,
		RTIntersection &nearestIsx );

	SBVHLeaf *
	CreateLeaf(
		SBVHNode *parent,
		PrimID first,
		PrimID last,
		PrimID &nodeCount,
		std::vector<PrimInfo> &geomInfos,
		std::vector<RTPrimitive*> &orderedGeoms,
		AABB &bboxAllGeoms );

	static SBVHNode *
	CreateNode(
		SBVHNode *parent,
		SBVHNode *nearChild,
		SBVHNode *farChild,
		PrimID &nodeCount,
		int dim );

	void PartitionEqualCounts(
		int dim,
		PrimID first,
		PrimID last,
		PrimID &mid,
		std::vector<PrimInfo> &geomInfos ) const;

	static void
	PartitionObjects(
		Cost minCostBucket,
		int dim,
		PrimID first,
		PrimID last,
		PrimID &mid,
		std::vector<PrimInfo> &geomInfos,
		AABB &bboxCentroids );

	std::tuple<Cost, BucketID>
	CalculateObjectSplitCost(
		int dim,
		PrimID first,
		PrimID last,
		std::vector<PrimInfo> &geomInfos,
		AABB &bboxCentroids,
		AABB &bboxAllGeoms ) const;

	std::tuple<Cost, BucketID>
	CalculateSpatialSplitCost(
		int dim,
		PrimID first,
		PrimID last,
		std::vector<PrimInfo> &geomInfos,
		AABB &bboxAllGeoms,
		std::vector<BucketInfo> &buckets ) const;

	static void
	PartitionSpatial(
		BucketID minCostBucket,
		int dim,
		PrimID first,
		PrimID last,
		PrimID &mid,
		std::vector<PrimInfo> &geomInfos,
		AABB &bboxAllGeoms );

	int getchildnum( SBVHNode *node );
  private:
	uint32_t nNodes, nLeafs, leafSize;
	

	unsigned int m_maxDepth;
	size_t m_spatialSplitBudget;
	unsigned int m_spatialSplitCount;

	std::vector<SBVHNode *> m_nodes;
};
