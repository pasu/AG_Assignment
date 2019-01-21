#pragma once

#include "RTMaterial.h"

#include <string>
#include <unordered_map>
class RTMaterialManager
{
  public:
	RTMaterialManager();
	~RTMaterialManager();

	RTMaterial& CreateMaterial( const vec3 &color, const ShadingType shadingType) ;
	RTMaterial &CreateMaterial( const vec3 &color, const vec3 &emission, const ShadingType shadingType );
	RTMaterial &CreateMaterial( const RTTexture *albedo, const ShadingType shadingType ) ;
	RTMaterial &CreateMaterial( const vec3 &color, const RTTexture *albedo, const ShadingType shadingType ) ;
	RTMaterial &CreateMaterial( const vec3 &color, const RTTexture *albedo, const ShadingType shadingType, const float reflectionFactor ) ;
	RTMaterial &CreateMaterial( const vec3 &color, const RTTexture *albedo, const vec2 &textureScale, const ShadingType shadingType, const float reflectionFactor, const float indexOfRefraction );

	RTMaterial &CreateMaterial( const vec3 &color, const vec3 &emission, const RTTexture *albedo, const vec2 &textureScale, const ShadingType shadingType, const float reflectionFactor, const float indexOfRefraction );

	void ClearAll();
	private:
	int getHashCode( const vec3 &color, const vec3 &emission, const RTTexture *albedo, const vec2 &textureScale, const ShadingType shadingType, const float reflectionFactor, const float indexOfRefraction ) const;
	private:
	unordered_map<int, RTMaterial*> mMaterials;
};
