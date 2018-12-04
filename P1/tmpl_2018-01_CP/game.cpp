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

#include "RTTextureManager.h"
#include "RTMaterialManager.h"
#include "RTChessBoardTexture.h"
// -----------------------------------------------------------
// Initialize the application
// -----------------------------------------------------------
RTTextureManager gTexManager;
RTMaterialManager gMaterialManager;

Scene scene( vec3( 0.1f ), vec3( 43.0f/255.0f, 203.0f / 255.0f, 246.0f/255.0f ) );
RenderOptions renderOptions;
RTCamera *pCamera = scene.getCamera();


void Game::Init()
{
	renderOptions.width = SCRWIDTH;
	renderOptions.height = SCRHEIGHT;
	renderOptions.maxRecursionDepth = 4;
	renderOptions.shadowBias = 0.01f;

	pTracer = new RayTracer( scene, renderOptions );

	//pCamera->turnLeft( -Utils::RT_PI / 2.0f );
	//scene_default();
	//scene_fresnel_beer();
	//scene_light();
	scene_tw();
	//gTexManager.CreateTexture( "./assets/floor_diffuse.png", true, 8, 6.0f );
// 	gTexManager.ClearAll();
// 	gMaterialManager.ClearAll();
// 	scene.ClearAllLight();
// 	scene.ClearAllObj();
// 	scene_default();
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
	// print something to the text window
// 	printf( "this goes to the console window.\n" );
// 	// draw a sprite
// 	rotatingGun.SetFrame( frame );
// 	rotatingGun.Draw( screen, 100, 100 );
	if (++frame == 10) {
		DWORD nt = GetTickCount();

		sprintf( buffer, "FPS: %.2f", 10000.0f / ( nt - tt ) );

		tt = nt;
		frame = 0;
	}

	screen->Print( buffer, 2, 2, 0xffffff );
}

void Tmpl8::Game::KeyDown( int key )
{
	static int index = 0;
	static int scenecount = 4;
	if (key == 79)
	{
		gTexManager.ClearAll();
		gMaterialManager.ClearAll();
		scene.ClearAllLight();
		scene.ClearAllObj();

		index++; 
		switch ( index % scenecount )
		{
		case 0:
			scene_default();
			break;
		case 1:
			scene_fresnel_beer();
			break;
		case 2:
			scene_light();
			break;
		case 3:
			scene_tw();
			break;
		}
	}
	
	if ( key == SDL_SCANCODE_LEFT )
	{
		gTexManager.ClearAll();
		gMaterialManager.ClearAll();
		scene.ClearAllLight();
		scene.ClearAllObj();

		index--;
		switch ( index % scenecount )
		{
		case 0:
			scene_default();
			break;
		case 1:
			scene_fresnel_beer();
			break;
		case 2:
			scene_light();
			break;
		case 3:
			scene_tw();
			break;
		}
	}

}

void Tmpl8::Game::MouseWheel( int y )
{
}

