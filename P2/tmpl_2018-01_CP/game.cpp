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
#include "RTTriangle.h"
#include "RTCameraController.h"

#include "RTTextureManager.h"
#include "RTMaterialManager.h"
#include "RTChessBoardTexture.h"

#include"GPURayTracer.h"
// -----------------------------------------------------------
// Initialize the application
// -----------------------------------------------------------
RTTextureManager gTexManager;
RTMaterialManager gMaterialManager;

//Scene scene( vec3( 0.1f ), vec3( 43.0f/255.0f, 203.0f / 255.0f, 246.0f/255.0f ) );
Scene scene( vec3( 0.1f ), vec3( 0.0f ) );
RenderOptions renderOptions;
RTCamera *pCamera = scene.getCamera();



void Game::Init()
{
	renderOptions.width = SCRWIDTH;
	renderOptions.height = SCRHEIGHT;
	renderOptions.maxRecursionDepth = 5;
	renderOptions.shadowBias = 0.01f;

	pTracer = new RayTracer( scene, renderOptions );

#ifdef ADVANCEDGL
    gpurt::init();
#endif

	//scene_bvh();
//#undef PHOTON_MAPPING
	scene_outdoor();
	//scene_indoor();
	//scene_depth();
	
#ifdef PHOTON_MAPPING
#endif
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
	bool bUpdate = updateCamera( *( scene.getCamera() ) );
	
	if ( bUpdate )
	{
		pTracer->Reset();
	}	
	
	scene.animate();
	scene.rebuildTopLevelBVH();
	pTracer->render( screen );

#ifdef ADVANCEDGL
    //gpurt::render(screen);
#endif

	// print something to the text window
// 	printf( "this goes to the console window.\n" );
// 	// draw a sprite
// 	rotatingGun.SetFrame( frame );
// 	rotatingGun.Draw( screen, 100, 100 );
	
	if (++frame == 30) {
// 		DWORD nt = GetTickCount();
// 
// 		sprintf( buffer, "FPS: %.2f", 30000.0f / ( nt - tt ) );
//         printf( "FPS: %.2f\n", 30000.0f / (nt - tt));
// 		tt = nt;
// 		frame = 0;
	}

	sprintf( buffer, "SPP: %d", pTracer->getSCount() );

	screen->Print( buffer, 2, 2, 0xffffff );
}

void Tmpl8::Game::KeyDown( int key )
{
	static int index = 0;
	static int scenecount = 3;
	if (key == 79)
	{
		gTexManager.ClearAll();
		gMaterialManager.ClearAll();
		scene.ClearAllLight();
		scene.ClearAllObj();
		scene.AttachSkyDome(NULL);

		index++;
		switch ( index % scenecount )
		{
		case 0:
			scene_outdoor();
			break;
		case 1:
			scene_indoor();
			break;
		case 2:
			scene_depth();
			break;
		}
		pTracer->Reset();
	}

	if (key == 30)
	{
		pTracer->SetFilterMethod( 1 );
	}

	if ( key == 31 )
	{
		pTracer->SetFilterMethod( 2 );
	}

	if ( key == 32 )
	{
		scene.getCamera()->aperture += 0.001f;
		pTracer->Reset();
	}

	if ( key == 33 )
	{
		scene.getCamera()->aperture > 0.001f ? scene.getCamera()->aperture -= 0.001f : 0.0f;
		pTracer->Reset();
	}

	//scene.getCamera()->setEye( vec3( 0 ) );
}

void Tmpl8::Game::MouseWheel( int y )
{
}


