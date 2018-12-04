#include "FXAA.h"
#include "precomp.h"

static vec3 tempPixels[1920 * 1080];
static vec3 *pixel_texture;
static int texture_width;
static int texture_height;

const float FXAA_REDUCE_MIN = 1.0 / 128.0;
const float FXAA_REDUCE_MUL = 1.0 / 8.0;
const float FXAA_SPAN_MAX = 8.0;

vec3 texture( vec2 texCoord )
{
	int x = (int)( texCoord.x + 0.5f );
	int y = (int)( texCoord.y + 0.5f );
	x = max( x, 0 );
	x = min( x, texture_width - 1 );
	y = max( y, 0 );
	y = min( y, texture_height - 1 );
	return pixel_texture[y * texture_width + x];
};

vec2 min_( vec2 a, vec2 b )
{
	return vec2( min( a.x, b.x ), min( a.y, b.y ) );
}

vec2 max_( vec2 a, vec2 b )
{
	return vec2( max( a.x, b.x ), max( a.y, b.y ) );
}

//SDL_Thread *thread = SDL_CreateThread( threadFunc, "TestThread", &threads.back() );
// SDL_WaitThread( threads.back().thread, &status );

int runFXAA_thread( void *interval )
{
	int x_min = ( (int *)interval )[0];
	int x_max = ( (int *)interval )[1];
	vec2 resolution = vec2( 1.0, 1.0 );

	for ( int y = 0; y < texture_height; y++ )
	{
		for ( int x = x_min; x < x_max; x++ )
		{
			vec2 texCoordVarying( (float)x, (float)y );
			vec3 rgbNW = texture( ( texCoordVarying + vec2( -1.0, -1.0 ) ) );
			vec3 rgbNE = texture( ( texCoordVarying + vec2( 1.0, -1.0 ) ) );
			vec3 rgbSW = texture( ( texCoordVarying + vec2( -1.0, 1.0 ) ) );
			vec3 rgbSE = texture( ( texCoordVarying + vec2( 1.0, 1.0 ) ) );
			vec3 rgbM = texture( texCoordVarying );

			float opacity = 1.0f;

			vec3 luma = vec3( 0.299, 0.587, 0.114 );

			float lumaNW = dot( rgbNW, luma );
			float lumaNE = dot( rgbNE, luma );
			float lumaSW = dot( rgbSW, luma );
			float lumaSE = dot( rgbSE, luma );
			float lumaM = dot( rgbM, luma );
			float lumaMin = min( lumaM, min( min( lumaNW, lumaNE ), min( lumaSW, lumaSE ) ) );
			float lumaMax = max( lumaM, max( max( lumaNW, lumaNE ), max( lumaSW, lumaSE ) ) );

			vec2 dir;
			dir.x = -( ( lumaNW + lumaNE ) - ( lumaSW + lumaSE ) );
			dir.y = ( ( lumaNW + lumaSW ) - ( lumaNE + lumaSE ) );

			float dirReduce = max( ( lumaNW + lumaNE + lumaSW + lumaSE ) * ( 0.25f * FXAA_REDUCE_MUL ), FXAA_REDUCE_MIN );

			float rcpDirMin = 1.0f / ( min( abs( dir.x ), abs( dir.y ) ) + dirReduce );
			dir = min_( vec2( FXAA_SPAN_MAX, FXAA_SPAN_MAX ),
						max_( vec2( -FXAA_SPAN_MAX, -FXAA_SPAN_MAX ),
							  dir * rcpDirMin ) ) *
				  resolution;

			vec3 rgbA = 0.5 * ( texture( texCoordVarying + dir * ( 1.0 / 3.0 - 0.5 ) ) +
								texture( texCoordVarying + dir * ( 2.0 / 3.0 - 0.5 ) ) );

			vec3 rgbB = rgbA * 0.5 + 0.25 * ( texture( texCoordVarying + dir * -0.5 ) +
											  texture( texCoordVarying + dir * 0.5 ) );

			float lumaB = dot( rgbB, luma );

			if ( ( lumaB < lumaMin ) || ( lumaB > lumaMax ) )
			{

				tempPixels[y * texture_width + x] = rgbA;
			}
			else
			{

				tempPixels[y * texture_width + x] = rgbB;
			}
		}
	}
	return 0;
}

void runFXAA( vec3 *pixels, int width, int height )
{
	texture_width = width;
	texture_height = height;
	pixel_texture = pixels;

	int intervals[NUMBER_THREAD][2];
	SDL_Thread *threads[NUMBER_THREAD];
	int size_chunk = width / NUMBER_THREAD;
	for ( int i = 0; i < NUMBER_THREAD; i++ )
	{
		intervals[i][0] = size_chunk * i;
		intervals[i][1] = size_chunk * ( i + 1 );
		threads[i] = SDL_CreateThread( runFXAA_thread, "", (void *)( intervals[i] ) );
	}

	for ( int i = 0; i < NUMBER_THREAD; i++ )
	{
		int status;
		SDL_WaitThread( threads[i], &status );
	}
	memcpy( pixels, tempPixels, sizeof( vec3 ) * width * height );
}
