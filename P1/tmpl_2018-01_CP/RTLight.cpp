#include "RTLight.h"
#include "precomp.h"
#include"RenderOptions.h"

RTLight::RTLight( vec3 _color, float _power ) : color( _color ), power( _power ), 
mConstantAttenutaionCoefficient( 0.0f ), mLinearAttenutaionCoefficient( 0.0f ), mQuadraticAttenutaionCoefficient(1.0f),
mUmbraAngle( 40.0f ), mPenumbraAngle( 160.0f ), mFalloffFactor( 1.0f )
{
	mUmbraAngle = mUmbraAngle * Utils::RT_PI / 180.0f;
	mPenumbraAngle = mPenumbraAngle * Utils::RT_PI / 180.0f;

	mHalfUmbraAngleCosine = cosf( mUmbraAngle / 2.0f );
	mHalfPenumbraAngleCosine = cosf( mPenumbraAngle / 2.0f );
}

RTLight::~RTLight()
{
}



class PointLight : public RTLight
{
  private:
	vec3 pos;

  public:
	PointLight( vec3 _color, float _power, vec3 _pos ) : RTLight( _color, _power ), pos( _pos )
	{
	}
	vec3 shade( const SurfacePointData &pd, const RayTracer &rt, const vec3 & texture )override
	{
		vec3 d = pos - pd.position;
		float l2 = d.sqrLentgh();
		float l = sqrt( l2 );
		vec3 nd = d * ( 1.0f / l );

		RTRay ray = RTRay( pd.position + rt.getRenderOptions().shadowBias * nd, nd );
		const RTIntersection &intersection = rt.findNearestObjectIntersection( ray );

		if ( !intersection.isIntersecting() || intersection.rayT > distanceToPoint( ray .orig) )
		{
			float cosine = pd.normal.dot( nd );
			cosine = ( cosine > 0 ? cosine : 0 ) / ( mConstantAttenutaionCoefficient + mLinearAttenutaionCoefficient * l + mQuadraticAttenutaionCoefficient*l2 ) * power;
			return cosine * texture * color;
		}
		else if ( intersection.object->getMaterial().shadingType == TRANSMISSIVE_AND_REFLECTIVE )
		{
			SurfacePointData shadowObjectSurfacePoint = intersection.object->getSurfacePointData( intersection );
			return vec3( .0f, .0f, .0f );
		}

		return vec3( .0f, .0f, .0f );
		
	}

	float distanceToPoint( const vec3 &point ) const
	{
		return ( pos - point ).length();
	}
};

class SpotLight : public RTLight
{
  private:
	vec3 pos;
	vec3 dir;

  public:
	SpotLight( vec3 _color, float _power, vec3 _pos, vec3 _dir ) : RTLight( _color, _power ), pos( _pos ), dir( _dir )
	{
	}
	vec3 shade( const SurfacePointData &pd, const RayTracer &rt, const vec3 &texture ) override
	{
		vec3 d = pos - pd.position;
		float l2 = d.sqrLentgh();
		float l = sqrt( l2 );
		vec3 nd = d * ( 1.0f / l );

		RTRay ray = RTRay( pd.position + rt.getRenderOptions().shadowBias * nd, nd );
		const RTIntersection &intersection = rt.findNearestObjectIntersection( ray );

		if ( !intersection.isIntersecting() || intersection.rayT > distanceToPoint( ray.orig ) )
		{
			vec3 lightVector = -dir;
			float lightVectorDotNormal = lightVector.dot( pd.normal );

			float lightDirectionDotLightVector = nd.dot( lightVector );
			float spotAttenuation = 0.0;
			if ( lightDirectionDotLightVector > mHalfUmbraAngleCosine )
			{
				spotAttenuation = 1.0;
			}
			else if ( lightDirectionDotLightVector < mHalfPenumbraAngleCosine )
			{
				spotAttenuation = 0.0;
			}
			else
			{
				const float factor = ( lightDirectionDotLightVector - mHalfPenumbraAngleCosine ) / ( mHalfUmbraAngleCosine - mHalfPenumbraAngleCosine );
				spotAttenuation = powf( factor, mFalloffFactor );
			}

			float disAttenuation = ( mConstantAttenutaionCoefficient + mLinearAttenutaionCoefficient * l + mQuadraticAttenutaionCoefficient * l2 );
			lightVectorDotNormal = ( lightVectorDotNormal > 0 ? lightVectorDotNormal : 0 ) / disAttenuation * spotAttenuation * power;
			return lightVectorDotNormal * texture * color;
		}
		else if ( intersection.object->getMaterial().shadingType == TRANSMISSIVE_AND_REFLECTIVE )
		{
			SurfacePointData shadowObjectSurfacePoint = intersection.object->getSurfacePointData( intersection );
			return vec3( .0f, .0f, .0f );
		}

		return vec3( .0f, .0f, .0f );
	}

	float distanceToPoint( const vec3 &point ) const
	{
		return ( pos - point ).length();
	}
};

class ParrallelLight : public RTLight
{
  private:
	vec3 direction;

  public:
	ParrallelLight( vec3 _color, float _power, vec3 _direction ) : RTLight( _color, _power ), direction( _direction )
	{

	}
	vec3 shade( const SurfacePointData &pd, const RayTracer &rt, const vec3& texture ) override
	{
		const RTRay ray = RTRay( pd.position - rt.getRenderOptions().shadowBias * direction, -direction );
		const RTIntersection intersection = rt.findNearestObjectIntersection( ray );
		if ( intersection.isIntersecting() )
		{
			return vec3( .0f, .0f, .0f );
		}
		float cosine = pd.normal.dot( -direction );
		cosine = ( cosine > 0 ? cosine : 0 ) * power;
		return cosine * texture * color;
	}
};


RTLight *RTLight::createPointLight( vec3 _color, float _power, vec3 _pos )
{
	return new PointLight( _color, _power, _pos );
}

RTLight *RTLight::createParralleLight(vec3 _color, float _power, vec3 _direction)
{
	return new ParrallelLight( _color, _power, _direction );
}

RTLight *RTLight::createSpotLight( vec3 _color, float _power, vec3 _pos, vec3 _direction )
{
	return new SpotLight( _color, _power, _pos, _direction );
}
