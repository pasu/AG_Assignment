#pragma once
#include "Utils.h"
#include "precomp.h"
class RTCamera
{
  public:
	vec3 getEye()
	{
		return eye;
	}
	void setEye( vec3 e )
	{
		eye = e;
	}

	void setFov(const float& angle)
	{
		float y = 1.0f;
		float x = tan( angle / Utils::RT_PI );

		tanFovHalf = vec2( x, y );
	}

	RTCamera()
	{
		// 		eye = vec3( 0, 0, 5 );
		// 		ahead = vec3( 0, 0, -1 );
		// 		right = vec3( 1, 0, 0 );
		// 		up = vec3( 0, 1, 0 );
		tanFovHalf = vec2( 1, 1 );
		//rotationM = mat4::rotatex( Utils::RT_PI );
		//mat4 translateM;
		//rotationM = translateM * rotationM;
		heading = .0f;
		pitch = .0f;
		bUpdate = true;
		position = vec4( 0 );
		position.w = 1.0f;

		updateRotationMatrix();
		Update();

		// depth field
		float factor = 0.1;
		aperture = 0;
		d = factor*SCRWIDTH / 2;
		viewplane_size = factor*SCRWIDTH;

		viewplane_center = eye + (float)d * ahead;
		pixel_size = (float)( viewplane_size / (float)SCRWIDTH );
		strata_size = pixel_size / (float)SAMPLE_NUM;
		left_up_corner = vec3( (float)SCRWIDTH * this->pixel_size * -0.5f,
							   (float)SCRHEIGHT * this->pixel_size * 0.5f,
							   (float)this->viewplane_center.z );
	}

	vec3 rayDirFromNdc( const vec2 ndc )
	{
		return vec3::normalize( ahead + right * ndc.x * tanFovHalf.x + up * ndc.y * tanFovHalf.y );
	}

	void updateRotationMatrix()
	{
		mat4 mHeading, mPitch;
		mHeading=mat4::rotatey( heading );
		mPitch=mat4::rotatex( pitch );
		mRotation = mPitch*mHeading;
		bUpdate = true;
	}

	void turnLeft(float rad) 
	{
		heading += rad;
		updateRotationMatrix();
	}
	void turnUp( float rad )
	{
		pitch += rad;
		pitch = std::max( pitch, -1.57f );
		pitch = std::min( pitch, 1.57f );
		updateRotationMatrix();
	}
	void moveForward(float d)
	{
		vec4 localD( 0.0f, 0.0f, d, 1.0f );
		vec4 globalD = mat4::rotatey(heading)*localD;
		position.x += globalD.x;
		position.z -= globalD.z;
		bUpdate = true;
	}
	void moveLeft( float d )
	{
		vec4 localD( -d, 0.0f, 0.0f, 1.0f );
		vec4 globalD = mat4::rotatey( heading ) * localD;
		position.x += globalD.x;
		position.z -= globalD.z;
		bUpdate = true;
	}
	void moveUp( float d )
	{
		position.y += d;
		bUpdate = true;
	}
	void Update()
	{
		
		if ( bUpdate )
		{

			eye = position.xyz;

			vec4 aixX = vec4( 1.0f, 0.0f, 0.0f, 0.0f ) * mRotation;
			vec4 aixY = vec4( 0.0f, 1.0f, 0.0f, 0.0f ) * mRotation;
			vec4 aixZ = vec4( 0.0f, 0.0f, 1.0f, 0.0f ) * mRotation;

			right.x = aixX.x;
			right.y = aixX.y;
			right.z = -aixX.z;

			up.x = aixY.x;
			up.y = aixY.y;
			up.z = -aixY.z;

			ahead.x = aixZ.x;
			ahead.y = aixZ.y;
			ahead.z = -aixZ.z;

			bUpdate = false;
		}
	}

	bool bNeedUpdate() const
	{
		return bUpdate;
	}

  public:
	vec3 eye;
	vec3 ahead;
	vec3 right;
	vec3 up;

	vec2 tanFovHalf;

	bool bUpdate;

	float heading;
	float pitch;
	mat4 mRotation;
	vec4 position;

	// Depth Field
	float d;
	float aperture;
	float viewplane_size;
	vec3 viewplane_center;
	vec3 left_up_corner;
	float pixel_size;
	float strata_size;

};
