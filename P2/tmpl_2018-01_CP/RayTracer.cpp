#include "RayTracer.h"
#include "ImgToolkit.h"
#include "Sampler.h"
#include "fxaa.h"
#include "precomp.h"
#include "ImgToolkit.h"
RayTracer::RayTracer( const Scene &scene, const RenderOptions &renderOptions ) : renderOptions( renderOptions ), scene( scene )
{
	size = renderOptions.width * renderOptions.height;
	pPixels = new unsigned int[size];
	pImgPixels = new vec3[size];
	hdrPixels = new vec3[size];

	fOnePixelSize = 2.0f / renderOptions.width;
	fStratificationSize = fOnePixelSize / SAMPLE_NUM;
	sample_count = 0;
	bFilter = false;
	nFilterType = 0;
	bProcessed = false;

	mMotion = false;
	frame_num = 4;

	pixels = new vec3[frame_num * size];
}

RayTracer::~RayTracer()
{
	if ( pPixels )
	{
		delete[] pPixels;
		pPixels = NULL;
	}

	if ( pImgPixels )
	{
		delete[] pImgPixels;
		pImgPixels = NULL;
	}

	if ( hdrPixels )
	{
		delete[] hdrPixels;
		hdrPixels = NULL;
	}
}

void RayTracer::traceChunk( int x_min, int x_max, int y_min, int y_max )
{
#ifdef BVH_PARTITION_TRAVERSAL_
	for ( int y = y_min; y <= y_max; y += RAYPACKET_DIM )
	{
		for ( int x = x_min; x <= x_max; x += RAYPACKET_DIM )
		{
			RayPacket rp( *this, vec2( x, y ) );

			vec3 colors[RAYPACKET_RAYS_PER_PACKET];
			this->castRayPacket( rp, colors );

			for ( unsigned int y0 = 0; y0 < RAYPACKET_DIM; ++y0 )
			{
				for ( unsigned int x0 = 0; x0 < RAYPACKET_DIM; ++x0 )
				{
					hdrPixels[( y + y0 ) * renderOptions.width + ( x + x0 )] = colors[y0 * RAYPACKET_DIM + x0];
				}
			}
		}
	}
#else
#ifdef PATH_TRACER
	int size = renderOptions.width * renderOptions.height;
	// path tracer
#pragma omp parallel for
	for ( int y = y_min; y <= y_max; ++y )
	{
#pragma omp parallel for
		for ( int x = x_min; x <= x_max; ++x )
		{
			if ( sample_count < SAMPLE_NUM2 )
			{
				RTRay r = generatePrimaryRayD( x, y, sample_count );
				RTIntersection intersection;
				vec3 color = sample( r, 0, intersection, true );
				color.x = sqrtf( color.x );
				color.y = sqrtf( color.y );
				color.z = sqrtf( color.z );
				if ( mMotion )
				{
					int nframeId = sample_count % frame_num;
					pixels[nframeId * size + y * renderOptions.width + x] = color;
				}
				else
				{
					hdrPixels[y * renderOptions.width + x] += color;
				}
				
			}
		}
	}
	sample_count++;
#else
	for ( int y = y_min; y <= y_max; ++y )
	{
		for ( int x = x_min; x <= x_max; ++x )
		{
			RTRay r = generatePrimaryRay( x, y );
			RTIntersection intersection;
			vec3 color = castRay( r, 0, intersection );
			hdrPixels[y * renderOptions.width + x] = color;
		}
	}
#endif // PATH_TRACER

#endif
}

