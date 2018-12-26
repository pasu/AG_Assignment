#include "precomp.h"
#include "RTFrustum.h"


RTFrustum::RTFrustum()
{
}

RTFrustum::~RTFrustum()
{
}

void RTFrustum::GenerateFrustum( const RTRay &topLeft, const RTRay &topRight, const RTRay &bottomLeft, const RTRay &bottomRight )
{
	m_points[0] = topLeft.orig;
	m_points[1] = topRight.orig;
	m_points[2] = bottomLeft.orig;
	m_points[3] = topLeft.orig;

	m_normals[0] = topRight.dir.cross( topLeft.dir );
	m_normals[1] = bottomRight.dir.cross( topRight.dir );
	m_normals[2] = bottomLeft.dir.cross( bottomRight.dir );
	m_normals[3] = topLeft.dir.cross( bottomLeft.dir );
}

bool RTFrustum::Intersect( const AABB &box ) const
{
	const vec3 boxes[8] = {vec3( box.min.x, box.min.y, box.min.z ),
						 vec3( box.max.x, box.max.y, box.max.z ),
						 vec3( box.min.x, box.min.y, box.max.z ),
						 vec3( box.min.x, box.max.y, box.min.z ),
						 vec3( box.min.x, box.max.y, box.max.z ),
						 vec3( box.max.x, box.min.y, box.min.z ),
						 vec3( box.max.x, box.min.y, box.max.z ),
						 vec3( box.max.x, box.max.y, box.min.z )};

	for ( unsigned int i = 0; i < 4; ++i )
	{
		int out_side = 0;

		const vec3 &pointPlane = m_points[i];
		const vec3 &normalPlane = m_normals[i];

		for ( unsigned int j = 0; j < 8; ++j )
		{
			const vec3 OP = pointPlane - boxes[j];
			const float dot = OP.dot( normalPlane );

			if ( dot < FLT_EPSILON )
			{
				out_side++;
			}
		}

		if ( out_side == 8 )
			return false;
	}

// 	if ( out_side == 4 )
// 		return true;

	return true;
}