void Tmpl8::Game::scene_default()
{
	scene.ambientLight = .4f;
	scene.backgroundColor = vec3( 8.0f / 255.0f, 165.0f / 255.0f, 211.0f / 255.0f );
	RTTexture *floorTexture = gTexManager.CreateTexture( "./assets/floor_diffuse.png", true, 8, 6.0f );
	RTTexture *boxTexture = gTexManager.CreateTexture( "./assets/box.png" );
	RTTexture *torusTexture = gTexManager.CreateTexture( "./assets/BumpyMetal.jpg" );
	torusTexture->generateMipmap( 8, 5 );
	RTTexture *meshTexture = gTexManager.CreateTexture( "./assets/Cesium_Man.jpg" );

	//RTMaterial &redspehreMaterial = gMaterialManager.CreateMaterial( vec3( 1, 0, 0 ), DIFFUSE_AND_REFLECTIVE );
	RTMaterial *redsphereMaterial_p = new RTMaterial( vec3( 1, 0, 0 ), DIFFUSE | REFLECT, vec3( 0.6, 0.4, 0.0 ) );
	RTMaterial& redsphereMaterial = *redsphereMaterial_p;

	//RTMaterial &yellowspehreMaterial = gMaterialManager.CreateMaterial( vec3( 1, 1, 0 ), DIFFUSE_AND_REFLECTIVE );
	RTMaterial *yellowspehreMaterial_p = new RTMaterial( vec3( 1, 1, 0 ), DIFFUSE | REFLECT, vec3( 0.6, 0.4, 0.0 ) );
	RTMaterial &yellowspehreMaterial = *yellowspehreMaterial_p;

	//RTMaterial &boxReflectiveMaterial = gMaterialManager.CreateMaterial( vec3( 1, 1, 1 ), boxTexture, DIFFUSE_AND_REFLECTIVE );
	RTMaterial *boxReflectiveMaterial_p = new RTMaterial( boxTexture, DIFFUSE | REFLECT, vec3( 0.8, 0.2, 0.0 ) );
	RTMaterial &boxReflectiveMaterial = *boxReflectiveMaterial_p;

	//RTMaterial &floorMaterial = gMaterialManager.CreateMaterial( vec3( 1, 1, 1 ), floorTexture, DIFFUSE_AND_REFLECTIVE );
	RTMaterial *floorMaterial_p = new RTMaterial( floorTexture, DIFFUSE | REFLECT, vec3( 0.7, 0.3, 0.0 ), vec2( 0.1, 0.1 ) );
	RTMaterial &floorMaterial = *floorMaterial_p;

	//RTMaterial &mirrorWallMaterial = gMaterialManager.CreateMaterial( vec3( 1, 1, 1 ), REFLECTIVE );
	RTMaterial *mirrorWallMaterial_p = new RTMaterial( vec3( 1, 1, 1 ), REFLECT, vec3( 0, 1, 0 ) );
	RTMaterial &mirrorWallMaterial = *mirrorWallMaterial_p;

	//RTMaterial &coneMaterial = gMaterialManager.CreateMaterial( vec3( 1, 1, 1 ), boxTexture, DIFFUSE );
	RTMaterial *coneMaterial_p = new RTMaterial( boxTexture, DIFFUSE, vec3( 1, 0, 0 ), vec2( 0.1, 0.1 ) );
	RTMaterial &coneMaterial = *coneMaterial_p;

	//RTMaterial &torusMaterial = gMaterialManager.CreateMaterial( vec3( 1, 1, 1 ), torusTexture, DIFFUSE );
	RTMaterial *torusMaterial_p = new RTMaterial( torusTexture, DIFFUSE, vec3( 1, 0, 0 ), vec2( 64, 64 ), 2.417 );
	RTMaterial &torusMaterial = *torusMaterial_p;

	//RTMaterial &sphereMaterial = gMaterialManager.CreateMaterial( vec3( 1, 1, 1 ), TRANSMISSIVE_AND_REFLECTIVE );
	RTMaterial *sphereMaterial_p = new RTMaterial( vec3( 1, 1, 1 ), REFRACT, vec3( 0, 0, 1 ), 2.417 );
	RTMaterial &sphereMaterial = *sphereMaterial_p;

	//RTMaterial &meshMaterial = gMaterialManager.CreateMaterial( vec3( 1, 1, 1 ), meshTexture, DIFFUSE );
	RTMaterial *meshMaterial_p = new RTMaterial( meshTexture, DIFFUSE, vec3( 1, 0, 0 ) );
	RTMaterial &meshMaterial = *meshMaterial_p;
	/////////////////////////////////////////////////////////////////////////
	scene.ambientLight = 0.1f;
	vec3 posL1 = vec3( 00.0f, 40.0f, -25.0f );
	vec3 posL2 = vec3( 10.0f, 10.0f, -60.0f );
	vec3 posL3 = vec3( -10.0f, 10.0f, -60.0f );
	vec3 posL4 = vec3( 0.0f, 0.0f, -10.0f );
	RTLight *pLight = RTLight::createPointLight( vec3( 1.0f, 1.0f, 1.0f ), 1500.0f, posL1 );
	RTLight *pLight2 = RTLight::createPointLight( vec3( 1.0f, 1.0f, 1.0f ), 1000.0f, posL2 );
	RTLight *pLight3 = RTLight::createPointLight( vec3( 1.0f, 1.0f, 1.0f ), 500.0f, posL3 );
	//RTLight *pLight4 = RTLight::createPointLight( vec3( 1.0f, 1.0f, 1.0f ), 500.0f, posL4 );

	scene.addLight( pLight );
	scene.addLight( pLight2 );
	scene.addLight( pLight3 );
	//scene.addLight( pLight4 );
	scene.addLight( RTLight::createParralleLight( vec3( 1.0f, 1.0f, 1.0f ), 0.5f, vec3( 0.707, -0.707, 0 ) ) );

	RTPlane *plane1 = new RTPlane( vec3( 0.0f, -11.5f, -30.0f ), vec3( 0.0f, 1.0f, 0.0f ), vec3( 1.0f, 0.0f, 0.0f ), floorMaterial );
	RTPlane *plane2 = new RTPlane( vec3( 0.0f, 0.0f, -120.0f ), vec3( 0.0f, 0.0f, 1.0f ), vec3( 1.0f, 0.0f, 0.0f ), mirrorWallMaterial );

	RTBox *box = new RTBox( vec3( 0.0f, 0.0f, -105.0f ), vec3( 20.0f, 20.0f, 10.0f ),
							boxReflectiveMaterial );

	RTSphere *pSphere1 = new RTSphere( vec3( -15.0f, 0.0f, -60.0f ), 5, redsphereMaterial );

	RTSphere *pSphere2 = new RTSphere( vec3( 15.0f, 0.0f, -60.0f ), 5.0f, yellowspehreMaterial );

	RTCone *pCone = new RTCone( vec3( 15, 25, -50 ), vec3( 15, 20, -50 ), 5, coneMaterial );

	RTTorus *pTorus = new RTTorus( vec3( -15.0f, 10.0f, -40.0f ), vec3( 0.0f, 1.0f, 2.0f ), 2.0f, 4.0f,
								   torusMaterial );

	RTSphere *pSphere3 = new RTSphere( vec3( 0.0f, 2.0f, -20.0f ), 3.0f, sphereMaterial );

// 	RTObjMesh *mesh = new RTObjMesh( "./assets/Cesium_Man.dae", meshMaterial );
// 	mesh->setPosition( 0.0f, 2.0f, -10.0f );
// 	mesh->setRotation( -Utils::RT_PI / 2.0, -Utils::RT_PI / 2.0, 0.0f );
// 	//mesh->setScale( .03f, .03f,.03f );
// 	mesh->applyTransforms();

	scene.addObject( plane1 );
	scene.addObject( plane2 );

	scene.addObject( box );

	scene.addObject( pSphere1 );
	scene.addObject( pSphere2 );

	scene.addObject( pCone );
	scene.addObject( pTorus );

	scene.addObject( pSphere3 );

	//scene.addObject( mesh );
	///////////////////////////////////////////////////////////////////////////////
}

