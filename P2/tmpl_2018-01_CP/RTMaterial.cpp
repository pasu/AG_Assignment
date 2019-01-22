#include "precomp.h"
#include "RTMaterial.h"
#include "Sampler.h"
#include "Utils.h"
#include "LocalSpace.h"

 RTMaterial::RTMaterial( const vec3 &color, const ShadingType shadingType ) 
	 : RTMaterial( color, vec3( 0 ), 0, vec2( 1 ), shadingType, 0.8f, 1.0f )
{
}

 RTMaterial::RTMaterial( const vec3 &color, const vec3 &emission, const ShadingType shadingType )
  : RTMaterial( color,emission, 0, vec2( 1 ), shadingType, 0.8f, 1.0f )
  {
	 
  }
 RTMaterial::RTMaterial( const RTTexture *albedo, const ShadingType shadingType )
	  : RTMaterial( vec3( 1.0f ), vec3( 0 ), albedo, vec2( 1 ), shadingType, 0.8f, 1.0f )
  {
}

 RTMaterial::RTMaterial( const vec3 &color, const RTTexture *albedo, const ShadingType shadingType )
	: RTMaterial( color, vec3( 0 ), albedo, vec2( 1 ), shadingType, 0.8f, 1.0f )
{
}

 RTMaterial::RTMaterial( const vec3 &color, const RTTexture *albedo, const ShadingType shadingType, const float reflectionFactor )
	: RTMaterial( color, vec3( 0 ), albedo, vec2( 1 ), shadingType, reflectionFactor, 1.0f )
{
}

 RTMaterial::RTMaterial( const vec3 &color, const vec3 &emission, const RTTexture *albedo, const vec2 &textureScale, const ShadingType shadingType, const float reflectionFactor, const float indexOfRefraction )
	: color( color ), emission( emission ),
	  albedoTexture( albedo ), shadingType( shadingType ),
	reflectionFactor( reflectionFactor ), indexOfRefraction( indexOfRefraction ), textureScale( textureScale )
{
	 pow_ = 2.0f;
	 k_ = 0.0f;
	 bLight = false;

	 this->emission = emission;
	 if ( !Utils::floatEquals( this->emission.x, 0.0f ) ||
		  !Utils::floatEquals( this->emission.y, 0.0f ) ||
		  !Utils::floatEquals( this->emission.z, 0.0f ) )
	 {
		 bLight = true;
	 }
 }

const vec3 RTMaterial::getAlbedoAtPoint( const float s, const float t, float z ) const
{
	if ( albedoTexture == 0 )
		return color;
	else
		return color * albedoTexture->getTexel( s, t,z, textureScale );
}

