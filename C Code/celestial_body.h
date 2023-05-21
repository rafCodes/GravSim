#ifndef CELESTIAL_BODY_H
#define CELESTIAL_BODY_H


// celestial body struct
typedef struct celestial_body{

    // if the celestial body is in view
    int in_view;

    // x location
    float x;

    // y location
    float y;
    
    // radius
    int r;

    // old radius
    int old_r;

    // original radius
    int r_orig;

    // mass
    float m;

    // x velocity
    float vx;

    // y velocity
    float vy;

    // time step / body mass
    float time_step_over_mass;

    // color
    unsigned int color;

    // pixel location
    int temp_x_pixel;
    int temp_y_pixel;

    // old pixel location
    int old_x_pixel;
    int old_y_pixel;

    // if the body is valid and was not merged
    int valid_celestial_body;

    // x acceleration
    float x_accel;

    // y acceleration
    float y_accel;

} celestial_body;

#endif