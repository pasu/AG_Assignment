#pragma once
#include<glad/glad.h>
#include "RenderOptions.h"
#include "Scene.h"
namespace gpurt {
    void init();
    void render(Surface *screen);
}