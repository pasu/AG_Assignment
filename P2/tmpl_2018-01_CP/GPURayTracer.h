#pragma once
#include<glad/glad.h>
#include "RenderOptions.h"
#include "GPURT_Scene.h"
namespace gpurt {
    void init();
    void render(Surface *screen);
}