#pragma once

#include <array>
#include <cmath>
#include <iomanip>
#include <ostream>
#include <vector>

/// Vectors ///

struct Vec4 {
    union {
        struct {
            float x, y, z, w;
        };
        std::array<float, 4> raw;
    };

    constexpr Vec4() : x(0), y(0), z(0), w(0) {}
    constexpr Vec4(float _w) : x(0), y(0), z(0), w(_w) {}
    constexpr Vec4(float _x, float _y, float _z, float _w)
        : x(_x), y(_y), z(_z), w(_w) {}

    constexpr inline bool operator==(const Vec4 &other) const {
        return this->x == other.x && this->y == other.y && this->z == other.z &&
               this->w == other.w;
    }
    constexpr inline float operator[](const int i) const { return raw[i]; }
    constexpr inline Vec4 operator+(const Vec4 &other) const {
        return Vec4(x + other.x, y + other.y, z + other.z, w + other.w);
    }
    constexpr inline Vec4 operator-(const Vec4 &other) const {
        return Vec4(x - other.x, y - other.y, z - other.z, w - other.w);
    }
    constexpr inline Vec4 operator*(const float f) const {
        return Vec4(x * f, y * f, z * f, w * f);
    }
    constexpr float module() const { return sqrtf(x * x + y * y + z * z); }
    constexpr Vec4 normalize(float l = 1) {
        return Vec4(*this) * (l / module());
    }

    friend std::ostream &operator<<(std::ostream &s, const Vec4 &vector);
};

// dot product (ignores 4th component)
constexpr float dot(const Vec4 &u, const Vec4 &v) {
    return u.x * v.x + u.y * v.y + u.z * v.z;
}
// cross product (ignores 4th component)
constexpr Vec4 cross(const Vec4 &u, const Vec4 &v) {
    return Vec4(u.y * v.z - u.z * v.y,  // uyvz-uzvy
                u.z * v.x - u.x * v.z,  // uzvx-uxvz
                u.x * v.y - u.y * v.x,  // uxvy-uyvx
                0.0f);                  // make it a direction vector
}

/// Matrices ///

struct Mat4 {
    union {
        struct {
            std::array<std::array<float, 4>, 4> m;
        };
        std::array<float, 16> raw;
    };

    constexpr Mat4() : raw() {}                 // zero-constructor
    constexpr Mat4(std::array<float, 16> _raw)  // all elements constructor
        : raw(_raw) {}
    constexpr Mat4(const Vec4 &u, const Vec4 &v,  // columns constructor
                   const Vec4 &w, const Vec4 &o) 
        : raw({u.x, v.x, w.x, o.x,
               u.y, v.y, w.y, o.y,
               u.z, v.z, w.z, o.z,
               u.w, v.w, w.w, o.w}) {}
    Mat4(float elem);                           // set all elements to "elem"

    // Transformation matrices
    static constexpr Mat4 identity() {
        return Mat4(std::array<float, 16>({1.0f, 0.0f, 0.0f, 0.0f,
                                           0.0f, 1.0f, 0.0f, 0.0f,
                                           0.0f, 0.0f, 1.0f, 0.0f,
                                           0.0f, 0.0f, 0.0f, 1.0f}));
    }
    static constexpr Mat4 translation(float xT, float yT, float zT) {
        return Mat4(std::array<float, 16>({1.0f, 0.0f, 0.0f,   xT,
                                           0.0f, 1.0f, 0.0f,   yT,
                                           0.0f, 0.0f, 1.0f,   zT,
                                           0.0f, 0.0f, 0.0f, 1.0f}));
    }
    static constexpr Mat4 scale(float xS, float yS, float zS) {
        return Mat4(std::array<float, 16>({  xS, 0.0f, 0.0f, 0.0f,
                                           0.0f,   yS, 0.0f, 0.0f,
                                           0.0f, 0.0f,   zS, 0.0f,
                                           0.0f, 0.0f, 0.0f, 1.0f}));
    }
    static constexpr Mat4 rotationX(float rad) {
        return Mat4(std::array<float, 16>(
            {1.0f,       0.0f,       0.0f, 0.0f,
             0.0f,  cosf(rad), -sinf(rad), 0.0f,
             0.0f,  sinf(rad),  cosf(rad), 0.0f,
             0.0f,        0.0f,      0.0f, 1.0f}));
    }
    static constexpr Mat4 rotationY(float rad) {
        return Mat4(std::array<float, 16>(
            { cosf(rad), 0.0f,  sinf(rad), 0.0f,
                   0.0f, 1.0f,       0.0f, 0.0f,
             -sinf(rad), 0.0f,  cosf(rad), 0.0f,
                   0.0f, 0.0f,       0.0f, 1.0f}));
    }
    static constexpr Mat4 rotationZ(float rad) {
        return Mat4(std::array<float, 16>(
            { cosf(rad), -sinf(rad), 0.0f, 0.0f,
              sinf(rad),  cosf(rad), 0.0f, 0.0f,
                   0.0f,       0.0f, 1.0f, 0.0f,
                   0.0f,       0.0f, 0.0f, 1.0f}));
    }
    // Change to basis uvw with origin o
    // If directions/points are correct, then
    //   uw = vw = ww = 0, ow = 1
    static constexpr Mat4 changeOfBasis(const Vec4 &u, const Vec4 &v,
                                        const Vec4 &w, const Vec4 &o) {
        return Mat4(u, v, w, o);
    }

    inline std::array<float, 4> &operator[](const int i) {
        return m[i];
    }

    // matrix * matrix
    inline Mat4 operator*(const Mat4 &a) const {
        Mat4 res;
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                for (int k = 0; k < 4; k++) {
                    res.m[i][j] += m[i][k] * a.m[k][j];
                }
            }
        }
        return res;
    }
    // matrix * vector
    inline Vec4 operator*(const Vec4 &v) const {
        Vec4 res;
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                res.raw[i] += m[i][j] * v[j];
            }
        }
        return res;
    }
    // matrix * scalar
    inline Mat4 operator*(const float f) const {
        Mat4 res;
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                res[i][j] = m[i][j] * f;
            }
        }
        return res;
    }

    float cofactor(int row, int col) const;
    Mat4 transpose() const;
    Mat4 inverse() const;

    friend std::ostream &operator<<(std::ostream &s, Mat4 &matrix);
};
