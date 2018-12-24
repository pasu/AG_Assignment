#include "precomp.h"
#include "RTPrimitive.h"

RTPrimitive::RTPrimitive( vec3 center, const RTMaterial &material )
	: pos( center ), material(material)
{
}

RTPrimitive::RTPrimitive( const RTMaterial &material ) : pos(vec3(0.0f)),material( material )
{
}

void RTPrimitive::getBounds( Vector3& vmin, Vector3& vmax, Vector3 v1, Vector3 v2, Vector3 v3, Vector3 v4, Vector3 v5, Vector3 v6, Vector3 v7, Vector3 v8 )
{
	vmin = vmax = v1;

	vmin = ::min( vmin, v2 );
	vmax = ::max( vmax, v2 );

	vmin = ::min( vmin, v3 );
	vmax = ::max( vmax, v3 );

	vmin = ::min( vmin, v4 );
	vmax = ::max( vmax, v4 );

	vmin = ::min( vmin, v5);
	vmax = ::max( vmax, v5 );

	vmin = ::min( vmin, v6 );
	vmax = ::max( vmax, v6 );

	vmin = ::min( vmin, v7 );
	vmax = ::max( vmax, v7 );

	vmin = ::min( vmin, v2 );
	vmax = ::max( vmax, v2 );
}
