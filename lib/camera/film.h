#pragma once

#include <iostream>
#include "math/geometry.h"
#include "math/random.h"

// Contains info. about the camera's film
// and various utilities to help generate rays
struct Film {
    const Vec4 origin, forward, up, right;
    Mat4 cob;                   // change of basis
    const int width, height;    // pixels (width x height)
    const Vec4 d0;              // position of film(-1, 1) in global coords
    const Vec4 deltaX, deltaY;  // 1/2 of a pixel's width/height in global scale
    float dofRadius = 0.0f;     // depth of field effect

    // Use explicit right vector (aspect ratio might be off)
    Film(int _width, int _height, Vec4 _origin, Vec4 _forward, Vec4 _up,
         Vec4 _right)
        : width(_width),
          height(_height),
          origin(_origin),
          forward(_forward),
          up(_up),
          right(_right),
          cob(Mat4::changeOfBasis(right, up, forward, origin)),
          d0(localToWorld(Vec4(-1.0f, 1.0f, 1.0f, 0.0f))),
          deltaX(localToWorld(Vec4(2.0f / width, 0, 0.0f, 0.0f))),
          deltaY(localToWorld(Vec4(0, -2.0f / height, 0.0f, 0.0f))) {}

    // Use aspect ratio instead of fixed right vector
    Film(int _width, int _height, Vec4 _origin, Vec4 _forward, Vec4 _up)
        : Film(_width, _height, _origin, _forward, _up,
               cross(_forward, _up).normalize() * _up.module() *
                   (_width / (float)_height)) {}

    // Add DoF effect (in a more explicit way)
    void setDoFRadius(const float _radius) { this->dofRadius = _radius; }

    // Convert camera's local space to world space
    inline Vec4 localToWorld(const Vec4 &v) const { return cob * v; }

    // Get random DoF displacement (simulate bigger camera hole)
    inline Vec4 getDoFDisplacement() const {
        if (dofRadius == 0.0f) {
            return Vec4();
        } else {
            float r = dofRadius * sqrtf(random01());
            Mat4 rotZ = Mat4::rotationZ(random01() * 2.0f * M_PI);
            return localToWorld(rotZ * Vec4(0.0f, r, 0.0f, 0.0f));
        }
    }

    inline Vec4 getPixelCenter(int px, int py) const {
        return d0 + deltaX * px + deltaY * py;
    }
};