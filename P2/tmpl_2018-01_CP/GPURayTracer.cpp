#include"precomp.h"
#include"GPURayTracer.h"

#define make_str(x) #x

static const char* shader_source = make_str(#version 430\n
    layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) in;

    struct Vertex {
        vec3 pos;
        int padding;
        vec3 normal;
        int padding_1;
    };

    struct Triangle {
        Vertex v1;
        Vertex v2;
        Vertex v3;
    };


    layout(std430, binding = 0) buffer TRIANGLE_BUFFER
    {
        Triangle triangles[];
    };
    layout(std430, binding = 1) buffer PIXEL_COLOR_BUFFER
    {
        vec4 pixel_color[640][640];
    };

    uniform uint frame_id;
    uniform uint triangle_number;

    struct Ray
    {
        vec3 pos;
        vec3 dir;
    };


    float xorshift32(inout uint seed)
    {
        seed ^= seed << 13;
        seed ^= seed >> 17;
        seed ^= seed << 5;
        return  seed * 2.3283064365387e-10f;
    }

    vec4 randomDirection(inout uint seed) {
        float longitude = xorshift32(seed)*3.1415926535897932384626433832795 * 2;
        float lattitude = xorshift32(seed)*3.1415926535897932384626433832795;
        float z = sin(lattitude);
        float s = cos(lattitude);
        return vec4(s*cos(longitude), s*sin(longitude), z, z);
    }
    void primaryRay(in uint x, in uint y, out Ray ray, inout uint seed)
    {
        ray.dir = normalize(vec3(float(x) + xorshift32(seed), float(640 - y) + xorshift32(seed), -320.0) - vec3(320.0, 320.0, 0));
        ray.pos = vec3(0, 2, 20);

    }

    float intersectTriangle(inout Ray r, inout Triangle t ){
        
        mat3 A = mat3(t.v1.pos.xyz - t.v2.pos.xyz, t.v1.pos.xyz - t.v3.pos.xyz, r.dir);

        if (abs(determinant(A)) < 0.0001)
            return 100000.0;

        vec3 l = inverse(A) * (t.v1.pos.xyz - r.pos);

        if (l.x < 0 || l.y < 0 || ((l.x + l.y) > 1) ) {
            return 100000.0;
        }
        return l.z;
    }
    void main(void) {
        uint largePrime1 = 386030683;
        uint largePrime2 = 919888919;
        uint largePrime3 = 101414101;

        uint random_seed = ((gl_GlobalInvocationID.x * largePrime1 + gl_GlobalInvocationID.y) * largePrime1 + frame_id * largePrime3);

        Ray ray;
        primaryRay(gl_GlobalInvocationID.x, gl_GlobalInvocationID.y, ray, random_seed);
        vec3 color = vec3(1);


        for (int j = 0; j < 2; j++)
        {
            float distance = 10000.0;
            vec3 intersect_pos = vec3(0);
            vec3 intersect_normal = vec3(0);
            vec3 intersect_color = vec3(0);
            int intersect_sphere = -1;
            for (int i = 0; i < triangle_number; i++) {
                if (intersectTriangle(ray, triangles[i])<distance) {
                    intersect_color = triangles[i].v1.normal;
                }
            }

            color = color * intersect_color;

        }

        pixel_color[gl_GlobalInvocationID.y][gl_GlobalInvocationID.x] = vec4(color, 1) + pixel_color[gl_GlobalInvocationID.y][gl_GlobalInvocationID.x];
    }\n

);

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
    glGenBuffers(1, &pbo);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo);
    glBufferData(GL_PIXEL_UNPACK_BUFFER, sizeof(b), b, GL_STREAM_READ);

    scene1 = gpurt::Scene::initScene1();
    scene1->load();

    scene1->bind();

    glGenBuffers(1, &screen_pixel_buffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, screen_pixel_buffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(screen_pixels), NULL, GL_STATIC_READ);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, screen_pixel_buffer);

    program = glCreateProgram();
    compute_shader = Utils::createShader(shader_source, GL_COMPUTE_SHADER);
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