void Tmpl8::Game::scene_fresnel_beer()
{
	//////////////////////////////////////////////////////////////////////////
	RTTexture *floorTexture = gTexManager.CreateTexture( "./assets/floor_diffuse.png", true, 8, 6.0f );
	RTMaterial &floorMaterial = *new RTMaterial( floorTexture, DIFFUSE, vec3( 1, 0, 0 ) , vec2(0.1,0.1));

	RTTexture *wallTexture = gTexManager.CreateTexture( "./assets/1d_debug.png", false, 8, 6.0f );
	RTMaterial &wallMaterial = *new RTMaterial( wallTexture, DIFFUSE, vec3( 1, 0, 0 ), vec2(0.08,0.08) );

	RTMaterial &lensMaterial = *new RTMaterial( vec3( 1, 1, 1 ), REFRACT | REFLECT, vec3( 0, 0.1, 0.9 ), 1.217 );

	RTMaterial &sphereMaterial = *new RTMaterial( vec3( 1, 0, 0 ), REFRACT | REFLECT, vec3( 0, 0.1, 0.9 ), 1.2 );


	/////////////////////////////////////////////////////////////////////////
	scene.ambientLight = 1.0f;

	vec3 posL1 = vec3( 0.0f, 10.0f, -10.0f );
	RTLight *pLight = RTLight::createPointLight( vec3( 1.0f, 1.0f, 1.0f ), 2500.0f, posL1 );
	
	scene.addLight( pLight );


	RTPlane *plane1 = new RTPlane( vec3( 0.0f, -30.0f, -40.0f ), vec3( 0.0f, 1.0f, 0.0f ), vec3( 1.0f, 0.0f, 0.0f ), floorMaterial );
	RTPlane *plane2 = new RTPlane( vec3( 0.0f, 0.0f, -100.0f ), vec3( 0.0f, 0.0f, 1.0f ), vec3( 1.0f, 0.0f, 0.0f ), wallMaterial );

	RTPlane *plane3 = new RTPlane( vec3( 0.0f, -10.0f, -100.0f ), vec3( 0.0f, 1.0f, 0.0f ), vec3( 1.0f, 0.0f, 0.0f ), lensMaterial );

	RTSphere *pSphere1 = new RTSphere( vec3( -5.0f, 0.0f, -10.0f ), 1.0f, sphereMaterial );
	RTSphere *pSphere2 = new RTSphere( vec3( -1.0f, 0.0f, -10.0f ), 2.0f, sphereMaterial );
	RTSphere *pSphere3 = new RTSphere( vec3( 5.0f, 0.0f, -10.0f ), 3.0f, sphereMaterial );

	scene.addObject( plane1 );
	scene.addObject( plane2 );
	scene.addObject( plane3 );

	scene.addObject( pSphere1 );
	scene.addObject( pSphere2 );
	scene.addObject( pSphere3 );
	///////////////////////////////////////////////////////////////////////////////
}

