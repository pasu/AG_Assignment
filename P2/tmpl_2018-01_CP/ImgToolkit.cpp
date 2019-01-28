#include "precomp.h"
#include "ImgToolkit.h"


ImgToolkit::ImgToolkit()
{
}

ImgToolkit::~ImgToolkit()
{
}

void ImgToolkit::median_filter( int width, int height, vec3 *buffer, vec3 *rbuffer )
{
#pragma omp parallel for
	for ( int y = 1; y < height - 1; y++ )
	{
#pragma omp parallel for
		for ( int x = 1; x < width - 1; x++ )
		{
			int k = 0;
			vec3 mask[9];
			for ( int yy = y - 1; yy <= y + 1; ++yy )
			{
				for ( int xx = x - 1; xx <= x + 1; ++xx )
				{
					mask[k] = buffer[yy * width + xx];
					k++;
				}
			}
			vec3 result;
			std::sort( &mask[0], &mask[8], [=]( vec3 a, vec3 b ) { return a.x < b.x; } );
			result.x = mask[4].x;
			std::sort( &mask[0], &mask[8], [=]( vec3 a, vec3 b ) { return a.y < b.y; } );
			result.y = mask[4].y;
			std::sort( &mask[0], &mask[8], [=]( vec3 a, vec3 b ) { return a.z < b.z; } );
			result.z = mask[4].z;
			rbuffer[y * width + x] = result;
		}
	}
}


void ImgToolkit::barrel_distortion( int width, int height, vec3 *buffer, vec3 *result )
{
	float alpha = 1.5;
	for ( int y = 0; y < height; ++y )
	{
		for ( int x = 0; x < width; ++x )
		{
			float yy = (float)y / height;
			float xx = (float)x / width;
			vec2 r = vec2( xx - 0.5, yy - 0.5 );
			float r_judge = r.dot( r );
			vec2 nr = r * ( 1 + alpha * r_judge );
			if ( nr.x <= 0.5 && nr.y <= 0.5 )
			{
				int x2 = (int)( ( nr.x + 0.5 ) * width );
				int y2 = (int)( ( nr.y + 0.5 ) * height );
				if ( x2 >= 0 && x2 <= width && y2 >= 0 && y2 <= height )
				{
					result[y*width+x] += buffer[y2*width+x2];
				}
			}
		}
	}
}

#define QX_DEF_CHAR_MAX 255

