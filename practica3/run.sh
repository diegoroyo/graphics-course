#!/bin/bash
time bin/pathtracer -w 1600 -h 1600 -p 256 -o out/spaceship.ppm
echo "Convirtiendo a PNG..."
../practica2/bin/tonemapper out/spaceship.ppm -t CLAMP_GAMMA 0.85 0.8 -o out/spaceship.png -p
echo "Done"
