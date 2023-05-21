// Modified from Professor Bruce Land's Mouse course code
// https://people.ece.cornell.edu/land/courses/ece5760/DE1_SOC/HPS_peripherials/
// HPS USB Programming

#ifndef MOUSE_CODE_H
#define MOUSE_CODE_H

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

#define cursor_len 2

// defines the mouse event struct
typedef struct mouse_event {

    // change in x and y
    int dx;
    int dy;

    // button states
    int left;
    int middle;
    int right;

} mouse_event;

// defines the cursor struct
typedef struct cursor_location {

    // current cursor location
    int x;
    int y;

    // old cursor location
    int old_x;
    int old_y;
    
} cursor_location;


int initializeMouse();
int refreshMouseData(mouse_event * mouse_data);
void updateCursorLocation(mouse_event * mouse_data, cursor_location * vga_cursor_location);

#endif