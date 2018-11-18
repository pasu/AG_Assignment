#pragma once
#include "RTLight.h"
#include "RTPrimitive.h"
#include "RTCamera.h"
#include <vector>

using namespace std;

class Scene
{
public:
	Scene( const vec3 &ambientLight, const vec3 &backgroundColor );
	~Scene();

	void addObject( RTPrimitive *object );
	void addLight( RTLight *light );
	RTCamera* getCamera();

	vec3 ambientLight;
	vec3 backgroundColor;

  private:
	vector<RTPrimitive *> primitivecollection;
	vector<RTLight *> lightcollection;
	RTCamera* camera;
};
