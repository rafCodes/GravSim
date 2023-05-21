#ifndef GRAVITY_SIM_H
#define GRAVITY_SIM_H

#include <stdio.h>  // file handling functions
#include <stdlib.h> // atoi
#include <math.h>   // powf
#include <float.h>  // FLT_MAX

#include "celestial_body.h"
#include "utils.h"

void acceleration_timestep(celestial_body c_b[], int num_bodies, float time_step);
void integrate_acceleration(celestial_body * body, float time_step);

void hps_accel_update(celestial_body c_b[], int num_bodies, int debug);
void calculate_hps_accel(celestial_body *body1, celestial_body *body2, int index1, int index2, int debug);

/**

MVP:

- extend color scheme by multiplying by 10?

frame_time - need to calc this 

- need number of bodies in each map - enum and return from importing - done
- buttons for other maps - one button for all cycling - added

- VGA blank is a pio, if only write to screen when it is blank, will get no flicker
- resetting the FPGA and creating PIO ports
- PIO port communication
- keep 1/30 seconds per update
    50,000,000 cycles per second
    8 interactions / cycle
    3500 objects max theoretically

 - preset maps
    - all read from the csv file(s)
        - binary star system
        - orbiting
        - proto disk (beginging of a solar system)
            - bunch of thingies spiraling in with some intial angular momentum
            - velocities around a center of mass 
        - Our solar system?

Sent to FPGA (3600 max):
- how many objects there are now - just before the write enable, once every cycle before data is sent
- write enable
    loop
    - index of object
    - Float Position X
    - Float Position y
    - Float Mass
    - Collision (body) radius (r_orig) (standard radius) (in same units as position (AU go brrr))
    - then send next index in this order to avoid the issue we avoided in drum lab of value in wrong place

- write disable

Receive from FPGA immediately after sending:
- wait until index of object changes
- index of object
- Float accel X
- Float accel Y
- then send next index in this order to avoid the issue we avoided in drum lab of value in wrong place
- object index that collided with or -1 if no collision
- then send next index in this order to avoid the issue we avoided in drum lab of value in wrong place




Extras:

    erase toggle for trails

    changing au to pixels with a button



    empty qsysy thing to see how fast can send data or if can memcopy into the pio ports - bruce
    so, use linux to get the physical address of each pio port


    HPS Collision process/things (only inbetween updates):
    - Preserve center of mass 
        x = (m1x1 + m2x2)/(m1+m2) 
        y = (m1y1 + m2y2)/(m1+m2)
    - Preserve momentum (p = mv)
    - vx = (m1*vx1 + m2*vx2)/(m1+m2)
    - vy = (m1*vy1 + m2*vy2)/(m1+m2)
    - Sum Masses
    - (m1+m2)
    - Change radius to reflect new mass

    - squishes the two objects together so need to do smart merge within the list by replacing the one with
    the earlier index
    - also need to keep an eye on the mouse planet to make sure it doesn't eat things


Spawning planets
    - does not get added to spawning state
            
    - Spawn planets
        - make sure to not accidentally write over the mouse planet
        - vector not important with doing
        - add 3 types of bodies - middle mouse enable planet placment
            - left is asteroid
            - right gas giant
        - remember to not add palent to graivty calculations until velocity has been decided (mass 0 initally?)
        - first click places, second adds velocity vector in that direction to said planet
        - onscreen button to leave mode


scrolling


*/


#endif


