#ifndef UTILS_H
#define UTILS_H

#include "celestial_body.h"

#include <stdio.h>  // file handling functions
#include <stdlib.h> // atoi
#include <string.h> // strtok
#include <math.h>   // pow

#define BUFFER_SIZE 100

float Q_rsqrt( float number );
unsigned int radiusToColor(int radius);
void importCelestialBodies(char fileName[], celestial_body celestial_bodies[], int num_bodies, int pixel_to_au, float time_step);
void printCelestialBody(celestial_body *c_b);
int check_if_on_screen(celestial_body *c_b, int pixel_to_au, int screen_width, int screen_height);
void updateCelestialInView(celestial_body c_bs[], int pixel_to_au, int screen_width, int screen_height, int num_bodies);
int findClosestCelestialBody(celestial_body c_b[], int num_bodies, int x, int y);
void updateCelestialBodyTimeStep(celestial_body c_b[], int num_bodies, float time_step);
void updateMousePlanet(celestial_body * c_b, int pixel_to_au, float mass, int x, int y, int radius);
unsigned int floatToReg27(float f);
float reg27ToFloat(unsigned int r);

#endif
