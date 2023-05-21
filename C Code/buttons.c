#include "buttons.h"
#include "vga_code.h"

/**
 * Compute the top left and bottom right corners of the button
*/
void prepareButtons(button_struct button_array[], int button_count){

    // add each button
    int i;
    for(i = 0; i < button_count; i++){
        button_array[i].top_left_x = button_array[i].x_center - (button_array[i].width / 2);
        button_array[i].top_left_y = button_array[i].y_center - (button_array[i].height / 2);
        button_array[i].bottom_right_x = button_array[i].x_center + (button_array[i].width / 2);
        button_array[i].bottom_right_y = button_array[i].y_center + (button_array[i].height / 2);

        if (button_array[i].has_symbol){
            button_array[i].smallest_side_over_2 = button_array[i].width < button_array[i].height ? button_array[i].width/3 : button_array[i].height/3;

            if (button_array[i].smallest_side_over_2 < 0){
                printf("Error: Button id: %d has shape size of 0, are you sure you want a button symbol with no width or height? If not, do not enable the symbol\n", i);
                exit(1);
            }
        }

        // check for invalid button location
        if(button_array[i].top_left_x < 0 || button_array[i].top_left_y < 0 || button_array[i].bottom_right_x < 0 || button_array[i].bottom_right_y < 0){
            printf("Error: Button id: %d has a negative coordinate, please pick a new center, width, and/or height\n", i);
            exit(1);
        }
    }
}

/**
 * Check if a button was pressed and if it was, return the button id, else return -1
*/
int check_button_press(button_struct buttons[], int num_buttons, int x, int y){
    int i;
    for(i = 0; i < num_buttons; i++){

        // make sure button is active
        if (buttons[i].active == 1){

            // check if the x and y coordinates are within the button
            if (x >= buttons[i].top_left_x && x <= buttons[i].bottom_right_x){
                if (y >= buttons[i].top_left_y && y <= buttons[i].bottom_right_y){
                    return i;
                }
            }
        }
    }
    return -1;
}