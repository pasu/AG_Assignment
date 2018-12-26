#include "precomp.h"
#include "RTObject.h"

RTObject::RTObject( RTGeometry *g ) : pGeometry( g )
{
	resetTransform();
}

void RTObject::updateAABBbounds()
{
}

bool RTObject::getIntersection( const RTRay &ray, RTIntersection &nearestIntersection ) const
{
	RTRay localRay;// translate the ray into local coordinate
	localRay.orig = ray.orig - pos;
	localRay.dir = ray.dir;
	localRay.distance_traveled = ray.distance_traveled; // for looking up in mip map table

    // invoking BVH traversal
	bool intersect =  pGeometry->getIntersection( localRay, nearestIntersection );

    

    // handle intersection surface data if intersecting
    if (intersect) {
        // get the surface data with respect to local coordinate
		nearestIntersection.surfacePointData = nearestIntersection.object->getSurfacePointData( nearestIntersection );

        // transform the syrface data into global coordinate
        nearestIntersection.surfacePointData.position = nearestIntersection.surfacePointData.position + pos;

        // override localRay with ray
		nearestIntersection.ray = &ray;
    }

    return intersect;
}

void RTObject::resetTransform()
{
	pos = vec3( 0 );
	mViewRotate = mat4::identity();
}

void RTObject::translate( const vec3 v )
{
	pos = pos + v;
}