float RTMaterial::brdf( const RTRay &ray, const SurfacePointData &hitPnt, const RTRay &ray_random ) const
{
	float cosine = 0;
	vec3 reflect_dir;
	switch ( shadingType )
	{
	case DIFFUSE:
		cosine = hitPnt.normal.dot( ray_random.dir );
		cosine = cosine < 0.0 ? 0.0f : cosine;
		return cosine * Utils::INV_PI;
		break;
	case REFLECTIVE:
		reflect_dir = ray.dir - ( ( 2 * ray.dir.dot( hitPnt.normal ) ) * hitPnt.normal );
		if ( Utils::floatEquals (reflect_dir.x,ray_random.dir.x) &&
			 Utils::floatEquals( reflect_dir.y, ray_random.dir.y ) &&
			 Utils::floatEquals( reflect_dir.z, ray_random.dir.z ) )
		{
			return 1.0f;
		}
		return 0.0f;
		break;
	case TRANSMISSIVE_AND_REFLECTIVE:
		return 1.0f;
		break;
	case DIFFUSE_AND_REFLECTIVE:
		reflect_dir = ray.dir - ( ( 2 * ray.dir.dot( hitPnt.normal ) ) * hitPnt.normal );
		if ( Utils::floatEquals( reflect_dir.x, ray_random.dir.x ) &&
			 Utils::floatEquals( reflect_dir.y, ray_random.dir.y ) &&
			 Utils::floatEquals( reflect_dir.z, ray_random.dir.z ) )
		{
			return 1.0f;
		}

		cosine = hitPnt.normal.dot( ray_random.dir );
		cosine = cosine < 0.0 ? 0.0f : cosine;
		return cosine * Utils::INV_PI;
		break;
	case Phong:
	{
		reflect_dir = ray.dir - ( ( 2 * ray.dir.dot( hitPnt.normal ) ) * hitPnt.normal );
		reflect_dir.normalize();

		float cosineS = reflect_dir.dot( ray_random.dir );
		cosineS = cosineS < Utils::EPSILON_FLOAT ? Utils::EPSILON_FLOAT : cosineS;
		float cosine = -1.0f * ray.dir.dot( ray_random.dir );
		cosine = cosine < 0.0f ? 0.0f : cosine;
		float result = Utils::EPSILON_FLOAT + ( pow_ + 2.0f ) * Utils::INV_PI * 0.5 * pow( cosineS, pow_ );
		result = result < 0.0f ? 0.0f : result;
		return result;
	}
		break;
	case Glossy:
	{
		if ( Utils::floatEquals(pow_,0.0f) ) return 2.0f;

		vec3 reflectDir = ray.dir - 2.0f * ( ray.dir * hitPnt.normal ) * hitPnt.normal;
		float cosine = ray_random.dir.dot( reflectDir );
		float cosineExt = sqrtf( 1.0f - pow_ * pow_ );

		// Within the lobe, give cosine/solidAngleMax (sum up to 1.0f)
		if ( Utils::floatEquals(cosine,cosineExt) )
			return 1.0f / ( 2.0f * Utils::RT_PI * sqrtf( 1 - pow_ * pow_ ) );

		return 0.0f;
	}
		break;
	case MICROFACET:
	{
		LocalSpace lspace;
		lspace.init( hitPnt.normal );
		vec3 N = lspace.world( hitPnt.normal );
		N.normalize();
		vec3 L = lspace.world( ray_random.dir );
		L.normalize();
		vec3 V = lspace.world( -1.0f * ray.dir );
		V.normalize();

		vec3 Half = L + V;
		Half.normalize();
		float NH = N.dot( Half ), NL = N.dot( L );
		float NV = N.dot( V ), invVH = 1.0f / ( V.dot( Half ) );
		NH = NH < 0.0f ? 0.0f : NH;
		NL = NL < 0.0f ? 0.0f : NL;
		NV = NV < 0.0f ? 0.0f : NV;
		invVH = invVH < 0.0f ? 0.0f : invVH;

		float Geo = min( 1.0f, min( 2.0f * NH * NV * invVH, 2.0f * NH * NL * invVH ) );
		float Fr = k_ + ( 1.0f - k_ ) * pow( ( 1.0f - Half.dot( L ) ), 5.0f );
		float Dh = 0.5f * Utils::INV_PI * ( pow_ + 2.0f ) * pow( Half.z, pow_ );
		float Denom = 4.0f * NV * NL;

		float result = Geo * Fr * Dh / Denom;
		result = result < 0.0f ? 0.0f : result;
		return result;
	}
		break;
	}

	return cosine * Utils::INV_PI;
}

vec3 random_phong_direction( float a )
{
	float r1 = (float)rand() / RAND_MAX, r2 = (float)rand() / RAND_MAX;

	float z = pow( r2, 1.0f / ( a + 1.0f ) );
	float t = pow( r2, 2.0f / ( a + 1.0f ) );
	float phi = 2.0f * Utils::RT_PI * r1;
	float rt = (float)sqrtf( 1.0f - t );
	float x = cosf( phi ) * rt;
	float y = sinf( phi ) * rt;

	return vec3( x, y, z );
}

vec3 random_in_unit_sphere()
{
	vec3 vec;

	do
	{
		vec = 2.0f * vec3( (float)rand() / RAND_MAX, (float)rand() / RAND_MAX, (float)rand() / RAND_MAX ) - vec3( 1.0f, 1.0f, 1.0f );
	} while ( vec.sqrLentgh() >= 1.0f );

	vec.normalize();
	return vec;
}

vec3 random_microfacet_direction( float alpha )
{
	float r1, r2;
	do
	{
		r1 = (float)rand() / RAND_MAX;
		r2 = (float)rand() / RAND_MAX;
	} while ( r1 == 0.0f || r2 == 1.0f );

	float cosTheta = pow( r1, ( 1.0f / ( alpha + 1 ) ) );
	float sinTheta = sqrtf( 1.0f - cosTheta * cosTheta );
	float Phi = 2.0f * Utils::RT_PI * r2;

	return vec3( sinTheta * cosf( Phi ),
					 sinTheta * sinf( Phi ),
					 cosTheta );
}

