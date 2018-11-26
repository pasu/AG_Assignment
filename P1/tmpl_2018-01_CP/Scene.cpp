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
}

Scene::~Scene()
{
	delete camera;

	for ( RTPrimitive* obj : primitivecollection )
	{
		delete obj;
		obj = NULL;
	}
	primitivecollection.clear();

	for ( RTLight *light : lightcollection )
	{
		delete light;
		light = NULL;
	}
	lightcollection.clear();
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