void Tmpl8::Game::scene_light()
{
	//////////////////////////////////////////////////////////////////////////
	vec3 lightBlue( 190. / 255., 237. / 255., 1. );
	vec3 lightRed( 248. / 255., 192. / 255., 196. / 255 );
	RTChessBoardTexture *chessboardTexture = gTexManager.CreateChessBoardTexture( vec3( 1 ), vec3( 0 ) );

	RTMaterial &brownCheckerBoardMaterial =* new RTMaterial( chessboardTexture, DIFFUSE,vec3(1,0,0), vec2(0.2f));
		//gMaterialManager.CreateMaterial( vec3( 1 ), chessboardTexture, vec2( 0.2f ), DIFFUSE, 0.8f, 2.5f );
	RTMaterial &whiteMaterial = *new RTMaterial( vec3( 1 ), DIFFUSE, vec3( 1, 0, 0 ) );
		//gMaterialManager.CreateMaterial( vec3( 1 ), 0, vec2( 0.1f ), DIFFUSE, 0.8f, 2.5f );
	RTMaterial &redMaterial = *new RTMaterial( vec3( 1, 0, 0 ), DIFFUSE, vec3( 1, 0, 0 ) );
		//gMaterialManager.CreateMaterial( vec3( 1, 0, 0 ), 0, vec2( 0.05f ), DIFFUSE, 0.8f, 2.5f );
	RTMaterial &greenMaterial = *new RTMaterial( vec3( 0, 1, 0 ), DIFFUSE, vec3( 1, 0, 0 ) );
		//gMaterialManager.CreateMaterial( vec3( 0, 1, 0 ), 0, vec2( 0.05f ), DIFFUSE, 0.8f, 2.5f );
	RTMaterial &blueGlassMaterial = *new RTMaterial( lightBlue, REFRACT | REFLECT,vec3(0,0.05,0.95),1.5 );
		//gMaterialManager.CreateMaterial( lightBlue, 0, vec2( 1.0f ), TRANSMISSIVE_AND_REFLECTIVE, 0.95f, 1.5f );
	RTMaterial &mirrorMaterial = *new RTMaterial( vec3( 1 ), REFLECT, vec3( 0, 0.8, 0 ) );
		//gMaterialManager.CreateMaterial( vec3( 1 ), 0, vec2( 1.0f ), REFLECTIVE, 0.8f, 2.5f );

	scene.addObject( new RTPlane( vec3( -10.0f, 0.0f, 0.0f ), vec3( 1.0f, 0.0f, 0.0f ), vec3( 0.0f, 0.0f, -1.0f ), redMaterial ) );
	scene.addObject( new RTPlane( vec3( 10.0f, 0.0f, 0.0f ), vec3( -1.0f, 0.0f, 0.0f ), vec3( 0.0f, 0.0f, -1.0f ), greenMaterial ) );
	scene.addObject( new RTPlane( vec3( 0.0f, -5.0f, 0.0f ), vec3( 0.0f, 1.0f, 0.0f ), vec3( 1.0f, 0.0f, 0.0f ), brownCheckerBoardMaterial ) );
 	scene.addObject( new RTPlane( vec3( 0.0f, 0.0f, -35.0f ), vec3( 0.0f, 0.0f, 1.0f ), vec3( 1.0f, 0.0f, 0.0f ), whiteMaterial ) );
	scene.addObject( new RTPlane( vec3( 0.0f, 13.0f, 0.0f ), vec3( 0.0f, -1.0f, 0.0f ), vec3( 1.0f, 0.0f, 0.0f ), whiteMaterial ) );
	scene.addObject( new RTPlane( vec3( 0.0f, 0.0f, 1.0f ), vec3( 0.0f, 0.0f, -1.0f ), vec3( 1.0f, 0.0f, 0.0f ), whiteMaterial ) );

	RTObjMesh *mesh = new RTObjMesh( "assets/bunny.obj", mirrorMaterial );
	mesh->setPosition( 0, -2, -7 );
	mesh->setRotation( 0.0f, 0.0f, 0.0f );
	mesh->setScale( 0.1f, 0.1f, 0.1f );
	mesh->applyTransforms();
	scene.addObject( mesh );

	//scene.addObject( new RTSphere( vec3( 3.0f, -1.0f, -25.0f ), 4.0f, mirrorMaterial ) );
	scene.addObject( new RTSphere( vec3( -4.0f, -3.0f, -15.0f ), 2.0f, mirrorMaterial ) );
	scene.addObject( new RTSphere( vec3( 5.0f, -4.0f, -13.0f ), 1.0f, mirrorMaterial ) );
 	
 	scene.addObject( new RTBox( vec3( -0.0f, 9.0f, -30.0f ), vec3( 21.0f, 2.0f, 3.0f ), blueGlassMaterial ) );
	/////////////////////////////////////////////////////////////////////////
	scene.ambientLight = 0.3f;

	//RTLight *pLight = RTLight::createSpotLight( vec3( 1.0f, 1.0f, 1.0f ), 200.0f, vec3( 0.0f, 7.0f, -18.0f ), vec3( 0.0f, -1.0f, 0.0f ) );
	RTLight *pLight = RTLight::createPointLight( vec3( 1.0f, 1.0f, 1.0f ), 40.0f, vec3( 0.0f, 7.0f, -18.0f ));
	//RTLight *pLight = RTLight::createParralleLight( vec3( 1.0f, 1.0f, 1.0f ), 3000.0f, vec3( 0.0f, -1.0f, 0.0f ) );
	scene.addLight( pLight );
	///////////////////////////////////////////////////////////////////////////////
}

