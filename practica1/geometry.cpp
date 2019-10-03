
#include "geometry.h"

Mat4::Mat4() {
    for (int i = 0; i < 16; ++i) {
        raw[i]=0.0;
    }
}

Mat4::Mat4(Vec4 u, Vec4 v, Vec4 w, Vec4 o) {
    for (int i = 0; i < 4; ++i) {
       mat[0][i]= u.raw[i];
    }
    for (int i = 0; i < 4; ++i) {
        mat[1][i]= v.raw[i];
    }
    for (int i = 0; i < 4; ++i) {
        mat[2][i]= w.raw[i];
    }
    for (int i = 0; i < 4; ++i) {
        mat[3][i]= o.raw[i];
    }
}