void Tmpl8::Game::scene_outdoor()
{
	scene.getCamera()->aperture = 0.0;
	//////////////////////////////////////////////////////////////////////////
	vec3 lightBlue( 190. / 255., 237. / 255., 0.9 );
	vec3 lightRed( 248. / 255., 192. / 255., 196. / 255 );
	RTTexture *pFloor = gTexManager.CreateTexture( "./assets/floor_diffuse.PNG", true, 8, 20.0f );
	RTTexture *pSkydome = gTexManager.CreateTexture( "./assets/skydome.jpg", true, 8, 20.0f );

	scene.AttachSkyDome( pSkydome );

	RTMaterial &floorMaterial = gMaterialManager.CreateMaterial( vec3( 0.7 ), pFloor, vec2( 0.2f ), Glossy, 0.8f, 2.5f );
	floorMaterial.pow_ = 0.05;

	vector<RTPrimitive *> arrObjs;
	arrObjs.push_back( new RTPlane( vec3( 0.0f, -10.0f, 0.0f ), vec3( 0.0f, 1.0f, 0.0f ), vec3( 1.0f, 0.0f, 0.0f ), floorMaterial ) );

	RTMaterial &whiteMaterial = gMaterialManager.CreateMaterial( vec3( 0.9 ), 0, vec2( 0.1f ), DIFFUSE, 0.8f, 2.5f );
	
	RTTexture *tower = gTexManager.CreateTexture( "./assets/Wood_Tower_Col.jpg", true, 8, 20.0f );
	RTMaterial &towerMaterial = gMaterialManager.CreateMaterial( vec3( 1, 1, 1 ), tower, vec2( 1 ), DIFFUSE_AND_REFLECTIVE, 0.0f, 2.5f );

	RTObjMesh *mesh = new RTObjMesh( "assets/wooden.dae", towerMaterial );
	mesh->setPosition( -1, -9, -25 );
	mesh->setRotation( -20.0f, 0.0f, 0.0f );
	mesh->setScale( 1, 1, 1 );
	mesh->applyTransforms();
	//	scene.addObject( mesh );
	vector<RTTriangle *> tarray;
	mesh->getTriangles( tarray );

	RTMaterial &blueGlassMaterial = gMaterialManager.CreateMaterial( vec3( 0.9 ), 0, vec2( 1.0f ), REFLECTIVE, 0.95f, 2.24f );
	blueGlassMaterial.pow_ = 10000;
	arrObjs.push_back( new RTSphere( vec3( 7.0f, -1.0f, -25.0f ), 3.0f, blueGlassMaterial ) );

	RTGeometry *robotGeometry = new RTGeometry();

	vec3 posL = vec3( 0, 5, -25 );
	RTMaterial &lightM = gMaterialManager.CreateMaterial( vec3( 1 ), vec3( 1000 ), DIFFUSE );
	RTPlane *plane = new RTPlane( posL, vec3( 0.0f, -1.0f, 0.0f ), vec3( 1.0f, 0.0f, 0.0f ), lightM, vec2( 2 ) );
	arrObjs.push_back( plane );
	
	for ( size_t i = 0; i < arrObjs.size(); i++ )
	{
		robotGeometry->addObject( arrObjs[i] );
	}

	for ( size_t i = 0; i < tarray.size(); i++ )
	{
		robotGeometry->addObject( tarray[i] );
	}

	robotGeometry->BuildBVHTree();

	RTObject *pRobot = new RTObject( robotGeometry );
	scene.addObject( pRobot );
	scene.BuildBVHTree();
	/////////////////////////////////////////////////////////////////////////
	scene.ambientLight = 0.3f;

	RTLight *pLight = RTLight::createAreaLight( vec3( 1.0f, 1.0f, 1.0f ), 0.0f, posL, plane );
	scene.addLight( pLight );

	scene.updateLightsWeight();
}


