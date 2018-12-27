#pragma once
#include "RTLight.h"
#include "RTPrimitive.h"
#include "RTCamera.h"
#include <vector>
#include "TopLevelBVH.h"
#include "RayPacket.h"
#include"RTObject.h"

using namespace std;

class Scene
{
public:
	Scene( const vec3 &ambientLight, const vec3 &backgroundColor );
	~Scene();

    void addObject(RTPrimitive * primitive);
	void addObject( RTObject *object );
	void addLight( RTLight *light );
	RTCamera* getCamera()const;

	inline const vector<RTLight *> &getLights() const { return lightcollection; }

	void ClearAllObj();
	void ClearAllLight();
	vec3 ambientLight;
	vec3 backgroundColor;
	unsigned int backgroundColorPixel;

	void BuildBVHTree();
	void rebuildTopLevelBVH();
	bool getIntersection( const RTRay &ray, RTIntersection &nearestIntersection ) const;
	RTIntersection findNearestObjectIntersection( const RTRay &ray ) const;
	void findNearestObjectIntersection( const RayPacket &raypacket, RTIntersection *intersections ) const;
	void animate();
  private:
	vector<RTObject *> objectcollection;// objects in the scene
	vector<RTLight *> lightcollection;
	RTCamera* camera;

	bool bInitializedBVH;
	TopLevelBVH *bvhTree;


};
