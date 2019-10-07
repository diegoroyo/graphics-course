#include "geometry.h"

/// Vectors ///

std::ostream &operator<<(std::ostream &s, Vec4 &vector) {
    s << "(" << vector.x << ", " << vector.y << ", "
             << vector.z << ", " << vector.w << ")";
    return s;
}

// dot product (ignores 4th component)
float dot(const Vec4 &u, const Vec4 &v) {
    return u.x * v.x + u.y * v.y + u.z * v.z;
}
// cross product (ignores 4th component)
Vec4 cross(const Vec4 &u, const Vec4 &v) {
    return Vec4(u.y * v.z - u.z * v.y,  // uyvz-uzvy
                u.z * v.x - u.x * v.z,  // uzvx-uxvz
                u.x * v.y - u.y * v.x,  // uxvy-uyvx
                0.0f);                  // make it a direction vector
}

/// Matrices ///

// fill constructor
Mat4::Mat4(float elem) {
    for (int i = 0; i < 16; ++i) {
        raw[i] = elem;
    }
}

// column vectors constructor
Mat4::Mat4(Vec4 &u, Vec4 &v, Vec4 &w, Vec4 &o) {
    for (int i = 0; i < 4; ++i) {
        m[i][0] = u.raw[i];
    }
    for (int i = 0; i < 4; ++i) {
        m[i][1] = v.raw[i];
    }
    for (int i = 0; i < 4; ++i) {
        m[i][2] = w.raw[i];
    }
    for (int i = 0; i < 4; ++i) {
        m[i][3] = o.raw[i];
    }
}

/**
 *  1 0 0 0
 *  0 1 0 0
 *  0 0 1 0
 *  0 0 0 1
 */
Mat4 Mat4::identity() {
    Mat4 iden;
    for (int i = 0; i < 4; ++i) {
        iden[i][i] = 1.0f;
    }
    return iden;
}

/**
 *  1 0 0 x
 *  0 1 0 y
 *  0 0 1 z
 *  0 0 0 1
 */
Mat4 Mat4::translation(float xT, float yT, float zT) {
    Mat4 res = Mat4::identity();
    res.raw[3] = xT;
    res.raw[7] = yT;
    res.raw[11] = zT;
    return res;
}

/* 
 * 1   0     0   0
 * 0  cos  -sin  0
 * 0  sin   cos  0
 * 0   0     0   1
 */
Mat4 Mat4::rotationX(float rad) {
    Mat4 res = Mat4::identity();
    res.raw[5] = std::cos(rad);
    res.raw[6] = -std::sin(rad);
    res.raw[9] = std::sin(rad);
    res.raw[10] = std::cos(rad);
    return res;
}

/* 
 *  cos  0  sin  0
 *   0   1   0   0
 * -sin  0  cos  0
 *   0   0   0   1
 */
Mat4 Mat4::rotationY(float rad) {
    Mat4 res = Mat4::identity();
    res.raw[0] = std::cos(rad);
    res.raw[2] = std::sin(rad);
    res.raw[8] = -std::sin(rad);
    res.raw[11] = std::cos(rad);
    return res;
}

/* 
 * cos  -sin  0  0
 * sin   cos  0  0
 *  0     0   1  0
 *  0     0   0  1
 */
Mat4 Mat4::rotationZ(float rad) {
    Mat4 res = Mat4::identity();
    res.raw[0] = std::cos(rad);
    res.raw[1] = -std::sin(rad);
    res.raw[4] = std::sin(rad);
    res.raw[5] = std::cos(rad);
    return res;
}

/**
 * x 0 0 0
 * 0 y 0 0
 * 0 0 z 0
 * 0 0 0 1
 */
Mat4 Mat4::scale(float xS, float yS, float zS) {
    Mat4 res = Mat4::identity();
    res.raw[0] = xS;
    res.raw[5] = yS;
    res.raw[10] = zS;
    return res;
}

/**
 * ux vx wx ox
 * uy vy wy oy
 * uz vz wz oz
 * uw vw ww ow
 * 
 * If directions/points are correct, then
 * uw = vw = ww = 0, ow = 1
 */
Mat4 Mat4::changeOfBasis(Vec4 &u, Vec4 &v, Vec4 &w, Vec4 &o) {
    return Mat4(u, v, w, o);
}

std::ostream &operator<<(std::ostream &s, Mat4 &matrix) {
    s << std::endl;
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            s << matrix[i][j] << ' ';
        }
        s << std::endl;
    }
    s << std::endl;
    return s;
}