void Tmpl8::Game::scene_indoor()
{
	scene.getCamera()->aperture = 0.0;
	//////////////////////////////////////////////////////////////////////////
	RTChessBoardTexture *chessboardTexture = gTexManager.CreateChessBoardTexture( vec3( 0.9 ), vec3( 0 ) );
	RTTexture *sphereT = gTexManager.CreateTexture( "./assets/box.PNG", true, 1, 100 );

	RTMaterial &brownCheckerBoardMaterial = gMaterialManager.CreateMaterial( vec3( 0.7 ), chessboardTexture, vec2( 0.2f ), DIFFUSE, 0.8f, 0.5f );
	brownCheckerBoardMaterial.pow_ = 0.3;
	RTMaterial &whiteMaterial = gMaterialManager.CreateMaterial( vec3( 0.7 ), 0, vec2( 0.1f ), DIFFUSE, 0.8f, 2.5f );
	RTMaterial &mirror = gMaterialManager.CreateMaterial( vec3( 0.7 ), 0, vec2( 0.1f ), REFLECTIVE, 0.8f, 2.5f );
	RTMaterial &redMaterial = gMaterialManager.CreateMaterial( vec3( 0.7, 0, 0 ), 0, vec2( 0.05f ), DIFFUSE, 0.8f, 2.5f );
	RTMaterial &greenMaterial = gMaterialManager.CreateMaterial( vec3( 0, 0.7, 0 ), 0, vec2( 0.05f ), DIFFUSE, 0.8f, 2.5f );
	
	RTMaterial &leftGlassMaterial = gMaterialManager.CreateMaterial( vec3( 0.9 ), sphereT, vec2( 1.0f ), Phong, 0.55f, 2.24f );
	leftGlassMaterial.pow_ = 400;

	RTMaterial &blueGlassMaterial = gMaterialManager.CreateMaterial( vec3( 0.9 ), 0, vec2( 1.0f ), TRANSMISSIVE_AND_REFLECTIVE, 0.95f, 1.4f );
	blueGlassMaterial.pow_ = 0;

	RTMaterial &mirrorMaterial = gMaterialManager.CreateMaterial( vec3( 0.9 ), 0, vec2( 1.0f ), DIFFUSE, 0.8f, 2.1f );
	mirrorMaterial.pow_ = 1;
	mirrorMaterial.k_ = 0.9;

	vector<RTPrimitive *> arrObjs;
	arrObjs.push_back( new RTPlane( vec3( -10.0f, 0.0f, 0.0f ), vec3( 1.0f, 0.0f, 0.0f ), vec3( 0.0f, 0.0f, -1.0f ), redMaterial ) );
	arrObjs.push_back( new RTPlane( vec3( 10.0f, 0.0f, 0.0f ), vec3( -1.0f, 0.0f, 0.0f ), vec3( 0.0f, 0.0f, -1.0f ), greenMaterial ) );
	arrObjs.push_back( new RTPlane( vec3( 0.0f, -8.0f, 0.0f ), vec3( 0.0f, 1.0f, 0.0f ), vec3( 1.0f, 0.0f, 0.0f ), brownCheckerBoardMaterial ) );
	//arrObjs.push_back( new RTPlane( vec3( 0.0f, 8.0f, 0.0f ), vec3( 0.0f, -1.0f, 0.0f ), vec3( 1.0f, 0.0f, 0.0f ), whiteMaterial ) );
	arrObjs.push_back( new RTPlane( vec3( 0.0f, 0.0f, -35.0f ), vec3( 0.0f, 0.0f, 1.0f ), vec3( 1.0f, 0.0f, 0.0f ), mirror ) );
	arrObjs.push_back( new RTPlane( vec3( 0.0f, 0.0f, 1.0f ), vec3( 0.0f, 0.0f, -1.0f ), vec3( 1.0f, 0.0f, 0.0f ), whiteMaterial ) );

	vec3 posL = vec3( 5, 5, -20 );
	vec3 posL2 = vec3( -7.0f, 5.0f, -20.0f );
	RTMaterial &lightM = gMaterialManager.CreateMaterial( vec3( 1 ), vec3( 3000 ), DIFFUSE );
	RTPlane *plane = new RTPlane( posL, vec3( 0.0f, -1.0f, 0.0f ), vec3( 1.0f, 0.0f, 0.0f ), lightM, vec2( 0.5 ) );
	RTPlane *plane2 = new RTPlane( posL2, vec3( 0.0f, -1.0f, 0.0f ), vec3( 1.0f, 0.0f, 0.0f ), lightM, vec2( 1.5 ) );
	arrObjs.push_back( plane );
	//arrObjs.push_back( plane2 );
	//arrObjs.push_back( new RTBox( vec3( 0.0f, 2.0f, -20.0f ), vec3( 2.0f), blueGlassMaterial ) );
	arrObjs.push_back( new RTSphere( vec3( 5.0f, -3.0f, -20.0f ), 2.0f, blueGlassMaterial ) );
	arrObjs.push_back( new RTSphere( vec3( 7.0f, 2.0f, -20.0f ), 1.0f, leftGlassMaterial ) );

	RTGeometry *robotGeometry = new RTGeometry();

	for ( size_t i = 0; i < arrObjs.size(); i++ )
	{
		robotGeometry->addObject( arrObjs[i] );
	}

	robotGeometry->BuildBVHTree();

	RTObject *pRobot = new RTObject( robotGeometry );
	scene.addObject( pRobot );
	scene.BuildBVHTree();
	/////////////////////////////////////////////////////////////////////////
	scene.ambientLight = 0.3f;

	RTLight *pLight = RTLight::createAreaLight( vec3( 1.0f, 1.0f, 1.0f ), 0.0f, posL, plane );
	scene.addLight( pLight );

	RTLight *pLight2 = RTLight::createAreaLight( vec3( 1.0f, 1.0f, 1.0f ), 0.0f, posL2, plane2 );
	//scene.addLight( pLight2 );
	scene.updateLightsWeight();

	pTracer->emit_photons();
	///////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////
}


