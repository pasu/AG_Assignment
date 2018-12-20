#pragma once
#include "RTSurfacePointData.h"
#include "template.h"
#include"RTMaterial.h"

#include "Utils.h"

class RayTracer;
class RTLight
{
  protected:
	vec3 color;
	float power;
	float currentIndexOfRefraction;

	// Used for point and spot light
	float mConstantAttenutaionCoefficient;
	float mLinearAttenutaionCoefficient;
	float mQuadraticAttenutaionCoefficient;

	// Used for spot light
	float mUmbraAngle;
	// Penumbra angle
	float mPenumbraAngle;
	// Falloff factor
	float mFalloffFactor;

	float mHalfUmbraAngleCosine;
	float mHalfPenumbraAngleCosine;

  public:
	RTLight( vec3 _color, float _power );
	~RTLight();

	static RTLight *createPointLight( vec3 _color, float _power, vec3 _pos );
	static RTLight *createParralleLight( vec3 _color, float _power, vec3 _direction );
	static RTLight *createSpotLight( vec3 _color, float _power, vec3 _pos,vec3 _direction );

	virtual vec3 shade( const SurfacePointData & pd, const RayTracer &rt, const vec3& texture) = 0; // shade a diffused surface

	void setAttenuation( const float& constant,
						 const float &linear, const float &quadratic )
	{
		mConstantAttenutaionCoefficient = constant;
		mLinearAttenutaionCoefficient = linear;
		mQuadraticAttenutaionCoefficient = quadratic;
	}

	void setSpotlightRange( const float &innerAngle, const float &outerAngle, const float& falloff )
	{
		mFalloffFactor = falloff;

		mUmbraAngle = innerAngle * Utils::RT_PI / 180.0f;
		mPenumbraAngle = outerAngle * Utils::RT_PI / 180.0f;
		mHalfUmbraAngleCosine = cosf( mUmbraAngle / 2.0f );
		mHalfPenumbraAngleCosine = cosf( mPenumbraAngle / 2.0f );
	}
};
