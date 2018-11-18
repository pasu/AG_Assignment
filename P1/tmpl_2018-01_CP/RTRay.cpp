#include "precomp.h"
#include "RTRay.h"


RTRay::RTRay() : orig( 0 ), dir( 0, 0, -1 )
{
}

RTRay::RTRay( const vec3 &o, const vec3 &d ) : orig( o ), dir( normalize( d ) )
{
}
