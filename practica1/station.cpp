#include "station.h"

#include <iostream>

Station::Station(Vec4 _center, Vec4 _axis, Vec4 _reference, float _inclination, float _azimuth)
: center(_center), axis(_axis), reference(_reference) {
    Vec4 d = reference - center;
    if (axis.module() / 2 != d.module()) {
        std::cerr << "Invalid planet parameters" << std::endl;
    }

    Vec4 kPlanet = cross(axis, d).normalize();
    Vec4 iPlanet = cross(axis, kPlanet).normalize(); // o al reves
    Vec4 jPlanet = axis.normalize();

    // s será el vector del centro a la estacion en UCS
    Vec4 s(0.0f, d.module(), 0.0f, 0.0f);
    // rotar s alrededor de iPlanet en (latitud) radianes
    // rotar s alrededor de jPlanet en (long) radianes

    // hacer la matriz de cambio de base (expresar coord. planeta en coords ucs, ya lo tenemos)

    // center + Matriz * s es la posicion de la estación en UCS

    // para la base de la estación:
    // k es la normal (s - center).normalize();
    // i es (k * axis)
    // j es (k * i ó i * k) una de las dos

}