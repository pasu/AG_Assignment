#include"precomp.h"
#include"GPURayTracer.h"

#define make_str(x) #x

static const char* shader_source = make_str(#version 430  \n
    layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;
    layout(std430, binding = 0) buffer SPHERE_XYZR_BUFFER
    { 
        vec4 sphere_xyzr[4];  
    }; 
    layout(std430, binding = 1) buffer SPHERE_RGBA_BUFFER 
    {  
        vec4 sphere_rgba[4];  
    };  
    layout(std430, binding = 2) buffer PIXEL_COLOR_BUFFER 
    {  
        vec4 pixel_color[640][640];  
    };  

    uniform uint frame_id;
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
        return  seed*2.3283064365387e-10f;
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
        ray.dir = normalize(vec3(float(x)+ xorshift32(seed), float(640 - y)+ xorshift32(seed), -320.0) - vec3(320.0, 320.0, 0));
        ray.pos = vec3(0, 1, 0);

    };
    void main(void) 
    {  
        uint largePrime1 = 386030683;
        uint largePrime2 = 919888919;
        uint largePrime3 = 101414101;

        uint random_seed = (gl_GlobalInvocationID.x * 640 + gl_GlobalInvocationID.y + frame_id * largePrime2)*largePrime1;

        Ray ray;  
        primaryRay(gl_GlobalInvocationID.x, gl_GlobalInvocationID.y, ray,random_seed);
        vec3 color = vec3(1);


        for (int j = 0; j < 2; j++) 
        {
            float distance = 100000.0;
            vec3 intersect_pos = vec3(0);
            vec3 intersect_normal = vec3(0);
            vec3 intersect_color = vec3(0);
            int intersect_sphere = -1;
            for (int i = 0; i < 4; i++) 
            { 
                float a = dot(ray.dir,ray.dir);  
                float b = 2 * dot(ray.pos - sphere_xyzr[i].xyz,ray.dir);  
                float c = dot(ray.pos - sphere_xyzr[i].xyz, ray.pos - sphere_xyzr[i].xyz) - sphere_xyzr[i].w*sphere_xyzr[i].w;  
                float delta = b * b - 4 * a*c;  
                if (delta < 0) 
                {  
                    continue;  
                }
                float lambda = (-b - sqrt(delta)) / (2 * a);  
                if (lambda <= 0.001) 
                { 
                    continue;  
                } 
                if (lambda < distance) {
                    distance = lambda;
                    intersect_pos = ray.pos + ray.dir*lambda;
                    intersect_normal = intersect_pos - sphere_xyzr[i].xyz;
                    intersect_color = sphere_rgba[i].rgb;
                    intersect_sphere = i;
                }

            }

            float lambda = -ray.pos.y / ray.dir.y;
            if (lambda > 0.001) {
                if (lambda < distance) {
                    intersect_pos = ray.pos + ray.dir*lambda;
                    intersect_normal = vec3(0, 1, 0);
                    intersect_color = vec3(0.5, 0.5, 0.5);
                    intersect_sphere = -1;
                }
            }

            color = color * intersect_color;

            if (intersect_sphere == 3) {
                break;
            }


            vec4 randomDir = randomDirection(random_seed);

            vec3 local_z = intersect_normal;
            vec3 local_x = vec3(local_z.y, local_z.z, local_z.x);
            vec3 local_y = cross(local_z, local_x);
            local_x = cross(local_y, local_z);
            local_x = normalize(local_x);
            local_y = normalize(local_y);

            ray.dir = local_x * randomDir.x + local_y * randomDir.y + local_z * randomDir.z;
            ray.pos = intersect_pos+0.001*ray.dir;

            color = color * randomDir.w;
        }

        pixel_color[gl_GlobalInvocationID.y][gl_GlobalInvocationID.x] = vec4(color, 1)+ pixel_color[gl_GlobalInvocationID.y][gl_GlobalInvocationID.x];
    }

);

using namespace gpurt;


static GLuint screen_texture=0;


GLuint pbo;
unsigned char b[640][640][4];


static GLuint sphere_xyzr_buffer = 0;
static GLuint sphere_rgba_buffer = 0;
static GLuint screen_pixel_buffer = 0;

vec4 screen_pixels[640][640];


GLuint program;
GLuint compute_shader;

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

    vec4 sphere_xyzr[4] = {
        {0,1.1,-4,1},
        {-2,1.1,-4,1},
        {2,1.1,-4,1},
        {0,4, -3,0.5}
    };
    vec4 sphere_rgba[4] = {
        {1,0,0,1},
        {0,1,0,1},
        {0,0,1,1},
        {20,20,20,1}
    };

    glGenBuffers(1, &sphere_xyzr_buffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, sphere_xyzr_buffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(sphere_xyzr), sphere_xyzr, GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, sphere_xyzr_buffer);

    glGenBuffers(1, &sphere_rgba_buffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, sphere_rgba_buffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(sphere_rgba), sphere_rgba, GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, sphere_rgba_buffer);

    glGenBuffers(1, &screen_pixel_buffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, screen_pixel_buffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(screen_pixels), NULL, GL_STATIC_READ);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, screen_pixel_buffer);

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

    glDispatchCompute(20, 20, 1);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, screen_pixel_buffer);
    glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(screen_pixels), screen_pixels);
    
    for (int y = 0; y < 640; y++) {
        for (int x = 0; x < 640; x++) {

            vec4 color = screen_pixels[y][x]*fac;
            screen->GetBuffer()[y * 640 + x] = colorf(color);
        }
    }
    
}