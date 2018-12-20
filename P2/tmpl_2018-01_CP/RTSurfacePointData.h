#pragma once

struct SurfacePointData
{
	vec3 normal;
	vec2 textureCoordinates;
	vec3 position;

	//Extra stuff
	vec3 tangent;
	vec3 bitangent;
};
