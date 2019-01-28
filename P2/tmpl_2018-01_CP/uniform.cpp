#include "precomp.h"
#include "uniform.h"


std::shared_ptr<Sampler> createUniformSampler()
{
	return std::shared_ptr<Sampler>( new uniform() );
}