void RayTracer::render( Surface *screen )
{
	scene.getCamera()->Update();

	//ImgToolkit::recursive_bilateral_filter( 0.03f, 0.1f, renderOptions.width, renderOptions.height, hdrPixels, pImgPixels );

#if NUMBER_THREAD == 1
	traceChunk( 0, renderOptions.width - 1, 0, renderOptions.height - 1 );
#else
	int chunkWidth = renderOptions.width / NUMBER_THREAD;
	for ( int i = 0; i < NUMBER_THREAD; i++ )
	{
		int x_min = chunkWidth * i;
		int x_max = std::min( x_min + chunkWidth, renderOptions.width ) - 1;
		startRenderThread( x_min, x_max, 0, renderOptions.height - 1 );
	}

	waitRenderThreads();
#endif
	//runFXAA( hdrPixels, renderOptions.width, renderOptions.height );
	int nS = sample_count;
	if ( mMotion )
	{
		nS = sample_count > 3 ? 3 : sample_count;
		for ( int y = 0; y < renderOptions.height; ++y )
		{
			for ( int x = 0; x < renderOptions.width; ++x )
			{
				hdrPixels[y * renderOptions.width + x] =
					(pixels[y * renderOptions.width + x] +
					pixels[renderOptions.width*renderOptions.height + y * renderOptions.width + x] +
					pixels[renderOptions.width * renderOptions.height * 2 + y * renderOptions.width + x]);
			}
		}
	}

	if ( bFilter )
	{
		if ( !bProcessed )
		{
			for ( int y = 0; y < renderOptions.height; ++y )
			{
				for ( int x = 0; x < renderOptions.width; ++x )
				{
#ifdef PATH_TRACER
					hdrPixels[y * renderOptions.width + x] = hdrPixels[y * renderOptions.width + x] * vec3( 1.0f / (float)( nS > SAMPLE_NUM2 ? SAMPLE_NUM2 : nS ) );
#endif // PATH_TRACER
				}
			}

			switch ( nFilterType )
			{
			case 1:
				ImgToolkit::recursive_bilateral_filter(0.03f,0.1f, renderOptions.width, renderOptions.height, hdrPixels, pImgPixels );
				break;
			case 2:
				ImgToolkit::median_filter( renderOptions.width, renderOptions.height, hdrPixels, pImgPixels );
				break;
			case 3:
				ImgToolkit::barrel_distortion( renderOptions.width, renderOptions.height, hdrPixels, pImgPixels );
				break;
			default:
				break;
			}
			

			for ( int y = 0; y < renderOptions.height; ++y )
			{
				for ( int x = 0; x < renderOptions.width; ++x )
				{
					auto color = pImgPixels[y * renderOptions.width + x];
#define lmt( x ) ( ( x ) < 255 ? ( x ) : 255 )
					unsigned int colorf = 0xff000000 | lmt( (unsigned int)( color.z * 255 ) ) | lmt( (unsigned int)( color.y * 255 ) ) << 8 | lmt( (unsigned int)( color.x * 255 ) ) << 16;
#undef lmt
					pPixels[y * renderOptions.width + x] = colorf;
				}
			}

			bProcessed = true;
		}		

		memcpy( screen->GetBuffer(), pPixels, size * 4 );
	}
	else
	{
		for ( int y = 0; y < renderOptions.height; ++y )
		{
			for ( int x = 0; x < renderOptions.width; ++x )
			{
#ifdef PATH_TRACER
				auto color = hdrPixels[y * renderOptions.width + x] * vec3( 1.0f / (float)( nS > SAMPLE_NUM2 ? SAMPLE_NUM2 : nS ) );
#else
				auto color = hdrPixels[y * renderOptions.width + x];
#endif // PATH_TRACER
#define lmt( x ) ( ( x ) < 255 ? ( x ) : 255 )
				unsigned int colorf = 0xff000000 | lmt( (unsigned int)( color.z * 255 ) ) | lmt( (unsigned int)( color.y * 255 ) ) << 8 | lmt( (unsigned int)( color.x * 255 ) ) << 16;
#undef lmt
				pPixels[y * renderOptions.width + x] = colorf;
			}
		}

		memcpy( screen->GetBuffer(), pPixels, size * 4 );
	}	
}

const RTRay &RayTracer::generatePrimaryRay( const int x, const int y, const int &sampleIds ) const
{
	vec3 origin = scene.getCamera()->getEye();
	vec2 ndcPixelCentre( ( 2.0f * x - renderOptions.width ) / renderOptions.width, ( renderOptions.height - 2.0f * y ) / renderOptions.height );

	// Stratification
	int u = sampleIds % SAMPLE_NUM;
	int v = sampleIds / SAMPLE_NUM;

	float deltaX = ( u + (float)rand() / RAND_MAX ) * fStratificationSize;
	float deltaY = ( v + (float)rand() / RAND_MAX ) * fStratificationSize;

	ndcPixelCentre += vec2( deltaX, -deltaY );

	vec3 dir = scene.getCamera()->rayDirFromNdc( ndcPixelCentre );
	return RTRay( origin, dir );
}

const RTRay &RayTracer::generatePrimaryRayD( const int x, const int y, const int &sampleIds ) const
{
	RTCamera *camera = scene.getCamera();
	// Stratification
	int u = sampleIds % SAMPLE_NUM;
	int v = sampleIds / SAMPLE_NUM;

	vec3 pixelPoint = camera->left_up_corner +
					  vec3( (float)x * camera->pixel_size, 0.0f, 0.0f ) +
					  vec3( 0.0f, (float)y * -1.0f * camera->pixel_size, 0.0f ) +
					  vec3( ( (float)v + (float)rand() / RAND_MAX ) * camera->strata_size, 0.0f, 0.0f ) +
					  vec3( 0.0f, ( (float)u + (float)rand() / RAND_MAX ) * -camera->strata_size, 0.0f );

	vec2 lensSample = scene.sampler()->get2D() * camera->aperture;
	vec3 lensPoint = camera->getEye() + lensSample.x * camera->right + lensSample.y * camera->up;
	vec3 dir = ( pixelPoint - lensPoint ).normalized();

	
	return RTRay( lensPoint, dir );
}

const vec3 RayTracer::castRay( const RTRay &ray, const int depth, RTIntersection &intersection ) const
{
	if ( depth > renderOptions.maxRecursionDepth )
		return scene.getColor( ray );

	intersection = findNearestObjectIntersection( ray );

	if ( intersection.isIntersecting() )
	{
		return shade( ray, intersection, depth );
	}
	else
		return scene.getColor( ray );
}

