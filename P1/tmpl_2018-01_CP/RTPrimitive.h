#pragma once
#include "RTRay.h"
class RTIntersection;

class RTPrimitive
{
  public:
	RTPrimitive( vec3 center );
	virtual const RTIntersection intersect( const RTRay &ray )const = 0;

  private:
	vec3 pos;
};
