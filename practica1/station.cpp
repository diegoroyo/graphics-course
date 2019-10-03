#include "station.h"

#include <iostream>

Station::Station(Vec4 _center, Vec4 _axis, Vec4 _reference, float _inclination, float _azimuth)
: center(_center), axis(_axis), reference(_reference) {
    Vec4 d = reference - center; // d: vector normal a la superficie en la ciudad de referencia
    if (axis.module() / 2 != d.module()) {
        std::cerr << "Invalid planet parameters" << std::endl;
    }

    Vec4 kPlanet = cross(axis, d).normalize();
    Vec4 iPlanet = cross(axis, kPlanet).normalize(); // o al reves
    Vec4 jPlanet = axis.normalize();

    // s será el vector del centro a la estacion en UCS
    Vec4 s(0.0f, d.module(), 0.0f, 0.0f); //base del planeta


    // rotar s alrededor de iPlanet en (latitud) radianes

    /* matriz transformacion:
     * 1        0           0       0
     * 0    cos _incl  -sin _incl   0
     * 0    sin _incl  cos _incl    0
     * 0        0           0       1
     */


    s = Mat4::rotationX(_inclination) * s;
    // rotar s alrededor de jPlanet en (long) radianes
    s = Mat4::rotationY(_azimuth) * s;

    /* matriz transformacion:
    * cos _azi      0    sin _azi     0
    * 0             1       0         0
    * -sin_azith    0   cos _azith    0
    * 0             0       0         1
    */

    // hacer la matriz de cambio de base (expresar coord. planeta en coords ucs, ya lo tenemos)
    Mat4 cob = Mat4::changeOfBasis(iPlanet, jPlanet, kPlanet, center);

    /* matriz cambio base:
      * iPlanet jplanet kplanet _center  x
      *                                  y
      *                                  z
      *                                  0|1
      */


    // center + Matriz * s es la posicion de la estación en UCS
    positionUCS = center + cob * s;

    // para la base de la estación:
    // k es la normal (s - center).normalize();
    k = (positionUCS - center).normalize();
    // i es (k * axis)
    i = cross(k, axis);
    // j es (k * i ó i * k) una de las dos
    j = cross(k, i);
}