void Tmpl8::Game::scene_depth()
{
	{
		//////////////////////////////////////////////////////////////////////////
		vec3 lightBlue( 190. / 255., 237. / 255., 0.9 );
		vec3 lightRed( 248. / 255., 192. / 255., 196. / 255 );
		RTTexture *pFloor = gTexManager.CreateTexture( "./assets/floor_diffuse.PNG", true, 8, 20.0f );
		RTTexture *pSkydome = gTexManager.CreateTexture( "./assets/skydome.jpg", true, 8, 20.0f );

		scene.AttachSkyDome( pSkydome );

		RTMaterial &floorMaterial = gMaterialManager.CreateMaterial( vec3( 0.9 ), pFloor, vec2( 0.2f ), REFLECTIVE, 0.8f, 2.5f );
		floorMaterial.pow_ = 0.05;

		vector<RTPrimitive *> arrObjs;
		arrObjs.push_back( new RTPlane( vec3( 0.0f, -10.0f, 0.0f ), vec3( 0.0f, 1.0f, 0.0f ), vec3( 1.0f, 0.0f, 0.0f ), floorMaterial ) );

		RTMaterial &blueGlassMaterial = gMaterialManager.CreateMaterial( vec3( 0.9 ), 0, vec2( 1.0f ), REFLECTIVE, 0.95f, 2.24f );
		blueGlassMaterial.pow_ = 10000;

		RTTexture *tower = gTexManager.CreateTexture( "./assets/Wood_Tower_Col.jpg", true, 8, 20.0f );
		RTMaterial &towerMaterial = gMaterialManager.CreateMaterial( vec3( 1, 1, 1 ), tower, vec2( 1 ), DIFFUSE_AND_REFLECTIVE, 0.0f, 2.5f );

		RTObjMesh *mesh = new RTObjMesh( "assets/wooden.dae", towerMaterial );

		RTGeometry *robotGeometry = new RTGeometry();
		for (int i=0;i<1;i++)
		{
			
			mesh->setPosition( -1 + i*5, -9, -15 + i*5 );
			mesh->setRotation( -20.0f, 0.0f, 0.0f );
			mesh->setScale( 1, 1, 1 );
			mesh->applyTransforms();
			//	scene.addObject( mesh );
			vector<RTTriangle *> tarray;
			mesh->getTriangles( tarray );

			for ( size_t i = 0; i < tarray.size(); i++ )
			{
				robotGeometry->addObject( tarray[i] );
			}
		}			

		vec3 posL = vec3( 0, 5, -25 );
		RTMaterial &lightM = gMaterialManager.CreateMaterial( vec3( 1 ), vec3( 1000 ), DIFFUSE );
		RTPlane *plane = new RTPlane( posL, vec3( 0.0f, -1.0f, 0.0f ), vec3( 1.0f, 0.0f, 0.0f ), lightM, vec2( 2 ) );
		arrObjs.push_back( plane );

		for ( size_t i = 0; i < arrObjs.size(); i++ )
		{
			robotGeometry->addObject( arrObjs[i] );
		}

		robotGeometry->BuildBVHTree();

		RTObject *pRobot = new RTObject( robotGeometry );
		scene.addObject( pRobot );
		scene.BuildBVHTree();
		/////////////////////////////////////////////////////////////////////////
		scene.ambientLight = 0.3f;

		RTLight *pLight = RTLight::createAreaLight( vec3( 1.0f, 1.0f, 1.0f ), 0.0f, posL, plane );
		scene.addLight( pLight );

		scene.updateLightsWeight();

		scene.getCamera()->aperture = 0.2;
	}
}

