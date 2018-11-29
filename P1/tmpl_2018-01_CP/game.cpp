#include "precomp.h" // include (only) this in every .cpp file
#include "RenderOptions.h"
#include "Scene.h"
#include "RayTracer.h"
#include "RTPlane.h"
#include "RTSphere.h"
#include "RTBox.h"
#include "RTObjMesh.h"
#include "RTTorus.h"
#include "RTCone.h"
#include "RTCameraController.h"
// -----------------------------------------------------------
// Initialize the application
// -----------------------------------------------------------

Scene scene( vec3( 0.1f ), vec3( 43.0f/255.0f, 203.0f / 255.0f, 246.0f/255.0f ) );
RenderOptions renderOptions;

RTTexture *floorTexture = new RTTexture();
RTTexture *boxTexture = new RTTexture();
RTTexture *torusTexture = new RTTexture();
RTTexture *meshTexture = new RTTexture();

RTMaterial redspehreMaterial( vec3( 1, 0, 0 ), DIFFUSE_AND_REFLECTIVE );
RTMaterial yellowspehreMaterial( vec3(1, 1, 0 ), DIFFUSE_AND_REFLECTIVE );

RTMaterial boxReflectiveMaterial( vec3( 1, 1, 1 ), boxTexture, DIFFUSE_AND_REFLECTIVE );
//RTMaterial boxwithTextureMaterial( vec3( 0, 1, 1 ), floorTexture, DIFFUSE );

RTMaterial floorMaterial( vec3( 1, 1, 1 ), floorTexture, DIFFUSE_AND_REFLECTIVE );
RTMaterial mirrorWallMaterial( vec3( 1, 1, 1 ), REFLECTIVE );

RTMaterial coneMaterial( vec3( 1, 1, 1 ), boxTexture, DIFFUSE );

RTMaterial torusMaterial( vec3( 1, 1, 1 ), torusTexture, DIFFUSE );
RTMaterial sphereMaterial( vec3( 1, 1, 1 ), TRANSMISSIVE_AND_REFLECTIVE );

RTMaterial meshMaterial( vec3( 1, 1, 1 ), meshTexture,DIFFUSE );