const vec3 RayTracer::sample( const RTRay &ray, const int depth, RTIntersection &intersection, bool lastSpecular )
{
	vec3 color_scene = scene.getColor( ray );

	intersection = findNearestObjectIntersection( ray );

	if ( !intersection.isIntersecting() )
	{
		return color_scene;
	}

	const RTMaterial &material = intersection.object->getMaterial();
	// apply MIS, need to consider this direct illumination
	if ( material.isLight() )
	{
		return material.getEmission();
		// 		if (lastSpecular)
		// 		{
		// 			return material.getEmission();
		// 		}
		// 		else
		// 		{
		// 			return color_scene;
		// 		}
	}

	float pdf_hemi_brdf, pdf_light_nee = 0.0f;
	float weight_indirect = 1.0f, weight_direct = 0.0f;

	// random walk ray cos weight
	vec3 random_dir, albedo;
	bool bContinue = material.evaluate( ray, intersection.surfacePointData, scene.getCamera()->getEye(), random_dir, pdf_hemi_brdf, albedo );
	if ( !bContinue )
	{
		return albedo;
	}

	vec3 finalColor = vec3( 0 );
	RTRay ray_random( intersection.surfacePointData.position + renderOptions.shadowBias * random_dir, random_dir );
	// NEE
	if ( ( material.shadingType & DIFFUSE ) == true )
	{
		RTLight *pL = NULL;
		vec3 pos = scene.RandomPointOnLight( pL );

		vec3 Pnt2light = pos - intersection.surfacePointData.position;
		float distance2 = Pnt2light.sqrLentgh();
		Pnt2light.normalize();

		if ( Pnt2light.dot( intersection.surfacePointData.normal ) > 0.0f && Pnt2light.dot( pL->mPlane->normal ) <= 0.0f )
		{
			float area = pL->getArea();

			vec3 org = intersection.surfacePointData.position + renderOptions.shadowBias * Pnt2light;
			RTRay lightSample = RTRay( org, Pnt2light );

			float distance = sqrtf( distance2 );

			if ( !isOcclusion( lightSample, distance - 0.1 ) )
			//RTIntersection it = findNearestObjectIntersection( lightSample );
			//if ( it.isIntersecting() && it.object->getMaterial().isLight() )
			{
				float solidAngle = Pnt2light.dot( pL->mPlane->normal ) * -1.0f * area / distance2;
				pdf_light_nee = 1.0f / solidAngle;

				float pdf_brdf = Pnt2light.dot( intersection.surfacePointData.normal ) * Utils::INV_PI;

				float w1 = BalanceHeuristicWeight( pdf_brdf, pdf_light_nee );
				float w2 = 1.0f - w1;

				float pdf_mis_nee = w1 * pdf_brdf + w2 * pdf_light_nee;

				vec3 color = pL->mPlane->getMaterial().getEmission() * ( 1.0 / pdf_mis_nee ) * albedo * material.brdf( ray, intersection.surfacePointData, lightSample ) * Utils::INV_PI;

				finalColor += color;
			}
#ifdef PHOTON_MAPPING
			else if ( depth == 0 )
			{
				Neighbor neighbors[NUM_PHOTON_RADIANCE];
				vec3 global_color = caustic( neighbors, intersection.surfacePointData.position,
											 intersection.surfacePointData.normal );

				finalColor += global_color * Utils::INV_PI * 0.5 * material.brdf( ray, intersection.surfacePointData, ray_random );
			}
#endif // PHOTON_MAPPING
		}
	}

	// Russian roulette
	float max = std::max( albedo.x, std::max( albedo.y, albedo.z ) );
	if ( depth > renderOptions.maxRecursionDepth )
	{
		if ( scene.sampler()->get1D() < max )
		{
			// Make up for the loss
			albedo *= ( 1.0 / max );
		}
		else
		{
			return color_scene;
		}
	}

	RTIntersection intersection2;
	vec3 diffuse_color = sample( ray_random, depth + 1, intersection2, false );
	// direct illumination adjust the pdf_mis
	if ( intersection2.isIntersecting() && intersection2.object->getMaterial().isLight() )
	{
		RTPlane *plane = (RTPlane *)( intersection2.object );
		SurfacePointData &sp = intersection2.surfacePointData;
		float distance2 = intersection2.rayT * intersection2.rayT;
		vec2 bounds = plane->boundaryxy;
		float area = bounds.x * bounds.y * 4;

		float solidAngle = ray_random.dir.dot( sp.normal ) * -1.0f * area / distance2;

		float pdf_hemi_nee = 1.0f / solidAngle;

		float w1 = BalanceHeuristicWeight( pdf_hemi_brdf, pdf_hemi_nee );
		float w2 = 1.0f - w1;

		pdf_hemi_brdf = w1 * pdf_hemi_brdf + w2 * pdf_hemi_nee;
	}
	vec3 colo_reflect = diffuse_color * material.brdf( ray, intersection.surfacePointData, ray_random ) * ( 1.0f / pdf_hemi_brdf );
	vec3 color_obj = albedo * colo_reflect * Utils::INV_PI;

	finalColor += color_obj;

#ifdef PHOTON_MAPPING
// 	if (depth == 0)
// 	{
// 		Neighbor neighbors[NUM_PHOTON_RADIANCE];
// 		vec3 caustic_color = caustic( neighbors, intersection.surfacePointData.position,
// 									  intersection.surfacePointData.normal );
//
// 		finalColor += caustic_color * 0.5 * Utils::INV_PI * material.brdf( ray, intersection.surfacePointData, ray_random );
// 		//*Utils::INV_PI;
// 	}
#endif // PHOTON_MAPPING

	return finalColor;
}

void RayTracer::castRayPacket( const RayPacket &raypacket, vec3 *colors ) const
{
	int depth = 0;
	RTIntersection intersections[RAYPACKET_RAYS_PER_PACKET];

	scene.findNearestObjectIntersection( raypacket, intersections );

	for ( int i = 0; i < RAYPACKET_RAYS_PER_PACKET; i++ )
	{
		RTIntersection &current = intersections[i];

		if ( current.isIntersecting() )
		{

			colors[i] = shade( raypacket.m_ray[i], current, depth );
		}
		else
			colors[i] = scene.getColor( raypacket.m_ray[i] );
	}
}

