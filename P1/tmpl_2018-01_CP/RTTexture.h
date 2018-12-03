#pragma once
class RTTexture
{
  public:
	RTTexture();
	virtual ~RTTexture();

	virtual const vec3 getTexel( float s, float t, float z, const vec2 &scale = vec2( 1, 1 ) ) const;

	void LoadTextureImage( const char *a_File );

	int GetWidth() { return m_Width; }
	int GetHeight() { return m_Height; }
	int GetPitch() { return m_Pitch; }
	void SetPitch( int a_Pitch ) { m_Pitch = a_Pitch; }

	void generateMipmap( int level, float mip0Distance );

  private:
	const vec3 bilinearInterpolation( float u, float v, int mip ) const;
	const vec3 getTexelFromFile( const int x, const int y, int mip ) const;
  private:
	vec3 *m_Buffer[8];
	int m_Width, m_Height;
	int m_Pitch;

	int mip_level;
	float mip0_distance;
};
