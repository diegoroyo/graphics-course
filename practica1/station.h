#include "geometry.h"

class Station {
    // información del planeta
    // center, axis, refcity
    Vec4 center, axis, reference;

    // información de la estacion
    // posicion (ucs)
    Vec4 positionUCS;
    // tangente long (i)
    // tangente lat (j)
    // normal (k)
    Vec4 i, j, k;
    Station(Vec4 center, Vec4 axis, Vec4 reference, float inclination, float azimuth);
}