inline void _recursive_bf(
	unsigned char *img,
	float sigma_spatial, float sigma_range,
	int width, int height, int channel,
	float *buffer = 0 )
{
	const int width_height = width * height;
	const int width_channel = width * channel;
	const int width_height_channel = width * height * channel;

	bool is_buffer_internal = ( buffer == 0 );
	if ( is_buffer_internal )
		buffer = new float[( width_height_channel + width_height + width_channel + width ) * 2];

	float *img_out_f = buffer;
	float *img_temp = &img_out_f[width_height_channel];
	float *map_factor_a = &img_temp[width_height_channel];
	float *map_factor_b = &map_factor_a[width_height];
	float *slice_factor_a = &map_factor_b[width_height];
	float *slice_factor_b = &slice_factor_a[width_channel];
	float *line_factor_a = &slice_factor_b[width_channel];
	float *line_factor_b = &line_factor_a[width];

	//compute a lookup table
	float range_table[QX_DEF_CHAR_MAX + 1];
	float inv_sigma_range = 1.0f / ( sigma_range * QX_DEF_CHAR_MAX );
	for ( int i = 0; i <= QX_DEF_CHAR_MAX; i++ )
		range_table[i] = static_cast<float>( exp( -i * inv_sigma_range ) );

	float alpha = static_cast<float>( exp( -sqrt( 2.0 ) / ( sigma_spatial * width ) ) );
	float ypr, ypg, ypb, ycr, ycg, ycb;
	float fp, fc;
	float inv_alpha_ = 1 - alpha;
	for ( int y = 0; y < height; y++ )
	{
		float *temp_x = &img_temp[y * width_channel];
		unsigned char *in_x = &img[y * width_channel];
		unsigned char *texture_x = &img[y * width_channel];
		*temp_x++ = ypr = *in_x++;
		*temp_x++ = ypg = *in_x++;
		*temp_x++ = ypb = *in_x++;
		unsigned char tpr = *texture_x++;
		unsigned char tpg = *texture_x++;
		unsigned char tpb = *texture_x++;

		float *temp_factor_x = &map_factor_a[y * width];
		*temp_factor_x++ = fp = 1;

		// from left to right
		for ( int x = 1; x < width; x++ )
		{
			unsigned char tcr = *texture_x++;
			unsigned char tcg = *texture_x++;
			unsigned char tcb = *texture_x++;
			unsigned char dr = abs( tcr - tpr );
			unsigned char dg = abs( tcg - tpg );
			unsigned char db = abs( tcb - tpb );
			int range_dist = ( ( ( dr << 1 ) + dg + db ) >> 2 );
			float weight = range_table[range_dist];
			float alpha_ = weight * alpha;
			*temp_x++ = ycr = inv_alpha_ * ( *in_x++ ) + alpha_ * ypr;
			*temp_x++ = ycg = inv_alpha_ * ( *in_x++ ) + alpha_ * ypg;
			*temp_x++ = ycb = inv_alpha_ * ( *in_x++ ) + alpha_ * ypb;
			tpr = tcr;
			tpg = tcg;
			tpb = tcb;
			ypr = ycr;
			ypg = ycg;
			ypb = ycb;
			*temp_factor_x++ = fc = inv_alpha_ + alpha_ * fp;
			fp = fc;
		}
		*--temp_x;
		*temp_x = 0.5f * ( ( *temp_x ) + ( *--in_x ) );
		*--temp_x;
		*temp_x = 0.5f * ( ( *temp_x ) + ( *--in_x ) );
		*--temp_x;
		*temp_x = 0.5f * ( ( *temp_x ) + ( *--in_x ) );
		tpr = *--texture_x;
		tpg = *--texture_x;
		tpb = *--texture_x;
		ypr = *in_x;
		ypg = *in_x;
		ypb = *in_x;

		*--temp_factor_x;
		*temp_factor_x = 0.5f * ( ( *temp_factor_x ) + 1 );
		fp = 1;

		// from right to left
		for ( int x = width - 2; x >= 0; x-- )
		{
			unsigned char tcr = *--texture_x;
			unsigned char tcg = *--texture_x;
			unsigned char tcb = *--texture_x;
			unsigned char dr = abs( tcr - tpr );
			unsigned char dg = abs( tcg - tpg );
			unsigned char db = abs( tcb - tpb );
			int range_dist = ( ( ( dr << 1 ) + dg + db ) >> 2 );
			float weight = range_table[range_dist];
			float alpha_ = weight * alpha;

			ycr = inv_alpha_ * ( *--in_x ) + alpha_ * ypr;
			ycg = inv_alpha_ * ( *--in_x ) + alpha_ * ypg;
			ycb = inv_alpha_ * ( *--in_x ) + alpha_ * ypb;
			*--temp_x;
			*temp_x = 0.5f * ( ( *temp_x ) + ycr );
			*--temp_x;
			*temp_x = 0.5f * ( ( *temp_x ) + ycg );
			*--temp_x;
			*temp_x = 0.5f * ( ( *temp_x ) + ycb );
			tpr = tcr;
			tpg = tcg;
			tpb = tcb;
			ypr = ycr;
			ypg = ycg;
			ypb = ycb;

			fc = inv_alpha_ + alpha_ * fp;
			*--temp_factor_x;
			*temp_factor_x = 0.5f * ( ( *temp_factor_x ) + fc );
			fp = fc;
		}
	}
	alpha = static_cast<float>( exp( -sqrt( 2.0 ) / ( sigma_spatial * height ) ) );
	inv_alpha_ = 1 - alpha;
	float *ycy, *ypy, *xcy;
	unsigned char *tcy, *tpy;
	memcpy( img_out_f, img_temp, sizeof( float ) * width_channel );

	float *in_factor = map_factor_a;
	float *ycf, *ypf, *xcf;
	memcpy( map_factor_b, in_factor, sizeof( float ) * width );
	for ( int y = 1; y < height; y++ )
	{
		tpy = &img[( y - 1 ) * width_channel];
		tcy = &img[y * width_channel];
		xcy = &img_temp[y * width_channel];
		ypy = &img_out_f[( y - 1 ) * width_channel];
		ycy = &img_out_f[y * width_channel];

		xcf = &in_factor[y * width];
		ypf = &map_factor_b[( y - 1 ) * width];
		ycf = &map_factor_b[y * width];
		for ( int x = 0; x < width; x++ )
		{
			unsigned char dr = abs( ( *tcy++ ) - ( *tpy++ ) );
			unsigned char dg = abs( ( *tcy++ ) - ( *tpy++ ) );
			unsigned char db = abs( ( *tcy++ ) - ( *tpy++ ) );
			int range_dist = ( ( ( dr << 1 ) + dg + db ) >> 2 );
			float weight = range_table[range_dist];
			float alpha_ = weight * alpha;
			for ( int c = 0; c < channel; c++ )
				*ycy++ = inv_alpha_ * ( *xcy++ ) + alpha_ * ( *ypy++ );
			*ycf++ = inv_alpha_ * ( *xcf++ ) + alpha_ * ( *ypf++ );
		}
	}
	int h1 = height - 1;
	ycf = line_factor_a;
	ypf = line_factor_b;
	memcpy( ypf, &in_factor[h1 * width], sizeof( float ) * width );
	for ( int x = 0; x < width; x++ )
		map_factor_b[h1 * width + x] = 0.5f * ( map_factor_b[h1 * width + x] + ypf[x] );

	ycy = slice_factor_a;
	ypy = slice_factor_b;
	memcpy( ypy, &img_temp[h1 * width_channel], sizeof( float ) * width_channel );
	int k = 0;
	for ( int x = 0; x < width; x++ )
	{
		for ( int c = 0; c < channel; c++ )
		{
			int idx = ( h1 * width + x ) * channel + c;
			img_out_f[idx] = 0.5f * ( img_out_f[idx] + ypy[k++] ) / map_factor_b[h1 * width + x];
		}
	}

	for ( int y = h1 - 1; y >= 0; y-- )
	{
		tpy = &img[( y + 1 ) * width_channel];
		tcy = &img[y * width_channel];
		xcy = &img_temp[y * width_channel];
		float *ycy_ = ycy;
		float *ypy_ = ypy;
		float *out_ = &img_out_f[y * width_channel];

		xcf = &in_factor[y * width];
		float *ycf_ = ycf;
		float *ypf_ = ypf;
		float *factor_ = &map_factor_b[y * width];
		for ( int x = 0; x < width; x++ )
		{
			unsigned char dr = abs( ( *tcy++ ) - ( *tpy++ ) );
			unsigned char dg = abs( ( *tcy++ ) - ( *tpy++ ) );
			unsigned char db = abs( ( *tcy++ ) - ( *tpy++ ) );
			int range_dist = ( ( ( dr << 1 ) + dg + db ) >> 2 );
			float weight = range_table[range_dist];
			float alpha_ = weight * alpha;

			float fcc = inv_alpha_ * ( *xcf++ ) + alpha_ * ( *ypf_++ );
			*ycf_++ = fcc;
			*factor_ = 0.5f * ( *factor_ + fcc );

			for ( int c = 0; c < channel; c++ )
			{
				float ycc = inv_alpha_ * ( *xcy++ ) + alpha_ * ( *ypy_++ );
				*ycy_++ = ycc;
				*out_ = 0.5f * ( *out_ + ycc ) / ( *factor_ );
				*out_++;
			}
			*factor_++;
		}
		memcpy( ypy, ycy, sizeof( float ) * width_channel );
		memcpy( ypf, ycf, sizeof( float ) * width );
	}

	for ( int i = 0; i < width_height_channel; ++i )
		img[i] = static_cast<unsigned char>( img_out_f[i] );

	if ( is_buffer_internal )
		delete[] buffer;
}

