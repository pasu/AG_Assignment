#include "precomp.h"

#include "Utils.h"
#include <limits>
#include <cmath>

const float Utils::MAX_FLOAT = numeric_limits<float>::max();
const float Utils::MIN_FLOAT = numeric_limits<float>::min();
const float Utils::RT_PI = 3.1415926535897f;
const float Utils::EPSILON_FLOAT = 1e-6f;

bool Utils::solveQuadratic(const float a, const float b, const float c, float &r0, float &r1)
{
	float discr = b * b - 4 * a * c;
	if (discr < 0)
		return false;
	else if (discr == 0)
		r0 = r1 = -0.5f * b / a;
	else {
		float q = (b > 0) ?
			-0.5f * (b + sqrt(discr)) :
			-0.5f * (b - sqrt(discr));
		r0 = q / a;
		r1 = c / q;
	}
	if (r0 > r1)
		swap(r0, r1);
	return true;
}

GLuint Utils::createShader(const char* source, GLenum type, const char* errinfo) {
    int len = std::strlen(source);

    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, &len);
    glCompileShader(shader);
    GLint testVal = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &testVal);
    if (testVal == GL_FALSE)
    {
        char infoLog[1024];
        glGetShaderInfoLog(shader, 1024, NULL, infoLog);
        glDeleteShader(shader);
        printf("shader error (%s) : %s\n", errinfo, infoLog);
        return 0;
    }
    return shader;
}

void Utils::linkProgram(GLuint program, const char* errinfo) {
    GLint testVal;
    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &testVal);
    if (testVal == GL_FALSE) {
        char infoLog[1024];
        glGetProgramInfoLog(program, 1024, NULL, infoLog);
        glDeleteProgram(program);

        printf("link error (%s) : %s\n", errinfo, infoLog);
    }
}