#pragma once
#include "RTLight.h"
#include "RTPrimitive.h"
#include "RTCamera.h"
#include <vector>
#include "BVH.h"

using namespace std;

class Scene
{
public:
	Scene( const vec3 &ambientLight, const vec3 &backgroundColor );
	~Scene();

	void addObject( RTPrimitive *object );
	void addLight( RTLight *light );
	RTCamera* getCamera()const;

	inline const vector<RTPrimitive *> &getObjects() const { return primitivecollection; }
	inline const vector<RTLight *> &getLights() const { return lightcollection; }

	void ClearAllObj();
	void ClearAllLight();
	vec3 ambientLight;
	vec3 backgroundColor;
	unsigned int backgroundColorPixel;

	void BuildBVHTree();
	bool getIntersection( const RTRay &ray, RTIntersection &nearestIntersection ) const;
	RTIntersection findNearestObjectIntersection( const RTRay &ray ) const;

  private:
	vector<RTPrimitive *> primitivecollection;
	vector<RTLight *> lightcollection;
	RTCamera* camera;

	bool bInitializedBVH;
	BVH *bvhTree;
};
