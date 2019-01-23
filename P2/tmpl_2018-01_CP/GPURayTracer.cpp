#include"precomp.h"
#include"GPURayTracer.h"

#define make_str(x) #x

static const char* shader_source = make_str(#version 430



);

using namespace gpurt;


static GLuint screen_texture=0;


GLuint pbo;
unsigned char b[640][640][4];

void gpurt::init() {
    for (int i = 0; i < 640; i++) {
        for (int j = 0; j < 640; j++) {
            b[i][j][2] = 255;
        }
    }
    glGenBuffers(1, &pbo);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo);
    glBufferData(GL_PIXEL_UNPACK_BUFFER, sizeof(b), b, GL_STREAM_READ);

    initScene();

}

void gpurt::render(Surface *screen) {

    glGetBufferSubData(GL_PIXEL_UNPACK_BUFFER, 0, sizeof(b), screen->GetBuffer());

   
}