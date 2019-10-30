#include <iostream>
#include "camera.h"
#include "figures.h"


int main(int argc, char** argv) {
    /*Parametros invocacion
     * resolucion imagen
     * rayos por pixel
     * imagen salida
     * formato imagen salida
     *
     if () {
        std::cerr << "Usage: ··· " << std::endl;
        return 1;
        }
     */


    Vec4 origenC(1,1,1,1);
    Vec4 rayo (0,1,0,0);
    Figures::Plane plano(Vec4 (0,1,0,0),5);
    Vec4 interseccion = plano.Intersection(rayo,origenC);

    std::cout << interseccion << std::endl;


    return 0;
}