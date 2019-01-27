#include"precomp.h"
#include"GPURT_Shader.h"


const char* rtshader::raw = make_str(#version 430\n
layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) in;
layout(std430, binding = 0) buffer PIXEL_COLOR_BUFFER {
    vec4 pixel_color[640][640];
};
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
layout(std430, binding = 1) buffer TRIANGLE_BUFFER {
    Triangle triangles[];
};

struct BVHNode {
    vec3 aabb_min;
    int left_first;
    vec3 aabb_max;
    int count;
};
layout(std430, binding = 2) buffer BVH_BUFFER {
    struct {
        vec3 aabb_min;
        int left_first;
        vec3 aabb_max;
        int count;
    }bvh[];
};

uniform uint frame_id;
uniform uint triangle_number;

struct Ray {
    vec3 pos;
    vec3 dir;
};

float xorshift32(inout uint seed) {
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

void primaryRay(in uint x, in uint y, out Ray ray, inout uint seed) {
    ray.dir = normalize(vec3(float(x) + xorshift32(seed), float(640 - y) + xorshift32(seed), -320.0) - vec3(320.0, 320.0, 0));
    ray.pos = vec3(0, 2, 5);

}

bool intersectAABB(vec3 aabb_min, vec3 aabb_max, Ray ray) {
    vec3 origin = ray.pos;
    vec3 dir = ray.dir;
    float local_tMin = 0.001;
    float tMax = 999099.0;
    for (int i = 0; i < 3; i++) {
        float invD = 1.0f / dir[i];
        float t0 = (aabb_min[i] - origin[i]) * invD;
        float t1 = (aabb_max[i] - origin[i]) * invD;
        if (invD < 0.0f) {
            float tmp = t0;
            t0 = t1;
            t1 = tmp;
        }

        local_tMin = max(t0, local_tMin);
        tMax = min(t1, tMax);
        if (tMax < local_tMin)
            return false;
    }
    return true;
}

float intersectTriangle(inout Ray r, inout Triangle t) {

    mat3 A = mat3(t.v1.pos.xyz - t.v2.pos.xyz, t.v1.pos.xyz - t.v3.pos.xyz, r.dir);

    if (abs(determinant(A)) < 0.0001)
        return 100000.0;

    vec3 l = inverse(A) * (t.v1.pos.xyz - r.pos);

    if (l.x < 0 || l.y < 0 || ((l.x + l.y) > 1)) {
        return 100000.0;
    }
    return l.z;
}

int stack[32];
int stack_top;
int stack_pop() {
    stack_top --;
    return stack[stack_top];
}
void stack_push(int i) {
    stack[stack_top] = i;
    stack_top++;
}

float intersectScene(Ray ray) {
    stack_push(0);
    float result = 999999.0;
    while (stack_top > 0) {
        int i = stack_pop();
        if (intersectAABB(bvh[i].aabb_min, bvh[i].aabb_max, ray)) {
            if (bvh[i].count == 0) {
                stack_push(bvh[i].left_first);
                stack_push(bvh[i].left_first + 1);
            }
            else {
                //result = 3.0 / (bvh[i].count);
                for (int j = 0; j < bvh[i].count; j++) {
                    float c = intersectTriangle(ray, triangles[j+bvh[i].left_first]);
                    if (c < result) {
                        result = c;
                    }
                }
            }
        }
    }
    return result;
}

void main(void) {

    stack_top = 0;

    uint largePrime1 = 386030683;
    uint largePrime2 = 919888919;
    uint largePrime3 = 101414101;

    uint random_seed = ((gl_GlobalInvocationID.x * largePrime1 + gl_GlobalInvocationID.y) * largePrime1 + frame_id * largePrime3);

    Ray ray;
    primaryRay(gl_GlobalInvocationID.x, gl_GlobalInvocationID.y, ray, random_seed);
    vec3 color = vec3(1);


    for (int j = 0; j < 6; j++)
    {
        float distance = 10000.0;
        vec3 intersect_pos = vec3(0);
        vec3 intersect_normal = vec3(0);
        vec3 intersect_color = vec3(0);
        int intersect_sphere = -1;
        /*for (int i = 0; i < triangle_number; i++) {
            if (intersectTriangle(ray, triangles[i]) < distance) {
                intersect_color = triangles[i].v1.normal;
            }
        }*/
        float l = intersectScene(ray);
        if (l< 10000.0) {
            intersect_color = vec3(l);
        }

        color = color * intersect_color;

    }

    pixel_color[gl_GlobalInvocationID.y][gl_GlobalInvocationID.x] = vec4(color, 1) + pixel_color[gl_GlobalInvocationID.y][gl_GlobalInvocationID.x];
}\n

);