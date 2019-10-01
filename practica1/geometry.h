#include <cmath>
#include <ostream>

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
    Vec4 &normalize() {
        Vec4 copy(*this);
        copy = copy * (1 / module());
        return copy;
    }
    friend std::ostream &operator<<(std::ostream &s, Vec4 &vector);
};

// dot product
float dot(const Vec4& u, const Vec4 &v) const {
    return u.x * v.x + u.y * v.y + u.z * v.z;
}
// cross product
Vec4 cross(const Vec4& u, const Vec4 &v) const {
    return Vec4(u.y * v.z - u.z * v.y,   // uyvz-uzvy
                u.z * v.x - u.x * v.z,   // uzvx-uxvz
                u.x * v.y - u.y * v.x,   // uxvy-uyvx
                0.0f); // es dirección
}

std::ostream &operator<<(std::ostream &s, Vec4 &vector) {
    s << "(" << vector.x << ", " << vector.y << ", " << vector.z << ", " << vector.w << ")";
    return s;
}