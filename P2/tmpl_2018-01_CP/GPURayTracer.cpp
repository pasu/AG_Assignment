#include"precomp.h"
#include"GPURayTracer.h"

#define make_str(x) #x

static const char* shader_source = make_str(#version 430



);

using namespace gpurt;


static GLuint screen_texture=0;


void gpurt::init() {
    glGenTextures(1, &screen_texture);
}

void gpurt::render() {
    glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);

   
}