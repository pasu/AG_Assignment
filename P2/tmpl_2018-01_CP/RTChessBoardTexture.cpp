#include "precomp.h"
#include "RTChessBoardTexture.h"


RTChessBoardTexture::RTChessBoardTexture( const vec3 &color1, const vec3 &color2 )
	:color1( color1 ), color2( color2 )
{
}

RTChessBoardTexture::~RTChessBoardTexture()
{
}

const vec3 RTChessBoardTexture::getTexel( float s, float t, float z, const vec2 &scale /*= vec2( 1, 1 ) */ ) const
{
	return ( ( modulo( s * scale.x ) < 0.5f ) ^ ( modulo( t * scale.y ) < 0.5f ) ) == 0 ? color1 : color2;
}
