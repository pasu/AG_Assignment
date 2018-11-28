#include "RTCameraController.h"
#include "precomp.h"



void updateRotation( RTCamera &camera )
{
	static bool left_pressed = false;
	static int lastX, lastY;

	int x, y;
	Uint32 buttons = SDL_GetMouseState( &x, &y );
	if ( ( SDL_BUTTON( 1 ) & buttons ) && !left_pressed )
	{
		left_pressed = true;
		lastX = x;
		lastY = y;
		printf( "left button pressed\n" );
	}
	if ( !( SDL_BUTTON( 1 ) & buttons ) && left_pressed )
	{
		left_pressed = false;
		printf( "left button released\n" );
	}

	if (left_pressed)
	{
		float rotX = ( lastX - x ) / 500.0f;
		camera.turnLeft( rotX );
		float rotY = ( lastY - y ) / 500.0f;
		camera.turnUp( rotY );
		lastX = x;
		lastY = y;
	}
}

void updateTranslation(RTCamera& camera)
{
	const Uint8 *state = SDL_GetKeyboardState( NULL );
	if (state[SDL_SCANCODE_W]) {
		camera.moveForward(0.2);
	}
	
	if (state[SDL_SCANCODE_S]) {
		camera.moveForward( -0.2 );
	}
	if ( state[SDL_SCANCODE_A] )
	{
		camera.moveLeft( 0.2 );
	}

	if ( state[SDL_SCANCODE_D] )
	{
		camera.moveLeft( -0.2 );
	}

	if ( state[SDL_SCANCODE_Q] )
	{
		camera.moveUp( 0.2 );
	}

	if ( state[SDL_SCANCODE_E] )
	{
		camera.moveUp( -0.2 );
	}
}

void updateCamera(RTCamera& camera)
{
	updateRotation(camera);
	updateTranslation(camera);
	camera.Update();
}
