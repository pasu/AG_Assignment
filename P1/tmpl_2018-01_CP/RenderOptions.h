#pragma once

#include <string>
using namespace std;

struct RenderOptions {
	int width;
	int height;
	float fov;
	int maxRecursionDepth;
	float shadowBias;
};