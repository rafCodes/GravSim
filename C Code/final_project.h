#ifndef FINAL_PROJECT_H
#define FINAL_PROJECT_H

#include <stdio.h>  // file handling functions
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/ipc.h> 
#include <sys/shm.h> 
#include <sys/mman.h>
#include <signal.h>

#include "address_map_arm_brl4.h"
#include "mouse_code.h"
#include "vga_code.h"
#include "drawing.h"
#include "celestial_body.h"
#include "utils.h"
#include "gravity_sim.h"
#include "buttons.h"

void redraw();
void initializeSimulation();
void main();
void pauseSimulation();
void playSimulation();
void resetSimulation();
void quitSimulation();
void slowDownSimulation();
void speedUpSimulation();
void runButtonFunction(int button_index);
void processMouse();
void cycleSimulation();
void toggleFPGARunning();
void timeDebuggingEnd();
void cycleMap();
void receiveFromFPGA();
void sendToFPGA();
void sigintHandler(int sig_num);
void timeDelaying();
void toggleClear();


#define NUM_MAPS 4

// enum of map names
enum mapNames{
    CELESTIAL_BODY_MAX_MAP,
    CELESTIAL_BODY_SMALL_MAP,
    ORBITING_MAP,
    SOLAR_SYSTEM_MAP,
};

// dictionary of map file names
char map_file_names[NUM_MAPS][80] = {
    "StarMaps/celestial_objects_max.csv",
    "StarMaps/celestial_objects_small.csv",
    "StarMaps/orbital_system.csv",
    "StarMaps/spiral_system.csv",
};

// array of number of bodies in each map
int map_num_bodies[NUM_MAPS] = {
    2200,
    100,
    1009,
    1681,
};

#endif