static void animateFunc( RTObject *object )
{

    object->translateGlobal( object->speed );

    if (object->pos.x > 300) {
		object->pos.x = -300;
    }
    if (object->pos.x < -300) {
		object->pos.x = 300;
    }
	if ( object->pos.y > 300 )
	{
		object->pos.y = -300;
	}
	if ( object->pos.y < -300 )
	{
		object->pos.y = 300;
	}
	if ( object->pos.z < -300 )
	{
		object->pos.z = 0;
	}
	if ( object->pos.z > 0 )
	{
		object->pos.z = -300;
	}
	object->rotateLocal( object->rotateAxis, 0.02 );
}

void Tmpl8::Game::scene_light()
{
	//////////////////////////////////////////////////////////////////////////
	vec3 lightBlue( 190. / 255., 237. / 255., 0.9 );
	vec3 lightRed( 248. / 255., 192. / 255., 196. / 255 );
	RTChessBoardTexture *chessboardTexture = gTexManager.CreateChessBoardTexture( vec3( 0.9 ), vec3( 0 ) );
	RTTexture *pFloor = gTexManager.CreateTexture( "./assets/floor_diffuse.PNG", true, 8, 20.0f );
	RTTexture *sphereT = gTexManager.CreateTexture( "./assets/box.PNG", true, 1, 100 );

	RTMaterial &brownCheckerBoardMaterial = gMaterialManager.CreateMaterial( vec3( 0.7 ), chessboardTexture, vec2( 0.2f ), DIFFUSE, 0.8f, 2.5f );
	brownCheckerBoardMaterial.pow_ = 0.3;
	RTMaterial &whiteMaterial = gMaterialManager.CreateMaterial( vec3( 0.7 ), 0, vec2( 0.1f ), REFLECTIVE, 0.8f, 2.5f );
	RTMaterial &whiteMaterial2 = gMaterialManager.CreateMaterial( vec3( 0.9 ), 0, vec2( 0.1f ), TRANSMISSIVE_AND_REFLECTIVE, 0.8f, 2.5f );
	RTMaterial &redMaterial = gMaterialManager.CreateMaterial( vec3( 0.7,0,0 ), 0, vec2( 0.05f ), DIFFUSE, 0.8f, 2.5f );
	RTMaterial &greenMaterial = gMaterialManager.CreateMaterial( vec3( 0,0.7,0 ), 0, vec2( 0.05f ), DIFFUSE, 0.8f, 2.5f );
	RTMaterial &blueGlassMaterial = gMaterialManager.CreateMaterial( vec3( 0.9 ), sphereT, vec2( 1.0f ), Phong, 0.55f, 2.24f );
	blueGlassMaterial.pow_ = 400;
	RTMaterial &mirrorMaterial = gMaterialManager.CreateMaterial( vec3( 0.9 ), 0, vec2( 1.0f ), DIFFUSE, 0.8f, 2.1f );
	mirrorMaterial.pow_ = 1;
	mirrorMaterial.k_ = 0.9;
	vector<RTPrimitive *> arrObjs;
	arrObjs.push_back( new RTPlane( vec3( -10.0f, 0.0f, 0.0f ), vec3( 1.0f, 0.0f, 0.0f ), vec3( 0.0f, 0.0f, -1.0f ), redMaterial ) );
	arrObjs.push_back( new RTPlane( vec3( 10.0f, 0.0f, 0.0f ), vec3( -1.0f, 0.0f, 0.0f ), vec3( 0.0f, 0.0f, -1.0f ), greenMaterial ) );
	arrObjs.push_back( new RTPlane( vec3( 0.0f, -8.0f, 0.0f ), vec3( 0.0f, 1.0f, 0.0f ), vec3( 1.0f, 0.0f, 0.0f ), brownCheckerBoardMaterial ) );
	arrObjs.push_back( new RTPlane( vec3( 0.0f, 0.0f, -35.0f ), vec3( 0.0f, 0.0f, 1.0f ), vec3( 1.0f, 0.0f, 0.0f ), whiteMaterial ) );
	//arrObjs.push_back( new RTPlane( vec3( 0.0f, 13.0f, 0.0f ), vec3( 0.0f, -1.0f, 0.0f ), vec3( 1.0f, 0.0f, 0.0f ), whiteMaterial ) );
	arrObjs.push_back( new RTPlane( vec3( 0.0f, 0.0f, 1.0f ), vec3( 0.0f, 0.0f, -1.0f ), vec3( 1.0f, 0.0f, 0.0f ), whiteMaterial ) );

	RTObjMesh *mesh = new RTObjMesh( "assets/bunny.obj", whiteMaterial2 );
	mesh->setPosition( 0, -5, -12 );
	mesh->setRotation( 0.0f, 0.0f, 0.0f );
	mesh->setScale( 1,1,1 );
	mesh->applyTransforms();
	//scene.addObject( mesh );

	vector<RTTriangle *> meshmilktarray;
	mesh->getTriangles( meshmilktarray );

	//scene.addObject( new RTSphere( vec3( 3.0f, -1.0f, -25.0f ), 4.0f, mirrorMaterial ) );
	//arrObjs.push_back( new RTSphere( vec3( -4.0f, -1.0f, -15.0f ), 3.0f, blueGlassMaterial ) );
	//arrObjs.push_back( new RTSphere( vec3( 5.0f, -1.0f, -15.0f ), 3.0f, blueGlassMaterial ) );

	//arrObjs.push_back( new RTBox( vec3( -0.0f, 0.0f, -7.0f ), vec3( 1.0f, 1.0f, 1.0f ), blueGlassMaterial ) );
	vec3 posL = vec3( 0, 5, -12 ); 
	vec3 posL2 = vec3( -4.0f, 5.0f, -15.0f ); 
	RTMaterial &lightM = gMaterialManager.CreateMaterial( vec3( 1 ), vec3( 100 ), DIFFUSE);
	RTPlane *plane = new RTPlane( posL, vec3( 0.0f, -1.0f, 0.0f ), vec3( 1.0f, 0.0f, 0.0f ), lightM, vec2( 2 ) );
	RTPlane *plane2 = new RTPlane( posL2, vec3( 0.0f, -1.0f, 0.0f ), vec3( 1.0f, 0.0f, 0.0f ), lightM, vec2( 2 ) );
	arrObjs.push_back( plane );
	//arrObjs.push_back( plane2 );
	RTGeometry *robotGeometry = new RTGeometry();

	for ( size_t i = 0; i < arrObjs.size(); i++ )
	{
		robotGeometry->addObject( arrObjs[i] );
	}

	for ( size_t i = 0; i < meshmilktarray.size(); i++ )
	{
		robotGeometry->addObject( meshmilktarray[i] );
	}

	robotGeometry->BuildBVHTree();

	RTObject *pRobot = new RTObject( robotGeometry );
	scene.addObject( pRobot );
	scene.BuildBVHTree();
	/////////////////////////////////////////////////////////////////////////
	scene.ambientLight = 0.3f;

	//RTLight *pLight = RTLight::createSpotLight( vec3( 1.0f, 1.0f, 1.0f ), 200.0f, vec3( 0.0f, 7.0f, -18.0f ), vec3( 0.0f, -1.0f, 0.0f ) );
	RTLight *pLight = RTLight::createAreaLight( vec3( 1.0f, 1.0f, 1.0f ), 0.0f, posL, plane );
	//RTLight *pLight = RTLight::createParralleLight( vec3( 1.0f, 1.0f, 1.0f ), 3000.0f, vec3( 0.0f, -1.0f, 0.0f ) );
	scene.addLight( pLight );

	RTLight *pLight2 = RTLight::createAreaLight( vec3( 1.0f, 1.0f, 1.0f ), 40.0f, posL2, plane2 );
	//RTLight *pLight = RTLight::createParralleLight( vec3( 1.0f, 1.0f, 1.0f ), 3000.0f, vec3( 0.0f, -1.0f, 0.0f ) );
	//scene.addLight( pLight2 );
	scene.updateLightsWeight();
	///////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////
}

