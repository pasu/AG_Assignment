#include "precomp.h"
#include "RTTextureManager.h"


RTTextureManager::RTTextureManager()
{

}

RTTextureManager::~RTTextureManager()
{
	ClearAll();
}

RTTexture *RTTextureManager::CreateTexture( string strFileName, bool bMipMap /*= false*/, int nLevelNum /*= 8*/, float dis /*= 6.0f */ )
{
	RTTexture *pTexture = NULL;

	std::unordered_map<std::string, RTTexture*>::const_iterator got = mTextures.find( strFileName );
	
	if (got == mTextures.end())
	{
		pTexture = new RTTexture();
		pTexture->LoadTextureImage( strFileName.c_str() );
		if (bMipMap)
		{
			pTexture->generateMipmap( nLevelNum, dis );
		}
		
		mTextures.insert( {strFileName, pTexture} );
	}
	else
	{
		pTexture = got->second;
	}

	return pTexture;
}

RTChessBoardTexture *RTTextureManager::CreateChessBoardTexture( const vec3 &color1, const vec3 &color2 )
{
	char buffer[64];
	sprintf( buffer, "%.2f,%.2f,%.2f;%.2f,%.2f,%.2f",
			 color1.x, color1.y, color1.z, color2.x, color2.y, color2.z );

	RTTexture *pTexture = NULL;

	std::unordered_map<std::string, RTTexture *>::const_iterator got = mTextures.find( buffer );

	if ( got == mTextures.end() )
	{
		pTexture = new RTChessBoardTexture( color1, color2);

		mTextures.insert( {buffer, pTexture} );
	}
	else
	{
		pTexture = got->second;
	}

	return (RTChessBoardTexture*)pTexture;
}

void RTTextureManager::ClearAll()
{
	for ( auto it = mTextures.begin(); it != mTextures.end(); ++it )
	{
		RTTexture *pTexture = it->second;
		delete pTexture;
		pTexture = NULL;
	}

	mTextures.clear();
}
