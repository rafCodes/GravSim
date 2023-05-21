#ifndef BUTTONS_H
#define BUTTONS_H

#include <stdio.h>  // file handling functions
#include <stdlib.h> // atoi

// used for drawing shapes in the buttons
enum button_symbol{
    CIRCLE,
    SQUARE,
};

// used for button names and ids
enum button_names{
    RESET,
    SPEED_UP,
    SLOW_DOWN,
    PLAY,
    PAUSE,
    QUIT,
    CLEAR,
    CYCLE_MAP,
    FPGA_ON,
};

// what is in a button
typedef struct button_struct{

    // center of button
    int x_center;
    int y_center;

    // button dimensions
    int width;
    int height;

    // button color
    int color;

    // has text
    int has_text;

    // button text
    char text[20];

    // button text x and y
    int text_x;
    int text_y;

    // has symbol
    int has_symbol;

    // button symbol
    enum button_symbol symbol;

    // button symbol color
    int symbol_color;

    // button id
    int id;

    // active
    int active;

    // reduce computations for redrawing buttons by storing the top left and bottom right corners
    int top_left_x;
    int top_left_y;

    int bottom_right_x;
    int bottom_right_y;

    int smallest_side_over_2;

} button_struct;

void prepareButtons(button_struct button_array[], int button_count);
int check_button_press(button_struct buttons[], int num_buttons, int x, int y);

#endif