const vec3 RayTracer::shade( const RTRay &castedRay, const RTIntersection &intersection, const int depth ) const
{
	auto &surfacePointData = intersection.surfacePointData;
	const RTMaterial &material = intersection.object->getMaterial();
	float distance = ( surfacePointData.position - scene.getCamera()->getEye() ).length();
	const vec3 &albedo = material.getAlbedoAtPoint( surfacePointData.textureCoordinates.x, surfacePointData.textureCoordinates.y, distance );

	//return 0x00ff0000;
	if ( material.shadingType == DIFFUSE )
	{
		return shade_diffuse( castedRay, intersection, depth );
	}
	else if ( material.shadingType == REFLECTIVE )
	{
		return material.reflectionFactor * shade_reflective( castedRay, intersection, depth );
	}
	else if ( material.shadingType == TRANSMISSIVE_AND_REFLECTIVE )
	{
		vec3 refractionColor = vec3( 0 );
		vec3 reflectionColor = vec3( 0 );

		const vec3 &normal = surfacePointData.normal;

		float reflectionFactor;
		reflectionFactor = fresnel( castedRay.dir, normal, material.indexOfRefraction );
		//reflectionFactor = material.reflectionFactor;
		// compute refraction if it is not a case of total internal reflection
		if ( reflectionFactor < 1.0f )
		{
			RTIntersection intersectionObj;
			refractionColor = shade_transmissive( castedRay, intersection, depth, intersectionObj );
			//refractionColor = vec3( 0, 0, 0 );

			if ( intersectionObj.isInSideObj() )
			{
				// Burger-Lambert-Beer law
				vec3 absorbance = albedo * 0.15f * ( -intersectionObj.rayT );
				vec3 transparency = vec3( expf( absorbance.x ), expf( absorbance.y ), expf( absorbance.z ) );
				refractionColor = refractionColor * transparency;
			}
		}
		reflectionColor = shade_reflective( castedRay, intersection, depth );
		//reflectionColor = vec3( 1, 1, 1 );
		return reflectionColor * reflectionFactor + refractionColor * ( 1.0f - reflectionFactor );
	}
	else if ( material.shadingType == DIFFUSE_AND_REFLECTIVE )
	{
		vec3 reflectionColor = shade_reflective( castedRay, intersection, depth );
		vec3 diffuseColor = shade_diffuse( castedRay, intersection, depth );
		return material.reflectionFactor * reflectionColor + ( 1.0f - material.reflectionFactor ) * diffuseColor;
	}
}

RTIntersection RayTracer::findNearestObjectIntersection( const RTRay &ray ) const
{
	return scene.findNearestObjectIntersection( ray );
}

bool RayTracer::isOcclusion( const RTRay &ray, const float &distance ) const
{
	return scene.isOcclusion( ray, distance );
}

float RayTracer::BalanceHeuristicWeight( float &pdf1, float &pdf2 ) const
{
	return ( pdf1 ) / ( pdf1 + pdf2 );
}

void RayTracer::Reset()
{
	this->sample_count = 0;
	memset( hdrPixels, 0, renderOptions.width * renderOptions.height * sizeof( vec3 ) );

	nFilterType = 0;
	bFilter = false;
	bProcessed = false;
}

inline bool compare_x( Photon i, Photon j )
{
	return i.p.x < j.p.x;
}

/**
 * Compare the y coordinates of two photons.
 * @param i The first photon.
 * @param j The second photon.
 * @return true if i smaller than j, false otherwise.
 */
inline bool compare_y( Photon i, Photon j )
{
	return i.p.y < j.p.y;
}

/**
 * Compare the z coordinates of two photons.
 * @param i The first photon.
 * @param j The second photon.
 * @return true if i smaller than j, false otherwise.
 */
inline bool compare_z( Photon i, Photon j )
{
	return i.p.z < j.p.z;
}

void construct_kdtree( std::vector<Photon> &L, unsigned begin, unsigned end )
{
	if ( end - begin == 0 )
	{
		return;
	}
	if ( end - begin == 1 )
	{
		// indicate the leaf node
		L[begin].flag = LEAF;
		return;
	}

	// calculate the variance
	unsigned median = begin + ( end - begin ) / 2;
	float x_avg = 0.0, y_avg = 0.0, z_avg = 0.0;
	float x_var = 0.0, y_var = 0.0, z_var = 0.0;
	float n = end - begin;
	std::vector<Photon>::iterator a = L.begin() + begin;
	std::vector<Photon>::iterator b = L.begin() + end;
	std::vector<Photon>::iterator it;
	for ( it = a; it != b; ++it )
	{
		x_avg += ( *it ).p.x;
		y_avg += ( *it ).p.y;
		z_avg += ( *it ).p.z;
	}
	for ( it = a; it != b; ++it )
	{
		x_var += ( ( *it ).p.x - x_avg ) * ( ( *it ).p.x - x_avg );
		y_var += ( ( *it ).p.y - y_avg ) * ( ( *it ).p.y - y_avg );
		z_var += ( ( *it ).p.z - z_avg ) * ( ( *it ).p.z - z_avg );
	}
	x_var /= n;
	y_var /= n;
	z_var /= n;

	// find the dimension with maximum variance
	float max_var = std::max( x_var, std::max( y_var, z_var ) );

	// split the dimension and indicate the splitting axis
	if ( max_var == x_var )
	{
		std::sort( L.begin() + begin, L.begin() + end, compare_x );
		L[median].flag = X_AXIS;
	}
	if ( max_var == y_var )
	{
		std::sort( L.begin() + begin, L.begin() + end, compare_y );
		L[median].flag = Y_AXIS;
	}
	if ( max_var == z_var )
	{
		std::sort( L.begin() + begin, L.begin() + end, compare_z );
		L[median].flag = Z_AXIS;
	}

	// recurse on left and right children
	construct_kdtree( L, begin, median );
	construct_kdtree( L, median + 1, end );
	return;
}

