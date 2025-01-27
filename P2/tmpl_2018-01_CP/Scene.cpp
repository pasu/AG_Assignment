#include "precomp.h"
#include "Scene.h"


Scene::Scene( const vec3 &ambientLight, const vec3 &backgroundColor, Scene ::SampleType _type) : ambientLight( ambientLight ), backgroundColor( backgroundColor )
{
	switch ( _type )
	{
	case Scene::uniform:
		sampler_ = createUniformSampler();
		break;
	default:
		break;
	}

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
	pSkyDome = NULL;
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


void Scene::addObject( RTPrimitive *primitive )
{
}

void Scene::addObject( RTObject *object )
{
	objectcollection.push_back( object );
}

void Scene::addLight( RTLight *light )
{
	lightcollection.push_back( light );

	float _power = light->getPower();
	float _area = light->getArea();

	lightImportances.push_back( _power * _area );
}

void Scene::updateLightsWeight()
{
	float sum = 0.0;
	for (int i=0;i<lightImportances.size();i++)
	{
		sum += lightImportances[i];
	}

	for ( int i = 0; i < lightImportances.size(); i++ )
	{
		lightImportances[i] = lightImportances[i] / sum;
	}
}

RTCamera *Scene::getCamera()const
{
	return camera;
}

void Scene::ClearAllObj()
{
	for ( RTObject *obj : objectcollection )
	{
		delete obj;
		obj = NULL;
	}
	objectcollection.clear();
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

    bvhTree = new TopLevelBVH( objectcollection );

	bInitializedBVH = true;
}

void Scene::rebuildTopLevelBVH()
{
	bvhTree->rebuild();
}

bool Scene::getIntersection( const RTRay &ray, RTIntersection &nearestIntersection )const
{
	return bvhTree->getIntersection( ray, &nearestIntersection);
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
		static auto &objects = objectcollection;

		for ( auto it = objects.begin(); it != objects.end(); ++it )
		{
			RTIntersection intersection;
			bool intersect = ( *it )->getIntersection( ray,intersection );

			if ( intersect &&
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

bool Scene::isOcclusion( const RTRay &ray, const float &distance ) const
{
	RTIntersection nearestIntersection;
	bool bOcclusion = false;
	if ( bvhTree->getIntersection( ray, &nearestIntersection, true, distance ) 
		&& nearestIntersection.rayT<distance)
	{
		bOcclusion = true;
	}
	
	return bOcclusion;
}

void Scene::animate()
{
    for (RTObject *object : objectcollection) {
		object->animate();
    }
}

Sampler *Scene::sampler() const
{
	return sampler_.get();
}

int Scene::getluckylight() const
{
	float sum = 0.0f;
	float pivot = (float)rand() / RAND_MAX;
	int index = 0;

	for ( int i = 0; i < lightImportances.size(); i++ )
	{
		sum += lightImportances[i];
		if ( pivot <= sum )
		{
			index = i;
			break;
		}
	}
	return index;
}

Tmpl8::vec3 Scene::RandomPointOnLight( RTLight *&pL ) const
{
	int luckyL = getluckylight();
	pL = lightcollection[luckyL];
	return pL->getRandomPnt();
}

void Scene::AttachSkyDome( RTTexture *pTexture )
{
	pSkyDome = pTexture;
}

Tmpl8::vec3 Scene::getColor( const RTRay &ray )const
{
	if (pSkyDome)
	{
		float u = (0.5f + Utils::INV_PI * 0.5f * atan2(ray.dir.z, ray.dir.x));
		float v = 1.0f - (0.5f - Utils::INV_PI * asinf(ray.dir.y));
		return pSkyDome->getTexel( u, v, 2 );
	}

	return backgroundColor;
}
