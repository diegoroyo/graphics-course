//
// Created by Yasmina on 30/10/2019.
//

#ifndef PRACTICA3_FIGURES_H
#define PRACTICA3_FIGURES_H


#include "geometry.h"
#include "rgbcolor.h"

class Figures {
    RGBColor emission;
private:
    class Sphere{
        Vec4 center;
        float radius;
    public:
        Sphere(const Vec4 &center, float radius);
        //funcion interseccion con vector --> utilizar funcion implicita
    };

    class Plane{
        Vec4 normal;
        float distToOrigin;
    public:
        Plane(const Vec4 &normal, float distToOrigin);
        //funcion interseccion --> ver si el vector y la normal no son perpendiculares?? origen y distancias?
    };
};


#endif //PRACTICA3_FIGURES_H