void Tmpl8::Game::scene_tw()
{
	//////////////////////////////////////////////////////////////////////////
// 	vec3 lightBlue( 190. / 255., 237. / 255., 1. );
// 	vec3 lightRed( 248. / 255., 192. / 255., 196. / 255 );
// 	RTChessBoardTexture *chessboardTexture = gTexManager.CreateChessBoardTexture( vec3( 1 ), vec3( 0 ) );
// 
// 	RTMaterial &brownCheckerBoardMaterial = gMaterialManager.CreateMaterial( vec3( 1 ), chessboardTexture, vec2( 0.2f,0.3f ), DIFFUSE, 0.8f, 2.5f );
// 	RTMaterial &whiteMaterial = gMaterialManager.CreateMaterial( vec3( 8.0f / 255.0f, 165.0f / 255.0f, 211.0f / 255.0f ), 0, vec2( 0.1f ), DIFFUSE, 0.8f, 2.5f );
// 	RTMaterial &redMaterial = gMaterialManager.CreateMaterial( vec3( 1, 0, 0 ), 0, vec2( 0.05f ), DIFFUSE, 0.8f, 2.5f );
// 	RTMaterial &greenMaterial = gMaterialManager.CreateMaterial( vec3( 0, 1, 0 ), 0, vec2( 0.05f ), DIFFUSE, 0.8f, 2.5f );
// 	RTMaterial &blueGlassMaterial = gMaterialManager.CreateMaterial( lightBlue, 0, vec2( 1.0f ), TRANSMISSIVE_AND_REFLECTIVE, 0.95f, 1.5f );
// 	RTMaterial &mirrorMaterial = gMaterialManager.CreateMaterial( vec3( 1 ), 0, vec2( 1.0f ), REFLECTIVE, 0.8f, 2.5f );
// 
// 	
// 	scene.addObject( new RTPlane( vec3( 0.0f, -5.0f, 0.0f ), vec3( 0.0f, 1.0f, 0.0f ), vec3( 1.0f, 0.0f, 0.0f ), brownCheckerBoardMaterial ) );
// 	scene.addObject( new RTPlane( vec3( 0.0f, 13.0f, 0.0f ), vec3( 0.0f, -1.0f, 0.0f ), vec3( 1.0f, 0.0f, 0.0f ), whiteMaterial ) );
// 
// 	//scene.addObject( new RTSphere( vec3( 3.0f, -1.0f, -25.0f ), 4.0f, mirrorMaterial ) );
// 	scene.addObject( new RTSphere( vec3( -4.0f, -3.0f, -15.0f ), 2.0f, mirrorMaterial ) );
// 	scene.addObject( new RTSphere( vec3( 5.0f, -4.0f, -13.0f ), 1.0f, mirrorMaterial ) );
// 
// 	/////////////////////////////////////////////////////////////////////////
// 	scene.ambientLight = 0.3f;
// 
// 	//RTLight *pLight = RTLight::createSpotLight( vec3( 1.0f, 1.0f, 1.0f ), 200.0f, vec3( 0.0f, 7.0f, -18.0f ), vec3( 0.0f, -1.0f, 0.0f ) );
// 	RTLight *pLight = RTLight::createPointLight( vec3( 1.0f, 1.0f, 1.0f ), 40.0f, vec3( 0.0f, 7.0f, -18.0f ) );
// 	//RTLight *pLight = RTLight::createParralleLight( vec3( 1.0f, 1.0f, 1.0f ), 3000.0f, vec3( 0.0f, -1.0f, 0.0f ) );
// 	scene.addLight( pLight );
// 
// 	return;
	///////////////////////////////////////////////////////////////////////////////

	scene.ambientLight = .4f;
	scene.backgroundColor = vec3( 8.0f / 255.0f, 165.0f / 255.0f, 211.0f / 255.0f );
	//////////////////////////////////////////////////////////////////////////
	RTChessBoardTexture *chessboardTexture = gTexManager.CreateChessBoardTexture( vec3( 219.0f / 255.0f, 31.0f / 255.0f, 7.0f / 255.0f ), vec3( 226.0f / 255.0f, 226.0f / 255.0f, 4.0f / 255.0f ) );

	RTMaterial *ryCheckerBoardMaterial = new RTMaterial( chessboardTexture, DIFFUSE, vec3( 1, 0, 0 ) ,vec2(0.2,0.2));
	RTMaterial *whiteGlassMaterial = new RTMaterial( vec3( 1 ),  DIFFUSE|REFRACT, vec3( 0.1, 0.0, 1.0f ), 1.5 );
	whiteGlassMaterial->highlight = 50.0f;
	RTMaterial*mirrorMaterial = new RTMaterial( vec3( 1,1,1 ), DIFFUSE | REFLECT, vec3( 0.3, 0.7, 0.0 ) );
	mirrorMaterial->highlight = 1.0f;
	RTPlane *plane = new RTPlane( vec3( 5.0f, -10.0f, -40.0f ), vec3( 0.0f, 1.0f, 0.0f ), vec3( 1.0f, 0.0f, 0.0f ), *ryCheckerBoardMaterial );
	plane->boundaryxy = vec2( 20.0f, 100.0f );
	scene.addObject( plane );

	RTSphere *sphere1 = new RTSphere( vec3( 5.0f, -3.0f, -16.0f ), 4.0f, *mirrorMaterial );
	scene.addObject( sphere1 );

	RTSphere *sphere2 = new RTSphere( vec3( -1.0f, 0.0f, -13.0f ), 4.0f, *whiteGlassMaterial );
	scene.addObject( sphere2 );

	RTInnerSphere *sphere3 = new RTInnerSphere( vec3( -1.0f, 0.0f, -13.0f ), 3.7f, *whiteGlassMaterial );
	scene.addObject( sphere3 );

	RTPlane *plane2 = new RTPlane( vec3( 0.0f, 0.0f, -2.0f ), vec3( 0.0f, 0.0f, 1.0f ), vec3( 1.0f, 0.0f, 0.0f ), *whiteGlassMaterial );
	plane2->boundaryxy = vec2( 0.6 );
	plane2->bCircle = true;
	//scene.addObject( plane2 );
	/////////////////////////////////////////////////////////////////////////
	

	RTLight *pLight = RTLight::createPointLight( vec3( 1.0f, 1.0f, 1.0f ), 4000.0f, vec3( -10.0f, 20.0f, 30.0f ) );
	//RTLight *pLight2 = RTLight::createSpotLight( vec3( 1.0f, 1.0f, 1.0f ), 80.0, vec3( -0.05, 0.25, 1.1f ), vec3 (0,0,-1));
	RTLight *pLight2 = RTLight::createPointLight( vec3( 1.0f, 1.0f, 1.0f ), 400.0, vec3( 0.02f, 0.02f, -1.4f ) );
	pLight2->setSpotlightRange( 1,2, 0.1 );
	pLight2->setAttenuation( 0, 0, 2 );
	scene.addLight( pLight );
	//scene.addLight( pLight2 );
	///////////////////////////////////////////////////////////////////////////////
}
