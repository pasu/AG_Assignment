#include"precomp.h"
#include"GPURayTracer.h"
#include"GPURT_Shader.h"

using namespace gpurt;


static GLuint screen_texture=0;


GLuint pbo;
unsigned char b[640][640][4];


static GLuint sphere_buffer = 0;
static GLuint screen_pixel_buffer = 0;

vec4 screen_pixels[640][640];


static GLuint program;
static GLuint compute_shader;
static gpurt::Scene* scene1;


void gpurt::init() {
    for (int i = 0; i < 640; i++) {
        for (int j = 0; j < 640; j++) {
            b[i][j][2] = 255;
        }
    }

    scene1 = gpurt::Scene::initScene1();

    scene1->init();

    scene1->upload();

    scene1->bind();

    glGenBuffers(1, &screen_pixel_buffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, screen_pixel_buffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(screen_pixels), NULL, GL_STATIC_READ);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, screen_pixel_buffer);

    program = glCreateProgram();
    compute_shader = Utils::createShader(rtshader::raw, GL_COMPUTE_SHADER);
    glAttachShader(program, compute_shader);

    Utils::linkProgram(program);

}

unsigned int clampColor(float c) {
    return std::min(static_cast<unsigned int>(std::max(c,0.0f) * 255),(unsigned int) 255);
}

unsigned int colorf(vec4 color) {
    return  0xff000000 | clampColor(color.z) | clampColor(color.y) << 8 | clampColor(color.x) << 16;
}

void gpurt::render(Surface *screen) {

    static unsigned int count = 0;
    count++;
    float fac = 1.0 / count;
    glUseProgram(program);

    glUniform1ui(glGetUniformLocation(program, "frame_id"), count);
    glUniform1ui(glGetUniformLocation(program, "triangle_number"), scene1->triangleCount());

    glDispatchCompute(40, 40, 1);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, screen_pixel_buffer);
    glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(screen_pixels), screen_pixels);
    
    for (int y = 0; y < 640; y++) {
        for (int x = 0; x < 640; x++) {

            vec4 color = screen_pixels[y][x]*fac;
            screen->GetBuffer()[y * 640 + x] = colorf(color);
        }
    }
    
}