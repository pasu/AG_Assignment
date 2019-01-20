#pragma once
#include "precomp.h"
#include "Sampler.h"

class uniform : public Sampler
{
  public:
	double get1D() override
	{
		return rng_.Uniform1();
	}

	vec2 get2D() override
	{
		return vec2( rng_.Uniform1(), rng_.Uniform1() );
	}
};

std::shared_ptr<Sampler> createUniformSampler();
