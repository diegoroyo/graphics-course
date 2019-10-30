//
// Created by Yasmina on 30/10/2019.
//

#pragma once


#include "../lib/geometry.h"
#include "../lib/ppmimage.h"

class Camera {
    Vec4 origin;
    Mat4 basis; //base con vectores u, l y f--3 vectores
    /* se puede construir la base a partir de la resolucion para mantener la proporcion entre los vectores de la
     * base y la resolucion en ancho y largo*/
    //resolucion ya en PPMImage
    PPMImage generated;
    int rpp; // rayos por pixel
    //

};

