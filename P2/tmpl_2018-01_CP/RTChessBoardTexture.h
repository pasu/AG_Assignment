#pragma once
#include "RTTexture.h"

class RTChessBoardTexture : public RTTexture
{
  public:
	RTChessBoardTexture( const vec3 &color1, const vec3 &color2 );
	~RTChessBoardTexture();

	const vec3 getTexel( float s, float t, float z, const vec2 &scale = vec2( 1, 1 ) ) const;

  private:
	vec3 color1;
	vec3 color2;

	inline float modulo( const float x ) const
	{
		return x - std::floor( x );
	}
};