void Tmpl8::Game::scene_bvh()
{
	//////////////////////////////////////////////////////////////////////////
	vec3 lightBlue( 190. / 255., 237. / 255., 1. );
	vec3 lightRed( 248. / 255., 192. / 255., 196. / 255 );
	
	RTTexture *manTexture = gTexManager.CreateTexture( "./assets/Cesium_Man.jpg", true, 8, 40.0f );
	RTTexture *milkTexture = gTexManager.CreateTexture( "./assets/Wood_Tower_Col.jpg", true, 8, 20.0f );
	RTTexture *carTexture = gTexManager.CreateTexture( "./assets/ambulance_red_d.tga", true, 8, 20.0f );

	RTMaterial &redMaterial = gMaterialManager.CreateMaterial( vec3( 1, 1, 1 ), NULL, vec2( 1 ), DIFFUSE, 0.0f, 2.5f );
	RTMaterial &milkMaterial = gMaterialManager.CreateMaterial( vec3( 1, 1, 1 ), milkTexture, vec2( 1 ), DIFFUSE_AND_REFLECTIVE, 0.0f, 2.5f );
	RTMaterial &carMaterial = gMaterialManager.CreateMaterial( vec3( 1, 1, 1 ), carTexture, vec2( 1 ), DIFFUSE_AND_REFLECTIVE, 0.0f, 2.5f );

	RTObjMesh *mesh = new RTObjMesh( "assets/Dragon.obj", redMaterial );
	mesh->setPosition( 30, -30, -283.5 );
	mesh->setRotation( 0, -30.0f, 0.0f );
	mesh->setScale( 0.1f, 0.1f, 0.1f );
	mesh->applyTransforms();
//	scene.addObject( mesh );
	vector<RTTriangle *> tarray;
	mesh->getTriangles( tarray );

    RTGeometry* robotGeometry = new RTGeometry();

	for ( size_t i = 0; i < tarray.size(); i++ )
	{
		robotGeometry->addObject( tarray[i] );
	}

	robotGeometry->BuildBVHTree();

	RTObjMesh *meshmilk = new RTObjMesh( "assets/wooden.dae", milkMaterial );
	meshmilk->setPosition( -2, -2, -35.5 );
	meshmilk->setRotation( 90.0f, 0.0f, 0.0f );
	//meshmilk->setScale( 0.1f, 0.1f, 0.1f );
	meshmilk->applyTransforms();
	//	scene.addObject( mesh );
	vector<RTTriangle *> meshmilktarray;
	meshmilk->getTriangles( meshmilktarray );

	RTGeometry *robotGeometry2 = new RTGeometry();

	for ( size_t i = 0; i < meshmilktarray.size(); i++ )
	{
		robotGeometry2->addObject( meshmilktarray[i] );
	}

	robotGeometry2->BuildBVHTree();

	RTObjMesh *carmilk = new RTObjMesh( "assets/ambulance.dae", carMaterial );
	carmilk->setPosition( -6, -2, -35.5 );
	carmilk->setRotation( 80.0f, 0.0f, 0.0f );
	//meshmilk->setScale( 0.1f, 0.1f, 0.1f );
	carmilk->applyTransforms();
	//	scene.addObject( mesh );
	vector<RTTriangle *> cartarray;
	carmilk->getTriangles( cartarray );

	RTGeometry *robotGeometry3 = new RTGeometry();

	for ( size_t i = 0; i < cartarray.size(); i++ )
	{
		robotGeometry3->addObject( cartarray[i] );
	}

	robotGeometry3->BuildBVHTree();

    //for (int i = 0; i < 100; i++) 
	{
		RTObject *pRobot = new RTObject( robotGeometry );
        scene.addObject(pRobot);

		RTObject *pRobot2 = new RTObject( robotGeometry2 );
		scene.addObject( pRobot2 );

		RTObject *pRobot3 = new RTObject( robotGeometry3 );
		scene.addObject( pRobot3 );
    }

	scene.ambientLight = 0.3f;

	RTLight *pLight = RTLight::createPointLight( vec3( 1.0f, 1.0f, 1.0f ), 40.0f, vec3( -2.0f, 2.0f, -35.0f ) );
	scene.addLight( pLight );

    scene.BuildBVHTree();

}

