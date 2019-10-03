#pragma once
#include <cmath>
#include <ostream>
#include <vector>



struct Vec4 {
    union {
        struct {
            float x, y, z, w;
        };
        float raw[4];
    };

    Vec4() : x(0), y(0), z(0), w(0) {}
    Vec4(float _w) : x(0), y(0), z(0), w(_w) {}
    Vec4(float _x, float _y, float _z, float _w) : x(_x), y(_y), z(_z), w(_w) {}
    inline Vec4 operator+(const Vec4 &other) const {
        return Vec4(x + other.x, y + other.y, z + other.z, w + other.w);
    }
    inline Vec4 operator-(const Vec4 &other) const {
        return Vec4(x - other.x, y - other.y, z - other.z, w - other.w);
    }
    inline Vec4 operator*(const float f) const {
        return Vec4(x * f, y * f, z * f, w * f);
    }
    float module() const { return std::sqrt(x * x + y * y + z * z); }
    Vec4 normalize() {
        return Vec4(*this) * (1 / module());
    }
    inline float operator[](const int i) const{
        return raw[i];
    }
    friend std::ostream &operator<<(std::ostream &s, Vec4 &vector);
};

float dot(const Vec4& u, const Vec4 &v);
Vec4 cross(const Vec4& u, const Vec4 &v);

struct Mat4{
   union {
       struct {
           float m[4][4];
       };
       float raw[16];
   };

    Mat4();

    Mat4(Vec4 u, Vec4 v, Vec4 w, Vec4 o);
    inline float* operator[](const int i) {
        return m[i];
    }
    //Multiplicacion de matrices

    inline Mat4 operator*(const Mat4& a) const {
        Mat4 res;
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                for (int k = 0; k < 4; k++) {
                    res[i][j] += m[i][k] * a.m[k][j];
                }
            }
        }
        return res;
    }

    inline Vec4 operator* (const Vec4& v) const {
        Vec4 res;
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                res.raw[i]= m[i][j]*v[j];
            }
        }
        return res;
    }
    // matriz rotacion (eje, angulo)
    static Mat4 rotationX(float _incl);
    static Mat4 rotationY(float _incl);
    static Mat4 rotationZ(float _incl);

    //matriz traslacion (x, y, z)
    static Mat4 translation(float xT, float yT, float zT);

    //matriz escalado (x,y,z)
    static Mat4 scale(float xS, float yS, float zS);
    //TODO: matriz inversa


    //matriz cambio de base
    static Mat4 changeOfBasis(Vec4 u, Vec4 v, Vec4 w, Vec4 o);



};