/**
 * Swap two elements in a max heap.
 * @param neighbors The max heap array.
 * @param a The index of first element.
 * @param b The index of second element.
 * @return void.
 */
inline void heap_swap( Neighbor *neighbors, int a, int b )
{
	unsigned a_i = ( neighbors[a] ).i;
	float a_s = ( neighbors[a] ).sq_dis;

	( neighbors[a] ).i = ( neighbors[b] ).i;
	( neighbors[a] ).sq_dis = ( neighbors[b] ).sq_dis;
	( neighbors[b] ).i = a_i;
	( neighbors[b] ).sq_dis = a_s;
}

/**
 * Remove an element in a max heap.
 * @param neighbors The max heap array.
 * @param size The size of the max heap.
 * @return void.
 */
void heap_remove( Neighbor *neighbors, int *size )
{
	// move the last element to the root node so that
	// the max element is replaced
	( neighbors[0] ).i = ( neighbors[*size - 1] ).i;
	( neighbors[0] ).sq_dis = ( neighbors[*size - 1] ).sq_dis;
	*size = *size - 1;

	int i = 0;
	int left, right, bigger;
	float i_val, left_val, right_val;
	// swap the root node element downward until it has no children
	// or both children have smaller values
	while ( 1 )
	{
		left = 2 * i + 1;
		right = 2 * i + 2;
		if ( left >= *size && right >= *size )
		{
			// i is a leaf node (has no child)
			return;
		}
		i_val = ( neighbors[i] ).sq_dis;
		left_val = ( left < *size ) ? ( neighbors[left] ).sq_dis : -1.0;
		right_val = ( right < *size ) ? ( neighbors[right] ).sq_dis : -1.0;
		if ( i_val >= left_val && i_val >= right_val )
		{
			// i is bigger than both children
			return;
		}
		if ( left_val == -1.0 && right_val != -1.0 )
		{
			// i is smaller than right child
			heap_swap( neighbors, i, right );
			i = right;
		}
		if ( left_val != -1.0 && right_val == -1.0 )
		{
			// i is smaller than left child
			heap_swap( neighbors, i, left );
			i = left;
		}
		else
		{
			// i is smaller than at least one of the child
			bigger = ( left_val > right_val ) ? left : right;
			heap_swap( neighbors, i, bigger );
			i = bigger;
		}
	}

	return;
}

/**
 * Insert an element in a max heap.
 * @param neighbors The max heap array.
 * @param size The size of the max heap.
 * @param e The index of photon in map.
 * @param e_dis The square distance of photon and intersection.
 * @return void.
 */
inline void heap_add( Neighbor *neighbors, int *size,
					  unsigned e, float e_dis )
{
	// insert a new element to the last element of max heap
	int i = *size;
	( neighbors[i] ).i = e;
	( neighbors[i] ).sq_dis = e_dis;
	*size = *size + 1;

	int parent;
	float i_val, parent_val;
	// swap the last element upward unitl it reaches the root node
	// or has a larger parent
	while ( 1 )
	{
		if ( i == 0 )
		{
			// reached root node
			return;
		}
		parent = ( i - 1 ) / 2;
		i_val = ( neighbors[i] ).sq_dis;
		parent_val = ( neighbors[parent] ).sq_dis;
		if ( parent_val >= i_val )
		{
			// parent is bigger than i
			return;
		}

		heap_swap( neighbors, i, parent );
		i = parent;
	}

	return;
}

/**
 * Add a new neighbor to the nearest neighbor max heap.
 * @param p The interection position.
 * @param norm The interection normal.
 * @param neighbors The max heap array.
 * @param L The photon map.
 * @param e The photon index of the map.
 * @param D The maximum square distance of photon in neighor heap.
 * @param size The size of the max heap.
 * @return void.
 */
inline void add_neighbor( vec3 p, vec3 norm, Neighbor *neighbors,
						  std::vector<Photon> &L, unsigned e, float *D, int *size )
{
	// disk check
	if ( abs( dot( norm, normalize( p - L[e].p ) ) ) > Utils::EPSILON_FLOAT )
	{
		return;
	}

	float e_dis = ( p - ( L[e] ).p ).sqrLentgh();
	if ( *size < NUM_PHOTON_RADIANCE || e_dis < *D )
	{
		// maintain the size of the max heap
		if ( *size == NUM_PHOTON_RADIANCE )
		{
			heap_remove( neighbors, size );
		}

		heap_add( neighbors, size, e, e_dis );

		// update the maximum square distance
		*D = ( neighbors[0] ).sq_dis;
	}
}

/**
 * Get the split value.
 * @param L The photon map.
 * @param i The photon index of the map.
 * @param axis The splitting axis.
 * @return The split value.
 */
inline float get_split( std::vector<Photon> &L, unsigned i, int axis )
{
	if ( axis == X_AXIS )
		return ( L[i] ).p.x;
	if ( axis == Y_AXIS )
		return ( L[i] ).p.y;
	if ( axis == Z_AXIS )
		return ( L[i] ).p.z;
	return 0.0;
}

/**
 * Get the coordinate of intersection of the splitting dimension.
 * @param p The interection position.
 * @param axis The splitting axis.
 * @return The coordinate of the splitting dimension.
 */
