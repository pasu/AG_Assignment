#include "precomp.h" // include (only) this in every .cpp file
#include "RenderOptions.h"
#include "Scene.h"
#include "RayTracer.h"
#include "RTPlane.h"
#include "RTSphere.h"
// -----------------------------------------------------------
// Initialize the application
// -----------------------------------------------------------

Scene scene( vec3( 0.1f ), vec3( 43.0f/255.0f, 203.0f / 255.0f, 246.0f/255.0f ) );
RenderOptions renderOptions;

RTMaterial whiteMaterial( vec3( 1, 1, 1 ), DIFFUSE );

RTMaterial redMaterial( vec3( 1, 0, 0 ), DIFFUSE );

RTMaterial whiteReflectiveMaterial( vec3( 1, 1, 1 ), REFLECTIVE );

void Game::Init()
{
	
	renderOptions.fov = 51.52f;
	renderOptions.width = 800;
	renderOptions.height = 800;
	renderOptions.maxRecursionDepth = 8;
	renderOptions.shadowBias = 0.001f;

	pTracer = new RayTracer( scene, renderOptions );

	/////////////////////////////////////////////////////////////////////////
	RTLight* pLight = RTLight::createPointLight( vec3( 1.0f, 1.0f, 1.0f ), 500.0f, vec3( 00.0f, 15.0f, -40.0f ) );
	scene.addLight( pLight );
	scene.addLight(RTLight::createParralleLight(vec3(1.0f,1.0f,1.0f),0.5f,vec3(0.707,-0.707,0)));

	scene.addObject( new RTSphere( vec3( 0.0f, 0.0f, -40.0f ), 10.0f, redMaterial ) );
	scene.addObject( new RTPlane( vec3( 0.1f, -9.0f, -10.0f ), vec3( 0.0f, 1.0f, 0.0f ), whiteMaterial ));
	auto s = scene.getObjects()[1]->getMaterial().shadingType;
	///////////////////////////////////////////////////////////////////////////////

}

// -----------------------------------------------------------
// Close down application
// -----------------------------------------------------------
void Game::Shutdown()
{
}

static Sprite rotatingGun( new Surface( "assets/aagun.tga" ), 36 );
static int frame = 0;

// -----------------------------------------------------------
// Main application tick function
// -----------------------------------------------------------
void Game::Tick( float deltaTime )
{
	// clear the graphics window
//	screen->Clear( 0 );
	// print something in the graphics window
	pTracer->render( screen );

	screen->Print( "hello world", 2, 2, 0xffffff );
	// print something to the text window
// 	printf( "this goes to the console window.\n" );
// 	// draw a sprite
// 	rotatingGun.SetFrame( frame );
// 	rotatingGun.Draw( screen, 100, 100 );
	if ( ++frame == 36 ) frame = 0;
}

void Tmpl8::Game::KeyDown( int key )
{
	switch ( key )
	{
	case 4://A
		scene.getCamera()->translateX( -1.0f );
		break;
	case 7: //D
		scene.getCamera()->translateX( 1.0f );
		break;
	case 22: //S
		scene.getCamera()->translateY( -1.0f );
		break;
	case 26: //W
		scene.getCamera()->translateY( 1.0f );
		break;
	default:
		break;
	}
}

void Tmpl8::Game::MouseWheel( int y )
{
	scene.getCamera()->translateZ( y );
}
