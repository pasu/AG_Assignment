#include "RayTracer.h"
#include "precomp.h"

static int threadFunc( void *threadInfo )
{
	RayTracer::RenderThread *ti = (RayTracer::RenderThread *)threadInfo;
	ti->rayTracer->traceChunk( ti->x_min, ti->x_max, ti->y_min, ti->y_max );
	return 0;
}

void RayTracer::startRenderThread( int x_min, int x_max, int y_min, int y_max )
{
	threads.push_back( {x_min, x_max, y_min, y_max, this, nullptr} );

	SDL_Thread *thread = SDL_CreateThread( threadFunc, "TestThread", &threads.back() );
	threads.back().thread = thread;
	
}

void RayTracer::waitRenderThreads()
{
	while ( !threads.empty() )
	{
		int status;
		SDL_WaitThread( threads.back().thread, &status );
		threads.pop_back();
	}
}