inline float get_p( Vector3 p, int axis )
{
	if ( axis == X_AXIS )
		return p.x;
	if ( axis == Y_AXIS )
		return p.y;
	if ( axis == Z_AXIS )
		return p.z;
	return 0.0;
}

/**
 * Get the nearest photon neighbors of a position.
 * @param p The interection position.
 * @param norm The interection normal.
 * @param neighbors The max heap array.
 * @param L The photon map.
 * @param e The photon index of the map.
 * @param D The maximum square distance of photon in neighor heap.
 * @param size The size of the max heap.
 * @return void.
 */
void lookup( vec3 p, vec3 norm, Neighbor *neighbors, std::vector<Photon> &L,
			 unsigned begin, unsigned end, float *D, int *size )
{
	if ( begin == end )
		return;
	if ( begin + 1 == end )
		// add photon at leaf node to neighbors heap
		return add_neighbor( p, norm, neighbors, L, begin, D, size );

	unsigned median = begin + ( end - begin ) / 2;
	// get splitting axis
	int flag = ( L[median] ).flag;
	float split_value = get_split( L, median, flag );
	float p_value = get_p( p, flag );
	// check which side of the splitting axis to traverse
	if ( p_value <= split_value )
	{
		// traverse left sub-tree first
		lookup( p, norm, neighbors, L, begin, median, D, size );
		// add the current node
		add_neighbor( p, norm, neighbors, L, median, D, size );
		// return if neighbors heap is full and all nodes in the
		// right sub-tree is further than those in neighbors heap
		if ( *size >= NUM_PHOTON_RADIANCE &&
			 ( p_value - split_value ) * ( p_value - split_value ) > *D )
		{
			return;
		}
		// traverse right sub-tree
		return lookup( p, norm, neighbors, L, median + 1, end, D, size );
	}
	else
	{
		// traverse right sub-tree first
		lookup( p, norm, neighbors, L, median + 1, end, D, size );
		// add the current node
		add_neighbor( p, norm, neighbors, L, median, D, size );
		// return if neighbors heap is full and all nodes in the
		// left sub-tree is further than those in neighbors heap
		if ( *size >= NUM_PHOTON_RADIANCE &&
			 ( p_value - split_value ) * ( p_value - split_value ) > *D )
		{
			return;
		}
		// traverse left sub-tree
		return lookup( p, norm, neighbors, L, begin, median, D, size );
	}
}

void RayTracer::emit_photons()
{
	for ( int i = 0; i < NUM_PHOTON; i++ )
	{
		RTLight *pL = NULL;
		vec3 pos = scene.RandomPointOnLight( pL );

		vec3 p;
		p.x = 2.0 * float( rand() ) / RAND_MAX - 1.0;
		p.y = 2.0 * float( rand() ) / RAND_MAX - 1.0;
		p.z = 2.0 * float( rand() ) / RAND_MAX - 1.0;
		p.normalize();

		vec3 dir = sampleCosHemisphere( pL->mPlane->normal );
		if ( NUM_PHOTON == 500 )
		{
			dir = pL->mPlane->normal; // numbers of samples are too slow
		}
		RTRay ray( pos, dir );

		// scale the intensity of the photon ray
		Photon ph = Photon( vec3( 1 ) );
		// do photon tracing
		trace_photon( ray, ph, 0, 0 );
	}

	construct_kdtree( global_illum_map, 0, global_illum_map.size() );
	construct_kdtree( caustic_map, 0, caustic_map.size() );
}

vec3 RayTracer::global_illumination( Neighbor *neighbors, vec3 p, vec3 norm )
{
	vec3 result = vec3( 0 );
	// D is the furthest photon squared distance to p in neighbors
	float D = Utils::MAX_FLOAT;
	int size = 0;
	// find the nearest photons
	lookup( p, norm, neighbors, global_illum_map,
			0, global_illum_map.size(), &D, &size );
	if ( size == 0 )
		return result;
	for ( int i = 0; i < size; i++ )
	{
		Photon ph = global_illum_map[( neighbors[i] ).i];
		result += ph.c * std::max( dot( norm, ph.dir ), 0.0f );
	}
	return result * ( 1.0 / ( D * PI ) );
}

/**
 * Get the color from caustic.
 * @param p The interection position.
 * @param norm The interection normal.
 * @param neighbors The max heap array.
 * @return The caustic color at intersection.
 */
vec3 RayTracer::caustic( Neighbor *neighbors, vec3 p, vec3 norm )
{
	vec3 result = vec3( 0 );
	// D is the furthest photon squared distance to p in neighbors
	float D = Utils::MAX_FLOAT;
	int size = 0;
	// find the nearest photons
	lookup( p, norm, neighbors, caustic_map,
			0, caustic_map.size(), &D, &size );
	if ( size == 0 )
		return result;
	for ( int i = 0; i < size; i++ )
	{
		Photon ph = caustic_map[( neighbors[i] ).i];
		result += ph.c * std::max( dot( norm, ph.dir ), 0.0f );
	}
	return result * ( 1.0 / ( D * PI ) );
}

void RayTracer::SetFilterMethod( int nType )
{
	if ( nType != 0 )
	{
		nFilterType = nType;
		bFilter = true;
	}
}

void RayTracer::SetMotion( bool bMotion )
{
	mMotion = bMotion;
}

