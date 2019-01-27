#include"precomp.h"
#include"GPURT_Shader.h"


const char* rtshader::raw = make_str(#version 430\n
layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) in;
layout(std430, binding = 0) buffer PIXEL_COLOR_BUFFER {
    vec4 pixel_color[640][640];
};
struct Vertex {
    vec3 pos;
    int _r;
    vec3 normal;
    int _s;
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

const float very_large_float = 1e9f;

uniform uint frame_id;
uniform uint triangle_number;

struct Ray {
    vec3 pos;
    vec3 dir;
};

uint random_seed;
float xorshift32() {
    random_seed ^= random_seed << 13;
    random_seed ^= random_seed >> 17;
    random_seed ^= random_seed << 5;
    return  random_seed * 2.3283064365387e-10f;
}

vec4 randomDirection() {
    float longitude = xorshift32()*3.1415926535897932384626433832795 * 2;
    float lattitude = xorshift32()*3.1415926535897932384626433832795;
    float z = sin(lattitude);
    float s = cos(lattitude);
    return vec4(s*cos(longitude), s*sin(longitude), z, z);
}

void primaryRay(in uint x, in uint y, out Ray ray) {
    ray.dir = normalize(vec3(float(x) + xorshift32(), float(640 - y) + xorshift32(), -320.0) - vec3(320.0, 320.0, 0));
    ray.pos = vec3(0, 3, 7);
}

bool intersectAABB(vec3 aabb_min,vec3 aabb_max,Ray ray) {
    vec3 origin = ray.pos;
    vec3 dir = ray.dir;
    float local_tMin = 0.001;
    float tMax = very_large_float;
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

// .xy: uv, .z: distance
vec3 intersectTriangle(Ray r, Triangle t) {

    mat3 A = mat3(t.v1.pos.xyz - t.v2.pos.xyz, t.v1.pos.xyz - t.v3.pos.xyz, r.dir);

    int d = int(abs(determinant(A))<0.0001);

    vec3 l = inverse(A) * (t.v1.pos.xyz - r.pos);


    l.z = (int(l.x < 0 || l.y < 0 || ((l.x + l.y) > 1))+ d)*very_large_float + l.z*(1-d);

    return l;
}

vec3 triangleNormal(Triangle t, vec2 uv) {
    float l = uv.x + uv.y;
    return mix(t.v1.normal, mix(t.v2.normal, t.v3.normal, uv.y / l), l);
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


struct IntersectScene {
    float distance;
    vec3 normal;
    int triangle_id;
};

IntersectScene intersectScene(Ray ray) {
    stack_push(0);

    IntersectScene result;
    result.distance = very_large_float;

    vec2 uv;
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
                    int tid = j + bvh[i].left_first;
                    vec3 c = intersectTriangle(ray, triangles[tid]);
                    if (c.z < result.distance) {
                        result.distance = c.z;
                        result.triangle_id = tid;
                        uv = c.xy;
                    }
                }
            }
        }
    }
    result.normal = triangleNormal(triangles[result.triangle_id],uv);
    result.normal = result.normal*(2 * (int(dot(result.normal, ray.dir) < 0) - 0.5));
    return result;
}

void main(void) {

    stack_top = 0;

    uint largePrime1 = 386030683;
    uint largePrime2 = 919888919;
    uint largePrime3 = 101414101;

    random_seed = ((gl_GlobalInvocationID.x * largePrime1 + gl_GlobalInvocationID.y) * largePrime1 + frame_id * largePrime3);

    Ray ray;
    primaryRay(gl_GlobalInvocationID.x, gl_GlobalInvocationID.y, ray);
    vec3 color = vec3(1);

    for (int j = 0; j < 10; j++)
    {
        IntersectScene ict = intersectScene(ray);
        if (ict.distance< very_large_float) {
            color = color * vec3(0.5);
            if (triangles[ict.triangle_id].v1._r == 1) {
                color = color * vec3(30);
                break;
            }
        }
        else {
            color = vec3(0);
            break;
        }

        ray.pos = ray.pos + ict.distance*ray.dir;

        vec4 randomDir = randomDirection();

        vec3 local_z = ict.normal;
        vec3 local_x = vec3(local_z.y, local_z.z, local_z.x);
        vec3 local_y = cross(local_z, local_x);
        local_x = cross(local_y, local_z);
        local_x = normalize(local_x);
        local_y = normalize(local_y);

        ray.dir = local_x * randomDir.x + local_y * randomDir.y + local_z * randomDir.z;
        ray.pos = ray.pos + 0.001*ray.dir;
    }

    pixel_color[gl_GlobalInvocationID.y][gl_GlobalInvocationID.x] = vec4(color, 1) + pixel_color[gl_GlobalInvocationID.y][gl_GlobalInvocationID.x];
}\n

);