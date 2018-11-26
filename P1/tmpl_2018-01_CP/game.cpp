#include "precomp.h" // include (only) this in every .cpp file
#include "RenderOptions.h"
#include "Scene.h"
#include "RayTracer.h"
#include "RTPlane.h"
#include "RTSphere.h"
#include "RTBox.h"
#include "RTObjMesh.h"
#include "RTTorus.h"
// -----------------------------------------------------------
// Initialize the application
// -----------------------------------------------------------

Scene scene( vec3( 0.1f ), vec3( 43.0f/255.0f, 203.0f / 255.0f, 246.0f/255.0f ) );
RenderOptions renderOptions;

RTMaterial redMaterial( vec3( 1, 0, 0 ), DIFFUSE );

RTMaterial whiteReflectiveMaterial( vec3( 0, 1, 1 ), REFLECTIVE );

RTTexture *floorTexture = new RTTexture();
RTTexture *droplet = new RTTexture();

RTMaterial whiteMaterial( vec3( 1, 1, 1 ), floorTexture, DIFFUSE );
RTMaterial transmissiveMaterial( droplet, DIFFUSE_AND_REFLECTIVE );

RTMaterial bmwMaterial( vec3( 1, 1, 1 ), TRANSMISSIVE_AND_REFLECTIVE );

void Game::Init()
{
	
	renderOptions.fov = 51.52f;
	renderOptions.width = 800;
	renderOptions.height = 800;
	renderOptions.maxRecursionDepth = 8;
	renderOptions.shadowBias = 0.001f;

	pTracer = new RayTracer( scene, renderOptions );
	//////////////////////////////////////////////////////////////////////////
	floorTexture->LoadImage("./assets/floor_diffuse.PNG");
	droplet->LoadImage( "./assets/BeachStones.jpg" );

	whiteMaterial.textureScale.x = 0.1f;
	whiteMaterial.textureScale.y = 0.1f;

	transmissiveMaterial.textureScale.x = 1.010f;
	transmissiveMaterial.textureScale.y = 1.010f;
	transmissiveMaterial.reflectionFactor = 0.3f;

	whiteReflectiveMaterial.reflectionFactor = 0.4f;
	bmwMaterial.indexOfRefraction = 1.2;
	
	/////////////////////////////////////////////////////////////////////////
	RTLight* pLight = RTLight::createPointLight( vec3( 1.0f, 1.0f, 1.0f ), 500.0f, vec3( 00.0f, 15.0f, -40.0f ) );
	RTLight *pLight2 = RTLight::createPointLight( vec3( 1.0f, 1.0f, 1.0f ), 500.0f, vec3( 10.0f, 0.0f, 0.0f ) );
	scene.addLight( pLight );
	scene.addLight( pLight2 );
	scene.addLight(RTLight::createParralleLight(vec3(1.0f,1.0f,1.0f),0.5f,vec3(0.707,-0.707,0)));

	RTTorus *pTorus = new RTTorus( vec3( 1.0f, 2.0f, -10.0f ), vec3( 0.0f, 1.0f, 1.0f ), 2.0f, 4.0f, redMaterial );
	scene.addObject( pTorus );

	RTObjMesh *mesh = new RTObjMesh( "./assets/Cup_Made_By_Tyro_Smith.obj", bmwMaterial );
	mesh->setPosition( -7.0f, -5.0f, -5.0f );
	mesh->setRotation( 0.0f, 0.0f, 0.0f );
	mesh->setScale( 0.5f, 0.5, 0.5f );
	mesh->applyTransforms();
	//scene.addObject( mesh );

	scene.addObject( new RTSphere( vec3( 15.0f, 0.0f, -20.0f ), 5, redMaterial ) );
	scene.addObject( new RTSphere( vec3( -5.0f, 5.0f, -20.0f ), 5.0f, bmwMaterial ) );

	scene.addObject( new RTBox( vec3( 5.0f, 0.0f, -55.0f ), vec3( 20.0f, 20.0f, 10.0f ), whiteReflectiveMaterial ) );

	scene.addObject( new RTPlane( vec3( 0.1f, -9.0f, -10.0f ), vec3( 0.0f, 1.0f, 0.0f ), vec3( 1.0f, 0.0f, 0.0f ), whiteMaterial ));
	//auto s = scene.getObjects()[1]->getMaterial().shadingType;
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
static DWORD tt = 0;
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
	if (++frame == 10) {
		DWORD nt = GetTickCount();
		printf( "%.2f\n", 10000.0f / ( nt - tt ) );
		tt = nt;
		frame = 0;
	}
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