void Tmpl8::Game::scene_toplevel()
{
	//////////////////////////////////////////////////////////////////////////
	vec3 lightBlue( 190. / 255., 237. / 255., 1. );
	vec3 lightRed( 248. / 255., 192. / 255., 196. / 255 );

	RTTexture *manTexture = gTexManager.CreateTexture( "./assets/Cesium_Man.jpg", true, 8, 40.0f );

	RTMaterial &redMaterial = gMaterialManager.CreateMaterial( vec3( 1, 1, 1 ), manTexture, vec2( 1 ), DIFFUSE_AND_REFLECTIVE, 0.0f, 2.5f );

	RTObjMesh *mesh = new RTObjMesh( "assets/Cesium_Man.dae", redMaterial );
	mesh->setPosition( 0, 0, 0 );
	mesh->setScale( 30.1f, 30.1f, 30.1f );
	mesh->applyTransforms();
	//	scene.addObject( mesh );
	vector<RTTriangle *> tarray;
	mesh->getTriangles( tarray );

	RTGeometry *robotGeometry = new RTGeometry();

	for ( size_t i = 0; i < tarray.size(); i++ )
	{
		robotGeometry->addObject( tarray[i] );
	}

	robotGeometry->BuildBVHTree();

	for ( int i = 0; i < 100; i++ )
	{
		RTObject *pRobot = new RTObject( robotGeometry );
		pRobot->translateGlobal( vec3( rand() / ( RAND_MAX / 600.0f ) - 300, rand() / ( RAND_MAX / 600.0f ) - 300, -rand() / ( RAND_MAX / 600.0f ) - 100 ) );
		pRobot->speed = vec3( rand() / float( RAND_MAX ) - 0.5, rand() / float( RAND_MAX ) - 0.5, -rand() / float( RAND_MAX ) + 0.5 );

		pRobot->rotateAxis = vec3( rand() / float( RAND_MAX ) - 0.5, rand() / float( RAND_MAX ) - 0.5, -rand() / float( RAND_MAX ) + 0.5 );
		pRobot->rotateAxis.normalize();

		pRobot->setAnimateFunc( animateFunc );
		scene.addObject( pRobot );
	}

	scene.ambientLight = 0.3f;

	RTLight *pLight = RTLight::createParralleLight( vec3( 1.0f, 1.0f, 1.0f ), 1.0f, vec3( -20.0f, -20.0f, 0.0f ) );
	scene.addLight( pLight );

	scene.BuildBVHTree();
}

