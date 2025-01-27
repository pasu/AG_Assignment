// add your includes to this file instead of to individual .cpp files
// to enjoy the benefits of precompiled headers:
// - fast compilation
// - solve issues with the order of header files once (here)
// do not include headers in header files (ever).

// Prevent expansion clashes (when using std::min and std::max):
#pragma comment( lib, "OpenGL32.lib" )
#define NOMINMAX

#define NUMBER_THREAD 1

#define BVH_ON
#define SAH_ON
#define BIN_NUM 100

//#define BVH_RANGED_TRAVERSAL
#define BVH_PARTITION_TRAVERSAL
//#undef BVH_PARTITION_TRAVERSAL

#define RAYPACKET_DIM ( 1 << 3 )
#define RAYPACKET_RAYS_PER_PACKET ( RAYPACKET_DIM * RAYPACKET_DIM )

#define SCRWIDTH 640
#define SCRHEIGHT 640
// #define FULLSCREEN
#define VSYNC
#define ADVANCEDGL // faster if your system supports it

#define SAMPLE_NUM 16
#define SAMPLE_NUM2 SAMPLE_NUM *SAMPLE_NUM

#define PATH_TRACER
#define NEE

#define PHOTON_MAPPING
#define NUM_PHOTON 10000

#define X_AXIS 0
#define Y_AXIS 1
#define Z_AXIS 2
#define LEAF 3
#define NUM_PHOTON_RADIANCE 10

// Glew should be included first
//#include <GL/glew.h>
#include "glad/glad.h"

// Comment for autoformatters: prevent reordering these two.
#include <GL/gl.h>

#ifdef _WIN32
// Followed by the Windows header
#include <Windows.h>

// Then import wglext: This library tries to include the Windows
// header WIN32_LEAN_AND_MEAN, unless it was already imported.
#include <GL/wglext.h>
// Extra definitions for redirectIO
#include <fcntl.h>
#include <io.h>
#endif

// External dependencies:
#include <FreeImage.h>
#include <SDL2/SDL.h>

// C++ headers
#include <algorithm>
#include <chrono>
#include <fstream>
#include <iostream>
#include <memory>
#include <random>
#include <vector>

// Namespaced C headers:
#include <cassert>
#include <cinttypes>
#include <cmath>
#include <cstdio>
#include <cstdlib>

// Header for AVX, and every technology before it.
// If your CPU does not support this, include the appropriate header instead.
// See: https://stackoverflow.com/a/11228864/2844473
#include <immintrin.h>

// clang-format off

// "Leak" common namespaces to all compilation units. This is not standard
// C++ practice but a mere simplification for this small project.
using namespace std;

#include "surface.h"
#include "template.h"

using namespace Tmpl8;

#include "game.h"
// clang-format on