inline bool refract2( vec3 d, vec3 norm, float n, float nt, vec3 *t )
{
	// if the number under the square root is negative, there is no
	// refracted ray and all of the energy is reflected, which is
	// known as total internal reflection.
	float d_n = dot( d, norm );
	float sq_rt = 1.0 - ( n * n * ( 1.0 - ( d_n * d_n ) ) ) / ( nt * nt );

	if ( sq_rt < 0.0 )
	{
		return false;
	}
	else
	{
		vec3 tmp =
			( n * ( d - norm * d_n ) ) * ( 1.0f / nt ) - norm * sqrt( sq_rt );
		t->x = tmp.x;
		t->y = tmp.y;
		t->z = tmp.z;
		return true;
	}
}
void RayTracer::trace_photon( const RTRay &ray, Photon &ph, int depth, int previous_bounce )
{
	if ( depth > renderOptions.maxRecursionDepth )
	{
		return;
	}

	RTIntersection intersection;
	intersection = findNearestObjectIntersection( ray );

	if ( !intersection.isIntersecting() )
	{
		return;
	}
	SurfacePointData &hitPnt = intersection.surfacePointData;
	ph.p = hitPnt.position;

	const RTMaterial &material = intersection.object->getMaterial();
	float distance = ( hitPnt.position - scene.getCamera()->getEye() ).length();
	const vec3 &albedo = material.getAlbedoAtPoint( hitPnt.textureCoordinates.x, hitPnt.textureCoordinates.y, distance );

	if ( material.shadingType == TRANSMISSIVE_AND_REFLECTIVE )
	{
		float c, n, nt;
		vec3 t;
		bool entering;

		// compute the reflected ray
		vec3 nd = ray.dir - ( ( 2 * ray.dir.dot( hitPnt.normal ) ) * hitPnt.normal );
		nd.normalize();
		RTRay reflect_r( hitPnt.position + nd * renderOptions.shadowBias, nd );

		// adjust photon power
		ph.c = ph.c * std::max( albedo.x, std::max( albedo.y, albedo.z ) );

		// entering the dielectric
		if ( dot( ray.dir, hitPnt.normal ) < 0.0 )
		{
			entering = true;
			n = 1;
			nt = material.indexOfRefraction;
			refract2( ray.dir.normalized(), hitPnt.normal, n, nt, &t );
			c = dot( -ray.dir.normalized(), hitPnt.normal );
		}
		// leaving the dialectric
		else
		{
			entering = false;
			n = material.indexOfRefraction;
			nt = 1;
			if ( refract2( ray.dir.normalized(), -hitPnt.normal, n, nt, &t ) )
			{
				c = dot( t.normalized(), hitPnt.normal );
			}
			// total internal reflection
			else
			{
				// R = 1 for total internal reflection
				return trace_photon( reflect_r, ph, depth + 1, REFLECTIVE );
			}
		}

		// compute Fresnel coefficient R
		float R0 = (float)pow( (double)( ( nt - 1.0 ) / ( nt + 1.0 ) ), 2.0 );
		float R = R0 + ( 1.0 - R0 ) * (float)pow( 1.0 - c, 5.0 );
		t.normalize();
		RTRay refract_r( hitPnt.position + t * renderOptions.shadowBias, t );

		if ( entering )
		{
			// Russian Roulette sampling
			if ( float( rand() ) / RAND_MAX < R )
			{
				return trace_photon( reflect_r, ph, depth + 1, REFLECTIVE );
			}
			else
			{
				return trace_photon( refract_r, ph, depth + 1, REFLECTIVE );
			}
		}
		else
		{
			// Russian Roulette sampling
			if ( float( rand() ) / RAND_MAX < R )
			{
				return trace_photon( reflect_r, ph, depth + 1, REFLECTIVE );
			}
			else
			{
				return trace_photon( refract_r, ph, depth + 1, REFLECTIVE );
			}
		}

		// 		int bounce = DIFFUSE;
		// 		vec3 random_dir;
		// 		float reflectionFactor;
		// 		reflectionFactor = Utils::fresnel( ray.dir, hitPnt.normal, material.indexOfRefraction );
		//
		// 		ph.c = ph.c * std::max( albedo.x, std::max( albedo.y, albedo.z ) );
		//
		// 		if (false && (float)rand() / RAND_MAX < reflectionFactor )
		// 		{
		// 			random_dir = ray.dir - ( ( 2 * ray.dir.dot( hitPnt.normal ) ) * hitPnt.normal );
		// 		}
		// 		else
		// 		{
		// 			bool bOut = true;
		// 			random_dir = Utils::refract( ray.dir, hitPnt.normal, material.indexOfRefraction,bOut );
		// 			if (bOut == false)
		// 			{
		// 				bounce = REFLECTIVE;
		// 			}
		// 		}
		// 		RTRay ray_ref( hitPnt.position + random_dir * renderOptions.shadowBias, random_dir );
		//
		// 		return trace_photon( ray_ref, ph, depth + 1, bounce );
	}
	else
	{
		int bounce = material.shadingType;
		vec3 random_dir;
		if ( material.shadingType == DIFFUSE_AND_REFLECTIVE )
		{
			if ( (float)rand() / RAND_MAX < material.reflectionFactor )
			{
				random_dir = ray.dir - ( ( 2 * ray.dir.dot( hitPnt.normal ) ) * hitPnt.normal );
				bounce = DIFFUSE;
			}
			else
			{
				random_dir = sampleCosHemisphere( hitPnt.normal );
				bounce = DIFFUSE;
			}
		}
		else if ( material.shadingType == DIFFUSE )
		{
			random_dir = sampleCosHemisphere( hitPnt.normal );
			bounce = DIFFUSE;
		}
		else
		{
			random_dir = ray.dir - ( ( 2 * ray.dir.dot( hitPnt.normal ) ) * hitPnt.normal );
			bounce = DIFFUSE;
		}

		if ( depth != 0 )
		{
			ph.dir = -normalize( ray.dir );
			if ( previous_bounce == DIFFUSE )
				global_illum_map.push_back( ph );
			if ( previous_bounce == REFLECTIVE )
				caustic_map.push_back( ph );
		}

		bool bAbsorb = false;
		float max = std::max( ph.c.x, std::max( ph.c.y, ph.c.z ) );
		if ( scene.sampler()->get1D() > max )
		{
			bAbsorb = true;
		}

		if ( bAbsorb )
			return;
		else
		{
			Photon reflected_ph = Photon( ph.c * albedo );
			RTRay ray_ref( hitPnt.position + random_dir * renderOptions.shadowBias, random_dir );
			return trace_photon( ray_ref, reflected_ph, depth + 1, bounce );
		}
	}
}

