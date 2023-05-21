#include "utils.h"
#include "vga_code.h"

/**
 * Fast inverse square-root - 1/sqrt(x)
 * From https://en.wikipedia.org/wiki/Fast_inverse_square_root
*/
float Q_rsqrt( float number ) {
    long i;
    float x2, y;
    const float threehalfs = 1.5F;
 
    x2 = number * 0.5F;
    y  = number;
    i  = *(long *)&y;           // treat float number as a long integer
    i  = 0x5F375A86 - ( i >> 1 ); // initial guess for Newton's method
    y  = *(float *)&i;         // treat long integer as a float number
    y  = y * ( threehalfs - ( x2 * y * y ) ); // one iteration of Newton's method
    return y;
}


/**
 * Radius to color - the larger the radius, the brighter the color
*/
unsigned int radiusToColor(int radius){
    return (unsigned int) (0x00FFFFFF - (radius * 0x00010101));
}

/**
 * Import initial values for celestial bodies from a csv file called celestial_bodies.csv
 * Takes in a pointer to the array of celestial bodies and number of bodies
*/
void importCelestialBodies(char fileName[],  celestial_body c_b[], int num_bodies, int pixel_to_au, float time_step){

    // open the file
    FILE *fp = fopen(fileName, "r");

    // check if the file opened
    if(fp == NULL){
        printf("Error opening file\n");

    // if the file opened
    } else {
        // for each body in the simulation
        // https://stackoverflow.com/questions/61078280/how-to-read-a-csv-file-in-c
        // create a temporary array to save the 6 floats
        int currentCelestialBody = -1;

        float temp[6];
        char buffer[BUFFER_SIZE];
        int index = 0; // current var in temp array


        while (fgets(buffer, BUFFER_SIZE, fp) && currentCelestialBody < num_bodies) {
            // If you need all the values in a row

            index = 0;

            char *token = strtok(buffer, ",");
            while (token) { 
                // add to buffer
                temp[index] = atof(token);

                index++;

                token = strtok(NULL, ",");
            }

            // to avoid getting the header
            if (currentCelestialBody != -1){
                // great, now fill in to the celestial body
                c_b[currentCelestialBody].in_view = 1;
                c_b[currentCelestialBody].x = temp[0]*pixel_to_au;
                c_b[currentCelestialBody].y = temp[1]*pixel_to_au;
                c_b[currentCelestialBody].r =       1;  // radii not used
                c_b[currentCelestialBody].r_orig =  1;  // radii not used
                c_b[currentCelestialBody].old_r =   1;  // radii not used
                c_b[currentCelestialBody].m = temp[3]*10.0;
                c_b[currentCelestialBody].vx = temp[4];
                c_b[currentCelestialBody].vy = temp[5];

                // set rest of values to 0

                // c_b[currentCelestialBody].f_prev_x = 0.0;
                // c_b[currentCelestialBody].f_prev_y = 0.0;

                // c_b[currentCelestialBody].f_curr_x = 0.0;
                // c_b[currentCelestialBody].f_curr_y = 0.0;

                // set the color based on the radius to be able to distinguish between bodies
                // 5 factor added to improve color range
                c_b[currentCelestialBody].color = radiusToColor((int)(temp[2]*5));

                c_b[currentCelestialBody].temp_x_pixel = 0;
                c_b[currentCelestialBody].temp_y_pixel = 0;

                c_b[currentCelestialBody].old_x_pixel = 0;
                c_b[currentCelestialBody].old_y_pixel = 0;
                
                c_b[currentCelestialBody].time_step_over_mass = time_step / c_b[currentCelestialBody].m;
                c_b[currentCelestialBody].valid_celestial_body = 1;

                c_b[currentCelestialBody].x_accel = 0;
                c_b[currentCelestialBody].y_accel = 0;

            }

            currentCelestialBody++;
        }

        // close the file
        fclose(fp);

        // verify that actually loaded the correct number of bodies
        if(currentCelestialBody != num_bodies){
            printf("Error: incorrect number of bodies loaded");

            // throw an error
            exit(1);
        }

        // the last body for the mouse only needs to be cleared to all zeros
        c_b[num_bodies].in_view = 0;
        c_b[num_bodies].x = 100;
        c_b[num_bodies].y = 100;
        c_b[num_bodies].r = 100;
        c_b[num_bodies].r_orig = 0;
        c_b[num_bodies].old_r =  0;
        c_b[num_bodies].m =  0;
        c_b[num_bodies].vx = 0;
        c_b[num_bodies].vy = 0;
        // c_b[num_bodies].f_prev_x = 0;
        // c_b[num_bodies].f_prev_y = 0;

        // c_b[num_bodies].f_curr_x = 0;
        // c_b[num_bodies].f_curr_y = 0;

        // mouse celestial body is white
        c_b[num_bodies].color = white;

        c_b[num_bodies].temp_x_pixel = 0;
        c_b[num_bodies].temp_y_pixel = 0;

        c_b[num_bodies].old_x_pixel = 0;
        c_b[num_bodies].old_y_pixel = 0;
        
        c_b[num_bodies].time_step_over_mass = 1;

        c_b[num_bodies].valid_celestial_body = 1;

        c_b[num_bodies].x_accel = 0;
        c_b[num_bodies].y_accel = 0;
    }
}

/**
 * Update mouse celestial body
*/
void updateMousePlanet(celestial_body * c_b, int pixel_to_au, float mass, int x, int y, int radius){

    // update the mouse celestial body location information
    c_b->in_view = 1;
    c_b->x = x*pixel_to_au; // convert to au from pixels
    c_b->y = y*pixel_to_au; // convert to au from pixels
    c_b->old_r = c_b->r;
    c_b->r = radius;
    c_b->r_orig = radius;
    c_b->m = mass;
}

