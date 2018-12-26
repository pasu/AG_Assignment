#include "precomp.h"
#include "Scene.h"


Scene::Scene( const vec3 &ambientLight, const vec3 &backgroundColor ) : ambientLight( ambientLight ), backgroundColor( backgroundColor )
{
	camera = new RTCamera;

	unsigned int r = backgroundColor.x * 255;
	unsigned int g = backgroundColor.y * 255;
	unsigned int b = backgroundColor.z * 255;
	int ro = r << 16;
	int go = g << 8;
	int bo = b;
	backgroundColorPixel = ro+go+bo;

	bInitializedBVH = false;
	bvhTree = NULL;
}

Scene::~Scene()
{
	delete camera;

	ClearAllLight();
	ClearAllObj();

	if (bvhTree)
	{
		delete bvhTree;
		bvhTree = NULL;
	}
}

void Scene::addObject( RTPrimitive *object )
{
	primitivecollection.push_back( object );
}

void Scene::addLight( RTLight *light )
{
	lightcollection.push_back( light );
}

RTCamera *Scene::getCamera()const
{
	return camera;
}

void Scene::ClearAllObj()
{
	for ( RTPrimitive *obj : primitivecollection )
	{
		delete obj;
		obj = NULL;
	}
	primitivecollection.clear();
}

void Scene::ClearAllLight()
{
	for ( RTLight *light : lightcollection )
	{
		delete light;
		light = NULL;
	}
	lightcollection.clear();
}

void Scene::BuildBVHTree()
{
	if ( bvhTree )
	{
		delete bvhTree;
	}

	bvhTree = new BVH( &primitivecollection );
	bInitializedBVH = true;
}

bool Scene::getIntersection( const RTRay &ray, RTIntersection &nearestIntersection )const
{
	return bvhTree->getIntersection( ray, &nearestIntersection,false);
}

RTIntersection Scene::findNearestObjectIntersection( const RTRay &ray ) const
{
	RTIntersection nearestIntersection;

	if (bInitializedBVH)
	{
		getIntersection( ray, nearestIntersection );
	}
	else
	{
		static auto &objects = primitivecollection;

		for ( auto it = objects.begin(); it != objects.end(); ++it )
		{
			const RTIntersection &intersection = ( *it )->intersect( ray );

			if ( intersection.isIntersecting() &&
				 ( !nearestIntersection.isIntersecting() || intersection.rayT < nearestIntersection.rayT ) )
			{
				nearestIntersection = intersection;
			}
		}
	}
	
	return nearestIntersection;
}

void Scene::findNearestObjectIntersection( const RayPacket &raypacket, RTIntersection *intersections ) const
{
	if ( bInitializedBVH )
	{
		bvhTree->getIntersection( raypacket, intersections );
	}
}
