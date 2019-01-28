#include "precomp.h"
#include "RTObject.h"

RTObject::RTObject( RTGeometry *g ) : pGeometry( g ), animateFunc(animateFuncDefault)
{
	resetTransform();
}

void RTObject::updateAABBbounds()
{
	const AABB &geoAABB = pGeometry->getAABBBounds();
	bounds.min = Vector3( 9999999.9f );
	bounds.max = Vector3(-9999999.9f);

    for (int i = 0; i < 8; i++) {
		vec3 v = vec3( geoAABB.min.x * ( ( i & 0b100 ) >> 2 ) + geoAABB.max.x * ( 1 - ( ( i & 0b100 ) >> 2 ) ),
					   geoAABB.min.y * ( ( i & 0b010 ) >> 1 ) + geoAABB.max.y * ( 1 - ( ( i & 0b010 ) >> 1 ) ),
					   geoAABB.min.z * ( ( i & 0b001 ) >> 0 ) + geoAABB.max.z * ( 1 - ( ( i & 0b001 ) >> 0 ) ) );
		v = mModelRotate * v + pos;
		Vector3 vv = Vector3( v.x, v.y, v.z );
		bounds.min = min( vv, bounds.min );
		bounds.max = max( vv, bounds.max );
    }
}

bool RTObject::getIntersection( const RTRay &ray, RTIntersection &nearestIntersection, bool occlusion, const float &distance ) const
{
	RTRay localRay;// translate the ray into local coordinate
    
	localRay.orig = mViewRotate*(ray.orig - pos);
	
	localRay.dir = mViewRotate * ray.dir;
	localRay.distance_traveled = ray.distance_traveled; // for looking up in mip map table

    // invoking BVH traversal
	bool intersect = pGeometry->getIntersection( localRay, nearestIntersection, occlusion, distance );

    

    // handle intersection surface data if intersecting
    if (intersect) {
        // get the surface data with respect to local coordinate
		nearestIntersection.surfacePointData = nearestIntersection.object->getSurfacePointData( nearestIntersection );

        // transform the syrface data into global coordinate
        nearestIntersection.surfacePointData.position = mModelRotate*nearestIntersection.surfacePointData.position + pos;
		nearestIntersection.surfacePointData.normal = mModelRotate * nearestIntersection.surfacePointData.normal;
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
	mViewRotate = mModelRotate;
	rotateAxis = vec3( 1, 0, 0 );
	rotateSpeed = .0f;
}

void RTObject::translateGlobal( const vec3 v )
{
	pos = pos + v;
}

void RTObject::rotateLocal( vec3 axis, float angle )
{
	mat4 mRotation = mat4::rotate( axis, angle );
	mat4 temp = mModelRotate*mRotation;
	mModelRotate = temp;
	mViewRotate = mModelRotate;
	mViewRotate.invert();
}

// invoke animate callback
void RTObject::animate()
{
	animateFunc( this );
}

void RTObject::animateFuncDefault( RTObject * )
{
}