/**
 * Update celestial body time step over mass
*/
void updateCelestialBodyTimeStep(celestial_body c_b[], int num_bodies, float time_step){
    // for each body
    int i;
    for(i = 0; i < num_bodies; i++){
        // update the time step over mass
        c_b[i].time_step_over_mass = time_step / c_b[i].m;
    }
}

/**
 * Print a celestial body's data
*/
void printCelestialBody(celestial_body * c_b){
    // print data of the current body
    printf("x pixels: %d, y pixels: %d\n", c_b->temp_x_pixel, c_b->temp_y_pixel);
    printf("x location: %f, y location: %f\n", c_b->x, c_b->y);
    printf("r: %d\n", c_b->r);
    printf("color: %d\n", c_b->color);
    printf("vx: %f, vy: %f\n", c_b->vx, c_b->vy);
    printf("f_curr_x: %f, f_curr_y: %f\n", c_b->f_curr_x, c_b->f_curr_y);
    printf("time step over mass: %f\n", c_b->time_step_over_mass);
    printf("f_prev_x: %f, f_prev_y: %f\n", c_b->f_prev_x, c_b->f_prev_y);
    printf("m: %f\n", c_b->m);
}

/**
 * Update if the celestial body is on screen
*/
void updateCelestialInView(celestial_body c_bs[], int pixel_to_au, int screen_width, int screen_height, int num_bodies){
    // for each body

    int i;

    for (i = 0; i < num_bodies; i++){
        // check if it is on screen
        c_bs[i].in_view = check_if_on_screen(&c_bs[i], pixel_to_au, screen_width, screen_height);
    }
}

/**
 * Check if a celestial body is on screen
*/
int check_if_on_screen(celestial_body *c_b, int pixel_to_au, int screen_width, int screen_height){

    // convert current distance to pixels
    c_b->temp_x_pixel = c_b->x / pixel_to_au;
    c_b->temp_y_pixel = c_b->y / pixel_to_au;

    // if the center of the body is on screen
    if (c_b->temp_x_pixel > 0 && c_b->temp_x_pixel < screen_width && c_b->temp_y_pixel > 0 && c_b->temp_y_pixel < screen_height){
        return 1;
    }

    // more intense check
    // radius IS in pixels
    int xMax = c_b->temp_x_pixel + c_b->r;
    int xMin = c_b->temp_x_pixel - c_b->r;

    // radius IS in pixels
    int yMax = c_b->temp_y_pixel + c_b->r;
    int yMin = c_b->temp_y_pixel - c_b->r;

    // is there a sliver of it on the screen?
    if (xMax < 0 || xMin > screen_width || yMax < 0 || yMin > screen_height){
        // no, not on the screen
        return 0;
    }

    return 1;
}


/**
 * Find closest celestial body to a x, y point
*/
int findClosestCelestialBody(celestial_body c_b[], int num_bodies, int x, int y){
    // find the closest celestial body to a x, y point

    int closest = 0; // index of closest body
    double closest_distance_squared = 409600; // off screen, 640*640
    int i;

    // for each body
    for (i = 0; i < num_bodies; i++){
        // calculate distance from the x, y point

        // difference in x, y
        int x_difference = pow(c_b[i].temp_x_pixel - x, 2);
        int y_difference = pow(c_b[i].temp_y_pixel - y, 2);

        // one over distance
        double distance_squared = x_difference + y_difference;

        // if it is closer than the current closest
        if (distance_squared < closest_distance_squared){
            // update closest
            closest = i;
            closest_distance_squared = distance_squared;
        }
    }
    
    return closest;
}



/**************************************************************************
 * Mark Eiding mje56                                                      *
 * ECE 5760                                                               *
 * Modified IEEE single precision FP                                      *
 * bit 26:      Sign     (0: pos, 1: neg)                                 *
 * bits[25:18]: Exponent (unsigned)                                       *
 * bits[17:0]:  Fraction (unsigned)                                       *
 *  (-1)^SIGN * 2^(EXP-127) * (1+.FRAC)                                   *
 * (http://en.wikipedia.org/wiki/Single-precision_floating-point_format)  *
 * Adapted from Skyler Schneider ss868                                    *
 *************************************************************************/
// from: https://people.ece.cornell.edu/land/courses/ece5760/DE1_SOC/HPS_peripherials/Examples_version_18.html
// Implementing on-chip memory
// Convert a C floating point into a 27-bit register floating point.
unsigned int floatToReg27(float f) {
    int f_f = (*(int*)&f);
    int f_sign = (f_f >> 31) & 0x1;
    int f_exp = (f_f >> 23) & 0xFF;
    int f_frac = f_f & 0x007FFFFF;
    int r_sign;
    int r_exp;
    int r_frac;
    r_sign = f_sign;
    if((f_exp == 0x00) || (f_exp == 0xFF)) {
        // 0x00 -> 0 or subnormal
        // 0xFF -> infinity or NaN
        r_exp = 0;
        r_frac = 0;
    } else {
        r_exp = (f_exp) & 0xFF;
        r_frac = ((f_frac >> 5)) & 0x0003FFFF;
    }
    return (r_sign << 26) | (r_exp << 18) | r_frac;
}

// Convert a 27-bit register floating point into a C floating point.
float reg27ToFloat(unsigned int r) {
    int sign = (r & 0x04000000) >> 26;
    unsigned int exp = (r & 0x03FC0000) >> 18;
    int frac = (r & 0x0003FFFF);
    float result = pow(2.0, (float) (exp-127.0));
    result = (1.0+(((float)frac) / 262144.0)) * result;
    if(sign) result = result * (-1);
    return result;
}


///////////////////////////////////////////// 
