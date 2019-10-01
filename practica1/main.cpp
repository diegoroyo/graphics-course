#include "station.h"
#include "geometry.h"

#include <fstream>
#include <iostream>
#include <sstream>

Station loadStation(std::ifstream& is) {
    char kk;
    Vec4 center(1), axis(0), reference(1);
    float inclination, azimuth;
    is << kk << center.x << center.y << center.z;
    is << kk << axis.x << axis.y << axis.z;
    is << kk << reference.x << reference.y << reference.z;
    is << kk << inclination;
    is << kk << azimuth;
    Station station(center, axis, reference, inclination, azimuth);
}

int main() {
    std::ifstream is("stations.txt");
    if (!is.is_open()) {
        std::cerr << "Can't open file stations.txt" << std::endl;
        is.close();
    }
    Station station1 = loadStation(is);
    Station station2 = loadStation(is);

    // tenemos bases ijk de ambas estaciones
    // calcular v (posicion estacion 2 - estacion 1)
    // pasar v a base de estacion 1 (expresar UCS en base de est. 1)
    // lo mismo para estacion 2
    
    // se choca con el planeta si prod. escalar k * dir(salida|entrada) es < 0
}