void ImgToolkit::recursive_bilateral_filter( float sigma_spatial, float sigma_range, int width, int height, vec3 *buffer, vec3 *result )
{
	unsigned char *img = new unsigned char[width * height * 3];

	for ( int y = 0; y < height; y++ )
	{
		for ( int x = 0; x < width; x++ )
		{
			img[y * width * 3 + x * 3] = 255 * std::min( 1.0f, std::max( 0.0f, (float)buffer[y * width + x].x ) );
			img[y * width * 3 + x * 3 + 1] = 255 * std::min( 1.0f, std::max( 0.0f, (float)buffer[y * width + x].y ) );
			img[y * width * 3 + x * 3 + 2] = 255 * std::min( 1.0f, std::max( 0.0f, (float)buffer[y * width + x].z ) );
		}
	}

	_recursive_bf( img, sigma_spatial ,sigma_range,width,height,3);

	for ( int y = 0; y < height; y++ )
	{
		for ( int x = 0; x < width; x++ )
		{
			result[y * width + x].x = (float)img[y * width * 3 + x * 3] /255.0f;
			result[y * width + x].y = (float)img[y * width * 3 + x * 3 + 1] / 255.0f;
			result[y * width + x].z = (float)img[y * width * 3 + x * 3 + 2] / 255.0f;

			// 			result[y * width + x].x = buffer[y * width + x].x;
// 			result[y * width + x].y = buffer[y * width + x].y;
// 			result[y * width + x].z = buffer[y * width + x].z;
		}
	}

	delete[] img;
	img = NULL;
}



