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
