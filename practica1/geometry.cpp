
#include "geometry.h"

Mat4::Mat4() {
    for (int i = 0; i < 16; ++i) {
        raw[i]=0.0;
    }
}

Mat4::Mat4(Vec4 u, Vec4 v, Vec4 w, Vec4 o) {
    for (int i = 0; i < 4; ++i) {
       m[0][i]= u.raw[i];
    }
    for (int i = 0; i < 4; ++i) {
        m[1][i]= v.raw[i];
    }
    for (int i = 0; i < 4; ++i) {
        m[2][i]= w.raw[i];
    }
    for (int i = 0; i < 4; ++i) {
        m[3][i]= o.raw[i];
    }
}

/* 1        0           0       0
* 0    cos _incl  -sin _incl   0
* 0    sin _incl  cos _incl    0
* 0        0           0       1*/
Mat4 rotationX(float _incl){
    return Mat4 (Vec4 (1,0,0,0), Vec4 (0, std::cos(_incl), std::sin(_incl),0), Vec4 (0, - std::sin(_incl), std::cos(_incl),0),
            Vec4 (0,0,0,1));
}
Mat4 rotationY(float _incl){
    return Mat4 (Vec4 (std::cos(_incl),0,-std::sin(_incl),0), Vec4 (0, 1, 0,0), Vec4 (std::sin(_incl), 0, std::cos(_incl),0),
                 Vec4 (0,0,0,1));
}
Mat4 rotationZ(float _incl){
    return Mat4 (Vec4 (std::cos(_incl),std::sin(_incl),0,0), Vec4 (-std::sin(_incl), std::cos(_incl), 0,0), Vec4 (0, 0, 1,0),
                 Vec4 (0,0,0,1));
}

Mat4 Mat4::translation(float xT, float yT, float zT) {
    Mat4 res;
    res.raw[0]=1;
    res.raw[3]=xT;
    res.raw[5]=1;
    res.raw[7]=yT;
    res.raw[10]=1;
    res.raw[11]=zT;
    res.raw[15]=1;
}

Mat4 scale(float xS, float yS, float zS){
    Mat4 res;
    res.raw[0]=xS;
    res.raw[5]=yS;
    res.raw[10]=zS;
    res.raw[15]=1;
}


Mat4 changeOfBasis(Vec4 u, Vec4 v, Vec4 w, Vec4 o){
    return Mat4 (u,v,w,o);
}

