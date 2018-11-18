#include "precomp.h"
#include "Scene.h"


Scene::Scene( const vec3 &ambientLight, const vec3 &backgroundColor ) : ambientLight( ambientLight ), backgroundColor( backgroundColor )
{
	camera = new RTCamera;

	int r = backgroundColor.x * 255;
	int g = backgroundColor.x * 255;
	int b = backgroundColor.x * 255;
	backgroundColorPixel = r << 16 + g << 8 + b;
}

Scene::~Scene()
{
	delete camera;
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
