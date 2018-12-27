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
	renderOptions.maxRecursionDepth = 5;
	renderOptions.shadowBias = 0.01f;

	pTracer = new RayTracer( scene, renderOptions );

	scene_bvh();
	
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
	scene.animate();
	scene.rebuildTopLevelBVH();
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
	static int scenecount = 3;
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
			scene_bvh();
			break;
		case 1:
			scene_toplevel();
			break;
		case 2:
			scene_sbvh();
			break;
		}
	}

	//scene.getCamera()->setEye( vec3( 0 ) );
}

void Tmpl8::Game::MouseWheel( int y )
{
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
