#include "precomp.h"
#include "RTIntersection.h"


RTIntersection::RTIntersection() : ray( 0 ), object( 0 ), rayT( -1 )
{
}

 RTIntersection::RTIntersection( const RTRay *ray, const RTPrimitive *object, const float rayT ) : ray( ray ), object( object ), rayT(rayT)
{
}
