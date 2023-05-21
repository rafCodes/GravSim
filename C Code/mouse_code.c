// Modified from Professor Bruce Land's Mouse course code
// https://people.ece.cornell.edu/land/courses/ece5760/DE1_SOC/HPS_peripherials/
// HPS USB Programming


#include "mouse_code.h"

int fd;

/**
 * Get mouse connection setup
*/
int initalizeMouse()
{
    const char *pDevice = "/dev/input/mice";

    // Open Mouse
    fd = open(pDevice, O_RDWR);
    if(fd == -1)
    {
        printf("ERROR Opening %s\n", pDevice);
        return -1;
    }

    //needed for nonblocking read()
    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK); 
}

/**
 * Refresh mouse data
*/
int refreshMouseData(mouse_event * mouse_data){

    int left, middle, right;
    signed char x, y;
    char data[3];

    // Read Mouse     
    int bytes = read(fd, data, sizeof(data));

    // if there is new data
    if(bytes > 0)
    {
        // get button states
        left = data[0] & 0x1;
        right = data[0] & 0x2;
        middle = data[0] & 0x4;

        // get change in x and y
        x = data[1];
        y = data[2];

        // update mouse data
        mouse_data->dx = x;
        mouse_data->dy = -y;
        mouse_data->left = left > 0 ? 1 : 0;
        mouse_data->middle = middle > 0 ? 1 : 0;
        mouse_data->right = right > 0 ? 1 : 0;

        return 1;

    }
    else{
        // update mouse data with all zeros
        mouse_data->dx = 0;
        mouse_data->dy = 0;
        mouse_data->left = 0;
        mouse_data->middle = 0;
        mouse_data->right = 0;
    }

    return 0;
}

/**
 * Update cursor location based on new mouse data
*/
void updateCursorLocation(mouse_event * mouse_data, cursor_location * vga_cursor_location){

    if (refreshMouseData(mouse_data) == 0){
        // no new data
        return;
    }

    // update vga_cursor_location
    int locationX = vga_cursor_location->x + mouse_data->dx;
    int locationY = vga_cursor_location->y + mouse_data->dy;

    // check if cursor is out of bounds
    if(locationX < 0){
        locationX = 0;
    }
    else if(locationX > 639){
        locationX = 639;
    }
    if(locationY < 0){
        locationY = 0;
    }
    else if(locationY > 479){
        locationY = 479;
    }

    // save old location for erasing
    vga_cursor_location->old_x = vga_cursor_location->x;
    vga_cursor_location->old_y = vga_cursor_location->y;

    // update location
    vga_cursor_location->x = locationX;
    vga_cursor_location->y = locationY;
}