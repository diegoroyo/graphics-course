#include <fstream>
#include <iostream>
#include <sstream>

#include "math/geometry.h"
#include "station.h"

Station loadStation(std::ifstream& is) {
    char kk;
    Vec4 center(1), axis(0), reference(1);
    float inclination, azimuth;
    is >> kk >> center.x >> center.y >> center.z;
    is >> kk >> axis.x >> axis.y >> axis.z;
    is >> kk >> reference.x >> reference.y >> reference.z;
    is >> kk >> inclination;
    is >> kk >> azimuth;
    return Station(center, axis, reference, inclination, azimuth);
}

int main() {
    std::ifstream is("stations3.txt");
    if (!is.is_open()) {
        std::cerr << "Can't open file stations.txt" << std::endl;
        is.close();
    }
    Station station1 = loadStation(is);
    Station station2 = loadStation(is);

    // tenemos bases ijk de ambas estaciones
    // calcular v (posicion estacion 2 - estacion 1)
    Vec4 v = station2.positionUCS - station1.positionUCS;
    // pasar v a base de estacion 1 (expresar UCS en base de est. 1)
    Mat4 cob1 = Mat4::changeOfBasis(station1.i, station1.j, station1.k,
                                    station1.positionUCS);
    Vec4 outDir = (cob1 * v).normalize();
    // lo mismo para estacion 2
    Mat4 cob2 = Mat4::changeOfBasis(station2.i, station2.j, station2.k,
                                    station2.positionUCS);
    Vec4 incDir = (cob2 * (v * -1.0f)).normalize();

    std::cout << "Outgoing direction: " << outDir << std::endl;
    std::cout << "Incoming direction: " << incDir << std::endl;

    // se choca con el planeta si prod. escalar k * dir(salida|entrada) es < 0
    // importante que los dos estén en el mismo sistema de coordenadas
    if (dot(station1.k, v) < 0)
        std::cout << "Collision in outgoing direction" << std::endl;
    if (dot(station2.k, v) > 0)
        std::cout << "Collision in incoming direction" << std::endl;

    // test para matrices inversas
    Mat4 mat;
    mat[0][3] = 2;
    mat[1][2] = 3;
    mat[2][1] = 4;
    mat[3][0] = 5;
    // mostrar original, transpuesta, inversa y inversa*original
    std::cout << mat << std::endl;
    Mat4 mattr = mat.transpose();
    std::cout << mattr << std::endl;
    Mat4 matinv = mat.inverse();
    std::cout << matinv << std::endl;
    Mat4 iden = matinv * mat;
    // A^-1 * A deberia ser la matriz identidad
    std::cout << iden << std::endl;
}