int RayTracer::getSCount()
{
	return sample_count;
}

const Tmpl8::vec3 RayTracer::shade_diffuse( const RTRay &castedRay, const RTIntersection &intersection, const int depth ) const
{
	auto &surfacePointData = intersection.surfacePointData;
	const RTMaterial &material = intersection.object->getMaterial();
	vec3 color( 0.0f );
	if ( castedRay.dir.dot( surfacePointData.normal ) > 0 )
	{
		return color;
	}
	const vec3 albedo = material.getAlbedoAtPoint( surfacePointData.textureCoordinates.x, surfacePointData.textureCoordinates.y, castedRay.distance_traveled + intersection.rayT );
	color = scene.ambientLight * albedo;
	static auto &light_list = scene.getLights();
	for ( RTLight *light : light_list )
	{
		color += light->shade( surfacePointData, *this, albedo );
	}

	return color;
}

const Tmpl8::vec3 RayTracer::shade_reflective( const RTRay &castedRay, const RTIntersection &intersection, const int depth ) const
{
	auto &surfacePointData = intersection.surfacePointData;
	const RTMaterial &material = intersection.object->getMaterial();
	vec3 nd = castedRay.dir - ( ( 2 * castedRay.dir.dot( surfacePointData.normal ) ) * surfacePointData.normal );
	RTRay refRay = RTRay( surfacePointData.position + renderOptions.shadowBias * nd, nd, castedRay.distance_traveled + intersection.rayT );

	RTIntersection intersection2;
	return castRay( refRay, depth + 1, intersection2 );
}

const Tmpl8::vec3 RayTracer::shade_transmissive( const RTRay &castedRay, const RTIntersection &intersection, const int depth, RTIntersection &intersectionObj ) const
{
	auto &surfacePointData = intersection.surfacePointData;
	const RTMaterial &material = intersection.object->getMaterial();
	const vec3 &normal = surfacePointData.normal; // normal texture later

	float distance = ( surfacePointData.position - scene.getCamera()->getEye() ).length();
	const vec3 &albedo = material.getAlbedoAtPoint( surfacePointData.textureCoordinates.x, surfacePointData.textureCoordinates.y, distance );
	const bool outside = dot( castedRay.dir, normal ) < 0;
	const vec3 bias = renderOptions.shadowBias * normal;
	const vec3 &refractionDirection = refract( castedRay.dir, normal, material.indexOfRefraction );
	const vec3 refractionRayOrig = outside ? surfacePointData.position - bias : surfacePointData.position + bias;
	const RTRay refractionRay( refractionRayOrig, refractionDirection, castedRay.distance_traveled + intersection.rayT );

	return castRay( refractionRay, depth + 1, intersectionObj ) * albedo;
}

float RayTracer::fresnel( const vec3 &I, const vec3 &N, const float refractionIndex ) const
{
	float cosi = Utils::clamp_rt( dot( I, N ), -1.0f, 1.0f );
	float etai = 1, etat = refractionIndex;
	if ( cosi > 0 )
		std::swap( etai, etat );

	//Snell's law
	float sint = etai / etat * sqrtf( std::max( 0.0f, 1.0f - cosi * cosi ) );

	// Total internal reflection
	if ( sint >= 1 )
	{
		return 1.0f;
	}
	else
	{
		float cost = sqrtf( std::max( 0.0f, 1.0f - sint * sint ) );
		cosi = fabsf( cosi );
		float Rs = ( ( etat * cosi ) - ( etai * cost ) ) / ( ( etat * cosi ) + ( etai * cost ) );
		float Rp = ( ( etai * cosi ) - ( etat * cost ) ) / ( ( etai * cosi ) + ( etat * cost ) );
		return ( Rs * Rs + Rp * Rp ) / 2.0f;
	}
}

const Tmpl8::vec3 RayTracer::refract( const vec3 &I, const vec3 &N, const float refractionIndex ) const
{
	float cosi = Utils::clamp_rt( dot( I, N ), -1.0f, 1.0f );
	float etai = 1, etat = refractionIndex;
	vec3 n = N;
	if ( cosi < 0.0f )
		cosi = -cosi;
	else
	{
		std::swap( etai, etat );
		n = -N;
	}
	float eta = etai / etat;
	float k = 1.0f - eta * eta * ( 1 - cosi * cosi );

	//k < 0 = total internal reflection
	return k < 0.0f ? vec3( 0.0f ) : ( eta * I ) + ( eta * cosi - sqrtf( k ) ) * n;
}
