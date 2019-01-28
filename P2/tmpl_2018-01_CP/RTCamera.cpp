#include "precomp.h"

void RTCamera::copyMCamera(mat4 & m) {
    for (int i = 0; i < 12; i++) {
        m[i] = mRotation[i];
    }
    for (int i = 0; i < 3; i++) {
        m[i + 12] = position[i];
    }
    m[15] = 1;
}
