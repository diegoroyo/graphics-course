//
// Created by Yasmina on 30/10/2019.
//

#ifndef PRACTICA3_FIGURES_H
#define PRACTICA3_FIGURES_H


#include "geometry.h"
#include "rgbcolor.h"

class Figures {
    RGBColor emission;
public:
    class Sphere{
        Vec4 center;
        float radius;
    public:
        Sphere(const Vec4 &center, float radius);
        //funcion interseccion con vector --> utilizar funcion implicita
        Vec4 Intersection(Vec4 v, Vec4 oc);
    };
public:
    class Plane{
        Vec4 normal;
        float distToOrigin;
    public:
        Plane(const Vec4 &normal, float distToOrigin);
        //Si existe devuelve el punto de interseccion entre el plano y v
        // Si no existe devuelve el punto (0,0,0)
        Vec4 Intersection(Vec4 v, Vec4 oc);

    };
};


#endif //PRACTICA3_FIGURES_H
