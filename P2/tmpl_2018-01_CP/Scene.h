#pragma once
#include "RTLight.h"
#include "RTPrimitive.h"
#include "RTCamera.h"
#include <vector>
#include "TopLevelBVH.h"
#include "RayPacket.h"
#include"RTObject.h"
#include "Sampler.h"
#include "uniform.h"

using namespace std;
struct Neighbor
{
	Neighbor()
	{
		sq_dis = -1.0;
		i = 0;
	};
	Neighbor( float s, unsigned e )
	{
		sq_dis = s;
		i = e;
	};
	// squared distance from photon to intersection
	float sq_dis;
	// index of photon in map
	unsigned i;
};

struct Photon
{
	Photon(){};
	Photon( char f )
	{
		flag = f;
	};
	Photon( vec3 i )
	{
		c = i;
	};
	// indicate the splitting axis or leaf node
	char flag;
	// position of photon
	vec3 p;
	// incident direction of photon ray
	vec3 dir;
	// intensity of photon
	vec3 c;
};

class Scene
{
  public:
	enum SampleType
	{
		uniform = 1
	};
  public:
	Scene( const vec3 &ambientLight, const vec3 &backgroundColor, Scene ::SampleType _type = Scene::uniform);
	~Scene();

    void addObject(RTPrimitive * primitive);
	void addObject( RTObject *object );
	void addLight( RTLight *light );
	void updateLightsWeight( );
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

	bool isOcclusion( const RTRay &ray, const float &distance ) const;
	void animate();

	Sampler *sampler()const;

	int getluckylight() const;

	vec3 RandomPointOnLight(RTLight*& pL)const;

	void AttachSkyDome(RTTexture* pTexture);

	vec3 getColor( const RTRay &ray )const;
  private:
	vector<RTObject *> objectcollection;// objects in the scene
	vector<RTLight *> lightcollection;
	vector<float> lightImportances;
	RTCamera* camera;

	bool bInitializedBVH;
	TopLevelBVH *bvhTree;

	std::shared_ptr<Sampler> sampler_;

	RTTexture *pSkyDome;
};
