#include "precomp.h"
#include "RTTexture.h"


RTTexture::RTTexture()
{
}

RTTexture::~RTTexture()
{
}

const Tmpl8::vec3 RTTexture::getTexel( float s, float t, const vec2 &scale /*= vec2( 1, 1 ) */ ) const
{
	s *= scale.x;
	t *= scale.y;

	float wrappedS;
	float wrappedT;

	if ( s >= 0 )
		wrappedS = s - (int)s;
	else
		wrappedS = 1 - ( abs( s ) - (int)abs( s ) );

	if ( t >= 0 )
		wrappedT = t - (int)t;
	else
		wrappedT = 1 - ( abs( t ) - (int)abs( t ) );

	//return getTexelFromFile(wrappedS*(width-1), wrappedT*(height-1));
	return bilinearInterpolation( wrappedS, wrappedT );
}

const Tmpl8::vec3 RTTexture::bilinearInterpolation( float u, float v ) const
{
	const float pu = ( m_Width - 1 ) * u;
	const float pv = ( m_Height - 1 ) * v;
	const int x = (int)pu;
	const int y = (int)pv;
	const float uPrime = pu - x;
	const float vPrime = pv - y;

	const int xl = x - 1 >= 0 ? x - 1 : m_Width - 1;
	const int xr = x + 1 < m_Width ? x + 1 : 0;
	const int yb = y - 1 >= 0 ? y - 1 : m_Height - 1;
	const int yt = y + 1 < m_Height ? y + 1 : 0;

	return ( 1.0f - uPrime ) * ( 1.0f - vPrime ) * getTexelFromFile( xl, yb ) +
		   uPrime * ( 1.0f - vPrime ) * getTexelFromFile( xr, yb ) +
		   ( 1.0f - uPrime ) * vPrime * getTexelFromFile( xl, yt ) +
		   uPrime * vPrime * getTexelFromFile( xr, yt );
}

const Tmpl8::vec3 RTTexture::getTexelFromFile( const int x, const int y ) const
{
	int basePixel = ( x + ( ( ( m_Height - 1 ) - y ) * m_Width ) ) * m_Pitch;
	byte *data = (byte *)m_Buffer;
	return {data[basePixel] / 255.0f, data[basePixel + 1] / 255.0f, data[basePixel + 2] / 255.0f};
}

void RTTexture::LoadImage( char *a_File )
{
	FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;
	fif = FreeImage_GetFileType( a_File, 0 );
	if ( fif == FIF_UNKNOWN ) fif = FreeImage_GetFIFFromFilename( a_File );
	FIBITMAP *tmp = FreeImage_Load( fif, a_File );
	FIBITMAP *dib = FreeImage_ConvertTo32Bits( tmp );
	FreeImage_Unload( tmp );
	m_Width = m_Pitch = FreeImage_GetWidth( dib );
	m_Height = FreeImage_GetHeight( dib );
	m_Buffer = (Pixel *)MALLOC64( m_Width * m_Height * sizeof( Pixel ) );

	for ( int y = 0; y < m_Height; y++ )
	{
		unsigned char *line = FreeImage_GetScanLine( dib, m_Height - 1 - y );
		memcpy( m_Buffer + y * m_Pitch, line, m_Width * sizeof( Pixel ) );
	}
	FreeImage_Unload( dib );
}
