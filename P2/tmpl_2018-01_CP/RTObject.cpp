#include "precomp.h"
#include "RTObject.h"

void RTObject::BuildBVHTree()
{
	bvhTree = new BVH( &primitivecollection );
}
