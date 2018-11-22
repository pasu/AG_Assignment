#pragma once
#include "precomp.h"
#include "Utils.h"
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

	void translateX(const float& dis)
	{
		mat4 translateM;
		translateM.cell[3] = dis;
		rotationM = translateM * rotationM;
		bUpdate = true;
	}

	void translateY( const float &dis )
	{
		mat4 translateM;
		translateM.cell[7] = dis;
		rotationM = translateM * rotationM;
		bUpdate = true;
	}

	void translateZ( const float &dis )
	{
		mat4 translateM;
		translateM.cell[11] = dis;
		rotationM = translateM * rotationM;
		bUpdate = true;
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
		rotationM.cell[11] = -5;
		//rotationM = translateM * rotationM;

		Update();
		
	}

	vec3 rayDirFromNdc( const vec2 ndc )
	{
		return vec3::normalize( ahead + right * ndc.x * tanFovHalf.x + up * ndc.y * tanFovHalf.y );
	}

	void Update()
	{
		if (bUpdate)
		{
			vec4 pos = vec4( 0.0f, 0.0f, 0.0f, 1.0f ) * rotationM;
			eye.x = pos.x;
			eye.y = pos.y;
			eye.z = -pos.z;

			vec4 aixX = vec4( 1.0f, 0.0f, 0.0f, 0.0f ) * rotationM;
			vec4 aixY = vec4( 0.0f, 1.0f, 0.0f, 0.0f ) * rotationM;
			vec4 aixZ = vec4( 0.0f, 0.0f, 1.0f, 0.0f ) * rotationM;

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

private:

	vec3 eye;
	vec3 ahead;
	vec3 right;
	vec3 up;

	vec2 tanFovHalf;

	mat4 rotationM;
	bool bUpdate;
};