bool RTMaterial::evaluate( const RTRay &ray, const SurfacePointData &hitPnt, vec3 &eye_pos, vec3 &random_dir, float &pdf, vec3 &albedo ) const
{
	float distance = ( hitPnt.position - eye_pos ).length();
	albedo = this->getAlbedoAtPoint( hitPnt.textureCoordinates.x, hitPnt.textureCoordinates.y, distance );
	
	LocalSpace lspace;
	switch ( shadingType )
	{
	case DIFFUSE:
		sampleDiffuse( hitPnt.normal, ray.dir, random_dir, pdf );
		return true;
		break;
	case REFLECTIVE:
		random_dir = ray.dir - ( ( 2 * ray.dir.dot( hitPnt.normal ) ) * hitPnt.normal );
		pdf = 1.0f;
		break;
	case TRANSMISSIVE_AND_REFLECTIVE:
		pdf = 1.0f;

		float reflectionFactor;
		reflectionFactor = Utils::fresnel( ray.dir, hitPnt.normal, indexOfRefraction );

		if ( (float)rand() / RAND_MAX < reflectionFactor )
		{
			random_dir = ray.dir - ( ( 2 * ray.dir.dot( hitPnt.normal ) ) * hitPnt.normal );
		}
		else
		{
			random_dir = Utils::refract( ray.dir, hitPnt.normal, indexOfRefraction );
		}
		break;
	case DIFFUSE_AND_REFLECTIVE:
		pdf = 1.0f;

		if ( (float)rand() / RAND_MAX < this->reflectionFactor )
		{
			// This part is much like what we do in Mirror
			random_dir = ray.dir - ( ( 2 * ray.dir.dot( hitPnt.normal ) ) * hitPnt.normal );
			return true;
		}
		else
		{
			return false;
		}
		break;
	case Phong:
	{
		lspace.init( hitPnt.normal );
		vec3 sampleDir = lspace.local( random_phong_direction( pow_ ) );
		sampleDir.normalize();
		vec3 reflectDir = ray.dir - 2.0f * ( ray.dir.dot( hitPnt.normal ) ) * hitPnt.normal;
		reflectDir.normalize();
		float cosine = sampleDir.dot( reflectDir );
		cosine = cosine < Utils::EPSILON_FLOAT ? Utils::EPSILON_FLOAT : cosine;
		random_dir = sampleDir;
		pdf = ( pow_ + 1.0f ) * 0.5f * Utils::INV_PI * pow( cosine, pow_ );
		pdf = pdf < Utils::EPSILON_FLOAT ? Utils::EPSILON_FLOAT : pdf;

		return true;
	}		
		break;
	case Glossy:
	{
		vec3 reflectDir = ray.dir - ( ( 2 * ray.dir.dot( hitPnt.normal ) ) * hitPnt.normal );
		vec3 samplePoint = hitPnt.position + reflectDir + pow_ * random_in_unit_sphere();
		random_dir = samplePoint - hitPnt.position;
		pdf = 1.0f / ( 2.0f * Utils::RT_PI * sqrtf( 1 - pow_ * pow_) );
		return true;
	}
		break;
	case MICROFACET:
	{
		lspace.init( hitPnt.normal );

		vec3 H = random_microfacet_direction( pow_ );
		H.normalize();
		vec3 N = lspace.world( hitPnt.normal );
		N.normalize();
		vec3 V = lspace.world( -1.0f * ray.dir );
		V.normalize();
		float VH = V.dot( H );
		VH = VH < 0.0f ? 0.0f : VH;
		vec3 L = 2.0f * VH * H - V;
		L.normalize();

		float HN = H.dot( N );
		HN = HN < 0.0f ? 0.0f : HN;

		pdf = ( pow_ + 2.0f ) * pow( H.z, ( pow_ + 1.0f ) ) / ( 8.0f * PI * VH );
		L = lspace.local( L );
		random_dir = L;
		return true;
	}
	break;
	default:
		break;
	}
}

const Tmpl8::vec3 RTMaterial::getEmission()const
{
	return emission;
}

bool RTMaterial::isLight()const
{
	return bLight;
}

void RTMaterial::sampleDiffuse( const vec3& normal, const vec3 &out, vec3 &in, float &pdf ) const
{
	in = sampleCosHemisphere( normal );
	pdf = in.dot( normal ) * Utils::INV_PI;
}