void Game::Init()
{
	renderOptions.fov = 51.52f;
	renderOptions.width = 800;
	renderOptions.height = 800;
	renderOptions.maxRecursionDepth = 8;
	renderOptions.shadowBias = 0.01f;

	pTracer = new RayTracer( scene, renderOptions );
	//////////////////////////////////////////////////////////////////////////
	floorTexture->LoadTextureImage("./assets/floor_diffuse.PNG");
	floorTexture->generateMipmap( 8, 6.0f );
	boxTexture->LoadTextureImage( "./assets/box.png" );
	torusTexture->LoadTextureImage( "./assets/BumpyMetal.jpg" );
	meshTexture->LoadTextureImage( "./assets/Cesium_Man.jpg" );
	floorMaterial.textureScale.x = 0.1f;
	floorMaterial.textureScale.y = 0.1f;
	floorMaterial.reflectionFactor = 0.3f;

	coneMaterial.textureScale.x = 0.1f;
	coneMaterial.textureScale.y = 0.1f;

	torusMaterial.textureScale.x = 64.0f;
	torusMaterial.textureScale.y = 64.0f;

	mirrorWallMaterial.reflectionFactor = 1.0f;

	redspehreMaterial.reflectionFactor = 0.4f;
	yellowspehreMaterial.reflectionFactor = 0.4f;

	boxReflectiveMaterial.reflectionFactor = 0.2f;
	torusMaterial.indexOfRefraction = 2.417f;
	sphereMaterial.indexOfRefraction = 2.417f;
	/////////////////////////////////////////////////////////////////////////
	vec3 posL1 = vec3( 00.0f, 40.0f, -5.0f );
	vec3 posL2 = vec3( 10.0f, 10.0f, -40.0f );
	vec3 posL3 = vec3( -10.0f, 10.0f, -40.0f );
	vec3 posL4 = vec3( 0.0f, 0.0f, 10.0f );
	RTLight* pLight = RTLight::createPointLight( vec3( 1.0f, 1.0f, 1.0f ), 1500.0f,  posL1);
	RTLight *pLight2 = RTLight::createPointLight( vec3( 1.0f, .0f, .0f ), 1000.0f, posL2 );
	RTLight *pLight3 = RTLight::createPointLight( vec3( .0f, 1.0f, .0f ), 500.0f, posL3 );
	RTLight *pLight4 = RTLight::createPointLight( vec3( 1.0f, 1.0f, 1.0f ), 500.0f, posL4 );

	scene.addLight( pLight );
	scene.addLight( pLight2 );
	scene.addLight( pLight3 );
	//scene.addLight( pLight4 );
	scene.addLight(RTLight::createParralleLight(vec3(1.0f,1.0f,1.0f),0.5f,vec3(0.707,-0.707,0)));

	RTPlane* plane1 = new RTPlane( vec3( 0.0f, -11.5f, -10.0f ), vec3( 0.0f, 1.0f, 0.0f ), vec3( 1.0f, 0.0f, 0.0f ), floorMaterial );
	RTPlane *plane2 = new RTPlane( vec3( 0.0f, 0.0f, -100.0f ), vec3( 0.0f, 0.0f, 1.0f ), vec3( 1.0f, 0.0f, 0.0f ), mirrorWallMaterial );

	

	RTBox *box = new RTBox( vec3( 0.0f, 0.0f, -85.0f ), vec3( 20.0f, 20.0f, 10.0f ),
							boxReflectiveMaterial );
	

	RTSphere *pSphere1 = new RTSphere( vec3( -15.0f, 0.0f, -40.0f ), 5, redspehreMaterial );
	

	RTSphere *pSphere2 = new RTSphere( vec3( 15.0f, 0.0f, -40.0f ), 5.0f, yellowspehreMaterial );
	

	RTCone *pCone = new RTCone( vec3(15,25,-30), vec3(15,20,-30), 5, coneMaterial );
	

	RTTorus *pTorus = new RTTorus( vec3( -15.0f, 10.0f, -20.0f ), vec3( 0.0f, 1.0f, 2.0f ), 2.0f, 4.0f,
								   torusMaterial );
	

 	RTSphere *pSphere3 = new RTSphere( vec3( 0.0f, 0.0f, 0.0f ), 3.0f, sphereMaterial );
// 	
// 
// 	RTSphere *pSphere4 = new RTSphere( vec3( 0.0f, 0.0f, -5.0f ), 2.0f, sphereMaterial );
// 	//scene.addObject( pSphere4 );
// 
// 	RTSphere *pSphere5 = new RTSphere( vec3( 0.0f, 5.0f, -10.0f ), 1.0f, sphereMaterial );
// 	//scene.addObject( pSphere5 );


	RTObjMesh *mesh = new RTObjMesh( "./assets/Cesium_Man.dae", meshMaterial );
	mesh->setPosition( 0.0f, 3.0f, -10.0f );
	mesh->setRotation( -Utils::RT_PI / 2.0, -Utils::RT_PI / 2.0, 0.0f );
	mesh->setScale( 5.3f, 5.3f, 5.3f );
	//mesh->applyTransforms();

	scene.addObject( plane1 );
	scene.addObject( plane2 );

	scene.addObject( box );

	scene.addObject( pSphere1 );
	scene.addObject( pSphere2 );

	//scene.addObject( pCone );
 	//scene.addObject( pTorus );

	//scene.addObject( pSphere3 );

	//scene.addObject( mesh );

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
	updateCamera(*(scene.getCamera()));
	
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

}

void Tmpl8::Game::MouseWheel( int y )
{
}
