#include "RTTexture.h"
#include "precomp.h"

RTTexture::RTTexture()
{
	for ( int i = 0; i < 8; i++ )
	{
		m_Buffer[i] = NULL;
	}
}

RTTexture::~RTTexture()
{
	for (int i=0;i<8;i++)
	{
		if (m_Buffer[i])
		{
			_aligned_free( m_Buffer[i] );
			m_Buffer[i] = NULL;
		}
	}
}

const Tmpl8::vec3 RTTexture::getTexel( float s, float t,float z, const vec2 &scale /*= vec2( 1, 1 ) */ ) const
{
	s *= scale.x;
	t *= scale.y;
	
	int mip = (int)(std::log((z / mip0_distance))/std::log(2))+1;

	mip = std::max( mip, 0 );
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
	return bilinearInterpolation( wrappedS, wrappedT ,mip);
}

const Tmpl8::vec3 RTTexture::bilinearInterpolation( float u, float v, int mip ) const
{
	mip = std::min( mip, mip_level-1 );
	int width = m_Width >> mip;
	int height = m_Height >> mip;
	const float pu = ( width - 1 ) * u;
	const float pv = ( height - 1 ) * v;
	const int x = (int)pu;
	const int y = (int)pv;
	const float uPrime = pu - x;
	const float vPrime = pv - y;

	const int xl = x - 1 >= 0 ? x - 1 : width - 1;
	const int xr = x + 1 < width ? x + 1 : 0;
	const int yb = y - 1 >= 0 ? y - 1 : height - 1;
	const int yt = y + 1 < height ? y + 1 : 0;

	return ( 1.0f - uPrime ) * ( 1.0f - vPrime ) * getTexelFromFile( xl, yb ,mip) +
		   uPrime * ( 1.0f - vPrime ) * getTexelFromFile( xr, yb,mip ) +
		   ( 1.0f - uPrime ) * vPrime * getTexelFromFile( xl, yt,mip ) +
		   uPrime * vPrime * getTexelFromFile( xr, yt,mip );
}

const Tmpl8::vec3 RTTexture::getTexelFromFile( const int x, const int y, int mip ) const
{
	int basePixel = ( x + ( ( ( (m_Height>>mip) - 1 ) - y ) * (m_Width>>mip) ) );
	return m_Buffer[mip][basePixel];
}

void RTTexture::LoadTextureImage( const char *a_File )
{
	FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;
	fif = FreeImage_GetFileType( a_File, 0 );
	if ( fif == FIF_UNKNOWN ) fif = FreeImage_GetFIFFromFilename( a_File );
	FIBITMAP *tmp = FreeImage_Load( fif, a_File );
	FIBITMAP *dib = FreeImage_ConvertTo32Bits( tmp );
	FreeImage_Unload( tmp );
	m_Width = m_Pitch = FreeImage_GetWidth( dib );
	m_Height = FreeImage_GetHeight( dib );
	m_Buffer[0] = (vec3 *)MALLOC64( m_Width * m_Height * sizeof( vec3 ) );

	for ( int y = 0; y < m_Height; y++ )
	{
		unsigned char *line = FreeImage_GetScanLine( dib, m_Height - 1 - y );
		for ( int x = 0; x < m_Width; x++ )
		{
			int basePixel = x * 4;
			unsigned char *data = (byte *)line;
			m_Buffer[0][y * m_Width + x] = {data[basePixel + 2] / 255.0f, data[basePixel + 1] / 255.0f, data[basePixel] / 255.0f};
		}
	}
	FreeImage_Unload( dib );
	mip_level = 1;
	mip0_distance = 10000.0f;
}

void RTTexture::generateMipmap( int level, float mip0Distance )
{
	mip_level = std::min( level, 8 );
	mip0_distance = mip0Distance;
	for ( int mip = 1; mip < mip_level; mip++ )
	{
		int currentWidth = m_Width >> mip;
		int currentHeight = m_Height >> mip;
		m_Buffer[mip] = (vec3 *)MALLOC64( currentWidth * currentHeight * sizeof( vec3 ) );
		for ( int y = 0; y < currentHeight; y++ )
		{
			//{lasty, lastx} = coordinate in (mip-1) level map
			int lasty = y *2;
			int lastWidth = currentWidth *2;
			vec3 *current = m_Buffer[mip];
			vec3 *last = m_Buffer[mip - 1];
			for ( int x = 0; x < currentWidth; x++ )
			{
				int lastx = x << 1;
				current[y * currentWidth + x] = ( last[lasty * lastWidth + lastx] + last[lasty * lastWidth + lastx + 1] +
												  last[( lasty + 1 ) * lastWidth + lastx] + last[( lasty + 1 ) * lastWidth + lastx + 1] ) *
												0.25f;
			}
		}
	}
}