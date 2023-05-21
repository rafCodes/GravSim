#include "gravity_sim.h"

/////////////////////////// INTEGRATION  ///////////////////////////

/**
 * Increment the simulation with respect to acceleration by 1 timestep
*/
void acceleration_timestep(celestial_body c_b[], int num_bodies, float time_step){

    int body_index;

    // integrate the acceleration over the timestep
    for(body_index = 0; body_index < num_bodies; body_index++){
        integrate_acceleration(&c_b[body_index], time_step);
    }
    
}

/**
 * Integrate the acceleration over a timestep
*/
void integrate_acceleration(celestial_body * body, float time_step){
    
        // a = f / m
        // del v = a * del t
        // v = v0 + del v
    
        // calculate the change in velocity
        float del_vx = body->x_accel * time_step;
        float del_vy = body->y_accel * time_step;
    
        // update the velocity
        body->vx += del_vx;
        body->vy += del_vy;
    
        // calculate the change in position
        float del_x = body->vx * time_step;
        float del_y = body->vy * time_step;
    
        // update the position
        body->x += del_x;
        body->y += del_y;
}

////////////////////////////////////////////////////////////////////////

//////////////////////////////  FORCE UPDATE  //////////////////////////////

/**
 * Calculate the force between all bodies in the simulation
 * Triangle calculation, force updates for both particles 
 * so only need to update particles with a larger index than current index
 * 
*/
void hps_accel_update(celestial_body c_b[], int num_bodies, int debug){
    
    // first clear the past accelerations
    int b;
    for (b = 0; b < num_bodies; b++){
        c_b[b].x_accel = 0;
        c_b[b].y_accel = 0;
    }

    int i;
    int j;

    // for each body in the simulation
    for(i = 0; i < num_bodies; i++){

        // for each other body in the simulation
        // N + N-1 + N-2 + ... + 1 updates
        for(j = i+1; j < num_bodies; j++){
            
            // calculate the force between the two bodies
            calculate_hps_accel(&c_b[i], &c_b[j], i, j, debug);
        }
    }
}

/**
 * Calculate the force between two bodies
*/
void calculate_hps_accel(celestial_body *body1, celestial_body *body2, int index1, int index2, int debug){

    // calculate the distance between the two bodies

    float x_diff = body1->x - body2->x;
    float y_diff = body1->y - body2->y;

    float distance_squared = (x_diff*x_diff) + (y_diff * y_diff);

    float one_over_r = Q_rsqrt(distance_squared);
    float eps_sq = 7.45058059692e-09;

    // done to avoid infinities
    float one_over_r_plus_eps_sq = Q_rsqrt(distance_squared + eps_sq);


    // calculate the force between the two bodies, G = 1 here
    float accel_precursor = distance_squared * one_over_r * one_over_r_plus_eps_sq * one_over_r_plus_eps_sq * one_over_r_plus_eps_sq;
    
    // calculate the angle between the two bodies without using atan2f (which is slow)
    if(accel_precursor > 10.0){
        // if need to scale down to avoid spikes in the force that cause bodies to fly off if they are too close
        // which does not happen in the real world
        accel_precursor = accel_precursor/10.0;
    }
    // use the ratio of the y and x components of the distance between the two bodies
    
    float x_unit_vector = x_diff * one_over_r;
    float y_unit_vector = y_diff * one_over_r;

    float force_x = accel_precursor * x_unit_vector;
    float force_y = accel_precursor * y_unit_vector;

    // subtract/add the force to the bodies
    body1->x_accel -= (force_x * body2->m/10.0);
    body1->y_accel -= (force_y * body2->m/10.0);
    
    body2->x_accel += (force_x * body1->m/10.0);
    body2->y_accel += (force_y * body1->m/10.0);
}
