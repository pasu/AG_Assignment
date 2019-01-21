#include "precomp.h"
#include "RTMaterialManager.h"


RTMaterialManager::RTMaterialManager()
{
}

RTMaterialManager::~RTMaterialManager()
{
	ClearAll();
}

RTMaterial &RTMaterialManager::CreateMaterial( const vec3 &color, const ShadingType shadingType ) 
{
	return CreateMaterial( color, vec3( 0 ), 0, vec2( 1 ), shadingType, 0.8f, 1.0f );
}

RTMaterial &RTMaterialManager::CreateMaterial( const RTTexture *albedo, const ShadingType shadingType ) 
{
	return CreateMaterial( vec3( 1.0f ), vec3( 0 ), albedo, vec2( 1 ), shadingType, 0.8f, 1.0f );
}

RTMaterial &RTMaterialManager::CreateMaterial( const vec3 &color, const RTTexture *albedo, const ShadingType shadingType ) 
{
	return CreateMaterial( color, vec3( 0 ), albedo, vec2( 1 ), shadingType, 0.8f, 1.0f );
}

RTMaterial &RTMaterialManager::CreateMaterial( const vec3 &color, const RTTexture *albedo, const ShadingType shadingType, const float reflectionFactor ) 
{
	return CreateMaterial( color, vec3( 0 ), albedo, vec2( 1 ), shadingType, reflectionFactor, 1.0f );
}

RTMaterial &RTMaterialManager::CreateMaterial( const vec3 &color, const vec3 &emission, const RTTexture *albedo, const vec2 &textureScale, const ShadingType shadingType, const float reflectionFactor, const float indexOfRefraction )
{
	int code = getHashCode( color, emission, albedo, textureScale, shadingType, reflectionFactor, indexOfRefraction );

	RTMaterial *pMaterial = NULL;

	std::unordered_map<int, RTMaterial* >::const_iterator got = mMaterials.find( code );

	if ( got == mMaterials.end() )
	{
		pMaterial = new RTMaterial( color, emission, albedo, textureScale, shadingType, reflectionFactor, indexOfRefraction );
		mMaterials.insert( {code, pMaterial} );

		return *pMaterial;
	}
	else
	{
		pMaterial = got->second;
		return *pMaterial;
	}
}

RTMaterial &RTMaterialManager::CreateMaterial( const vec3 &color, const vec3 &emission, const ShadingType shadingType )
{
	return CreateMaterial( color, emission, 0, vec2( 1 ), shadingType, 0.8f, 1.0f );
}

RTMaterial &RTMaterialManager::CreateMaterial( const vec3 &color, const RTTexture *albedo, const vec2 &textureScale, const ShadingType shadingType, const float reflectionFactor, const float indexOfRefraction )
{
	return CreateMaterial( color, vec3( 0 ), albedo, textureScale, shadingType, reflectionFactor, indexOfRefraction );
}

void RTMaterialManager::ClearAll()
{
	for ( auto it = mMaterials.begin(); it != mMaterials.end(); ++it )
	{
		RTMaterial *pMaterial = it->second;
		delete pMaterial;
		pMaterial = NULL;
	}

	mMaterials.clear();
}

int RTMaterialManager::getHashCode( const vec3 &color, const vec3 &emission, const RTTexture *albedo, const vec2 &textureScale, const ShadingType shadingType, const float reflectionFactor, const float indexOfRefraction ) const
{
	std::hash<std::string> h;

	char buffer[64];
	sprintf( buffer, "%.2f,%.2f,%.2f;%.2f,%.2f,%.2f;%d;%.2f,%.2f;%d;%.2f;%.2f", 
		color.x, color.y, color.z, emission.x, emission.y, emission.z, albedo, textureScale.x, textureScale.y, (int)shadingType, reflectionFactor, indexOfRefraction );

	int code = h( buffer );
	return code;
}
