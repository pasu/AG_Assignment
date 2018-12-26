#pragma once
#ifndef __RT_OBJECT__
#define __RT_OBJECT__

#include "precomp.h"
#include"RTPrimitive.h"
#include"BVH.h"
class RTObject
{
  public:
	void BuildBVHTree();
  private:
	vector<RTPrimitive *> primitivecollection;

	BVH *bvhTree;

    // rigid transform
    mat4 mViewRotate;
	vec3 pos;

};

#endif