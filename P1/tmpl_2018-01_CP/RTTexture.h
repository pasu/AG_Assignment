#pragma once
class RTTexture
{
  public:
	RTTexture();
	~RTTexture();

	const vec3 getTexel( float s, float t, const vec2 &scale = vec2( 1, 1 ) ) const;

	void RTTexture::LoadImage( char *a_File );

	Pixel *GetBuffer() { return m_Buffer; }
	void SetBuffer( Pixel *a_Buffer ) { m_Buffer = a_Buffer; }
	int GetWidth() { return m_Width; }
	int GetHeight() { return m_Height; }
	int GetPitch() { return m_Pitch; }
	void SetPitch( int a_Pitch ) { m_Pitch = a_Pitch; }

  private:
	const vec3 bilinearInterpolation( float u, float v ) const;
	const vec3 getTexelFromFile( const int x, const int y ) const;
  private:
	Pixel *m_Buffer;
	int m_Width, m_Height;
	int m_Pitch;
};
