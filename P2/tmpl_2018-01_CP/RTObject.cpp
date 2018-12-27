#include "precomp.h"
#include "RTObject.h"

RTObject::RTObject( RTGeometry *g ) : pGeometry( g ), animateFunc(animateFuncDefault)
{
	resetTransform();
}

void RTObject::updateAABBbounds()
{
	const AABB &geoAABB = pGeometry->getAABBBounds();
	bounds.min = geoAABB.min + pos;
	bounds.max = geoAABB.max + pos;
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
	speed = vec3( 0 );
	mModelRotate = mat4::identity();
}

void RTObject::translateGlobal( const vec3 v )
{
	pos = pos + v;
}

void RTObject::rotateLocal( vec3 axis, float angle )
{
	mat4 mRotation = mat4::rotate( axis, angle );
	mat4 temp = mRotation * mModelRotate;
	mModelRotate = temp;

}

void RTObject::animate()
{
	animateFunc( this );
}

void RTObject::animateFuncDefault( RTObject * )
{
}
