#pragma once
#include "RayTracer.h"

namespace Tmpl8 {

class Game
{
public:
	void SetTarget( Surface* surface ) { screen = surface; }
	void Init();
	void Shutdown();
	void Tick( float deltaTime );
	void MouseUp( int button ) { /* implement if you want to detect mouse button presses */ }
	void MouseDown( int button ) { /* implement if you want to detect mouse button presses */ }
	void MouseMove( int x, int y ) { /* implement if you want to detect mouse movement */ }
	void KeyUp( int key ) { /* implement if you want to handle keys */ }
	void KeyDown( int key );
	void MouseWheel( int y );

	//void scene_default();
	//void scene_fresnel_beer();
	//void scene_light();
	void scene_tw();

  private:
	Surface* screen;
	RayTracer *pTracer;

	char buffer[128];
};

}; // namespace Tmpl8