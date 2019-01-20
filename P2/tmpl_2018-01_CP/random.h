#pragma once

#include <ctime>
#include <random>

class RandomGenerator
{
  public:
	RandomGenerator() : generator_( time( 0 ) ), distribution1_( 0, 1 ), distribution2_( -1, 1 ) {}

	double Uniform1() { return distribution1_( generator_ ); }

	double Uniform2() { return distribution2_( generator_ ); }

  private:
	std::default_random_engine generator_;
	std::uniform_real_distribution<double> distribution1_;
	std::uniform_real_distribution<double> distribution2_;
};
