#pragma once
#include "precomp.h"
#include "random.h"

class Sampler
{
  public:
	Sampler() {}

	virtual void startNextSample() {}

	virtual double get1D() = 0;

	virtual vec2 get2D() = 0;

	virtual ~Sampler() {}

  protected:
	RandomGenerator rng_;
};

vec3 sampleCosHemisphere( const vec3 &normal );

vec3 sampleCosSphere( const vec3 &normal, double sin_, double cos_ );
