#pragma once
#include "RTTexture.h"
#include "RTChessBoardTexture.h"

#include <string>
#include <unordered_map>

using namespace std;

class RTTextureManager
{
  public:
	RTTextureManager();
	~RTTextureManager();

	RTTexture* CreateTexture( string strFileName, bool bMipMap = false, int nLevelNum = 8,float dis = 6.0f ) ;
	RTChessBoardTexture *CreateChessBoardTexture( const vec3 &color1, const vec3 &color2 );

	void ClearAll();
  private:
	unordered_map<std::string, RTTexture *> mTextures;

};