void Tmpl8::Game::scene_sbvh()
{
	//////////////////////////////////////////////////////////////////////////
	vec3 lightBlue( 190. / 255., 237. / 255., 1. );
	vec3 lightRed( 248. / 255., 192. / 255., 196. / 255 );

	RTTexture *manTexture = gTexManager.CreateTexture( "./assets/Cesium_Man.jpg", true, 8, 40.0f );

	RTMaterial &redMaterial = gMaterialManager.CreateMaterial( vec3( 1, 1, 1 ), manTexture, vec2( 1 ), DIFFUSE_AND_REFLECTIVE, 0.0f, 2.5f );

	RTObjMesh *mesh = new RTObjMesh( "assets/Cesium_Man.dae", redMaterial );
	mesh->setPosition( 0, -3, -3.5 );
	mesh->setRotation( 180.0f, -90.0f, 0.0f );
	mesh->applyTransforms();
	//	scene.addObject( mesh );
	vector<RTTriangle *> tarray;
	mesh->getTriangles( tarray );

	RTGeometry *robotGeometry = new RTGeometry();

	for ( size_t i = 0; i < tarray.size(); i++ )
	{
		robotGeometry->addObject( tarray[i] );
	}

	robotGeometry->BuildSBVHTree();

	//for (int i = 0; i < 100; i++)
	{
		RTObject *pRobot = new RTObject( robotGeometry );
		scene.addObject( pRobot );
	}

	scene.ambientLight = 0.9f;

	RTLight *pLight = RTLight::createPointLight( vec3( 1.0f, 1.0f, 1.0f ), 400.0f, vec3( 0.0f, 20.0f, -100.0f ) );
	scene.addLight( pLight );

	scene.BuildBVHTree();
}
