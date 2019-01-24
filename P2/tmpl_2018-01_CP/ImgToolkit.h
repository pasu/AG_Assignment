#pragma once
class ImgToolkit
{
  public:
	ImgToolkit();
	~ImgToolkit();

	static void median_filter( int width, int height, vec3 *buffer, vec3 *result );
	static void recursive_bilateral_filter( float sigma_spatial, float sigma_range, int width, int height, vec3 *buffer, vec3 *result );
};
