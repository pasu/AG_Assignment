#include "precomp.h"
#include "Sampler.h"

const double DOU_PI = 2.0 * PI;

static RandomGenerator rng;
// HaltonSampler halton(0, 2);

vec3 sampleCosHemisphere( const vec3 &normal )
{
	vec3 u, v, w( normal );

	if ( std::fabs( w.x ) > std::fabs( w.y ) )
		u = normalize( cross( vec3( 0, 1, 0 ), w ) );
	else
		u = normalize( cross( vec3( 1, 0, 0 ), w ) );
	v = cross( w, u );

	double a = rng.Uniform1(), b = rng.Uniform1(), sini = std::sqrt( a ), cosi = DOU_PI * b;
	// double a = halton.get1D(), b = halton.get1D(), sini = std::sqrt(1-a*a), cosi = DOU_PI * b;
	// return normalize(normal + Vector3d(std::cos(cosi)*sini, std::sin(cosi)*sini, std::sqrt(a)));
	return normalize( ( sini * std::cos( cosi ) * u ) + ( sini * std::sin( cosi ) * v ) + ( std::sqrt( 1 - a ) * w ) );
}

vec3 sampleCosSphere( const vec3 &normal, double sin_, double cos_ )
{
	vec3 u, v, w( normal );

	if ( std::fabs( w.x ) > 0.1 )
		u = normalize( cross( vec3( 0, 1, 0 ), w ) );
	else
		u = normalize( cross( vec3( 1, 0, 0 ), w ) );
	v = normalize( cross( w, u ) );

	double a = rng.Uniform1(), phi = DOU_PI * a;

	return normalize( ( sin_ * std::cos( phi ) * u ) + ( sin_ * std::sin( phi ) * v ) + ( cos_ * w ) );
}
