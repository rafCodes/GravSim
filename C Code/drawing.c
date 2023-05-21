// used for drawing shapes, buttons, etc

#include "drawing.h"

//////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////// CELESTIAL BODIES //////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////

/**
 * Erase the celestial bodies
*/
void eraseCelestialBodies(celestial_body c_b[], int num_bodies){
    // for each body in the simulation
    int i;

    for(i = 0; i < num_bodies; i++){

        // only need to erase the body if it was on the screen
        if (c_b[i].in_view == 1){

            // erase the star
            VGA_star(c_b[i].old_x_pixel, 
                        c_b[i].old_y_pixel, 
                        black);
        }
    }
}

/**
 * Draw the celestial bodies to the screen, updates old radius and old pixel location
*/
void drawCelestialBodies(celestial_body c_b[], int num_bodies){

    // for each body in the simulation
    int i;

    for(i = 0; i < num_bodies; i++){

        // check if the body is on the screen
        if (c_b->in_view == 1){

            // draw the star
            VGA_star(c_b[i].temp_x_pixel, 
                        c_b[i].temp_y_pixel, 
                        c_b[i].color);

            // some alternatives to VGA_star, same parameters
            // VGA_super_star
            // VGA_warped_world

            // update old pixel location - only matters if drawn
            c_b[i].old_x_pixel = c_b[i].temp_x_pixel;
            c_b[i].old_y_pixel = c_b[i].temp_y_pixel;
        }
    }
}

/**
 * Redraw the celestial bodies
*/
void redrawCelestialBodies(celestial_body c_b[], int num_bodies, int pixel_to_au, int width, int height, int erase_toggle){

    if (erase_toggle == 1){
        // erase the old bodies
        eraseCelestialBodies(c_b, num_bodies);
    }

    // check if the bodies are on the screen
    updateCelestialInView(c_b, pixel_to_au, width, height, num_bodies);
    
    // draw the new bodies
    drawCelestialBodies(c_b, num_bodies);
}

///////////////////////////////////////////////////////////////////////////
////////////////////////////////// MOUSE //////////////////////////////////
///////////////////////////////////////////////////////////////////////////

/**
 * Redraw the mouse cursor to the screen
*/
void redrawMouseCursor(cursor_location * c_l){
    // erase the old cursor square
    VGA_box(    c_l->old_x +cursor_len, 
                c_l->old_y +cursor_len,
                c_l->old_x -cursor_len,
                c_l->old_y -cursor_len,
                black);

    // draw the new cursor square
    VGA_box(    c_l->x +cursor_len, 
                c_l->y +cursor_len,
                c_l->x -cursor_len,
                c_l->y -cursor_len,
                green);
}

////////////////////////////////// BUTTONS //////////////////////////////////

/**
 * Draw all buttons to the screen
*/
void redrawButtons(button_struct buttons[], int num_buttons){

    // for each button
    int i;

    for(i = 0; i < num_buttons; i++){

        // erase the button if not active - buttons cannot be on top of eachother otherwise this will erase the wrong button
        if (buttons[i].active == 0){
            VGA_box(buttons[i].top_left_x, 
                    buttons[i].top_left_y, 
                    buttons[i].bottom_right_x, 
                    buttons[i].bottom_right_y, 
                    black);
        }

        // draw the button if active
        if (buttons[i].active == 1){
            // draw the button
            drawButton(&buttons[i]);
        }
    }
}

/**
 * Draw a button to the screen
*/
void drawButton(button_struct * button){

    // check if the button has text (no erasing, just redrawing if deleted by accident)
    if (button->has_text){
        // draw the text
        VGA_text(button->text_x, 
                button->text_y, 
                button->text);
    }

    // draw the button if it has a color
    if (button->color != clear){
        VGA_box(button->top_left_x, 
                button->top_left_y, 
                button->bottom_right_x, 
                button->bottom_right_y, 
                button->color);
    }

    // check if button has a symbol, can be embedded inside the button
    if (button->has_symbol == 1){

        // draw the symbol - circle or square

        if (button->symbol == CIRCLE){
            VGA_disc(button->x_center, 
                        button->y_center, 
                        button->smallest_side_over_2, 
                        button->symbol_color);
        }

        else if (button->symbol == SQUARE){
            VGA_box(button->x_center - button->smallest_side_over_2, 
                    button->y_center - button->smallest_side_over_2, 
                    button->x_center + button->smallest_side_over_2, 
                    button->y_center + button->smallest_side_over_2, 
                    button->symbol_color);
        }
    }
}

////////////////////////////// TEXT //////////////////////////////////

/**
 * Redraw all debug/info text to the screen
*/
void redrawText(int num_bodies, int mouse_enabled, float mouse_mass, float time_step, int erasing_toggle, int frame_time){
	VGA_text_clear();

    // create char label buffers
    char num_bodies_text[40] = "Number of bodies: ";
    char time_step_text[40] = "Time step: ";
    char erasing_toggle_text[40] = "Erasing Toggle: ";
    char frame_time_text[40] = "Frame time (us): ";

    char mouse_enabled_text[40] = "Mouse enabled: ";
    char mouse_mass_text[40] = "Mouse mass: ";


	// add labels
    VGA_text(5, 1, num_bodies_text);
    VGA_text(5, 2, time_step_text);
    VGA_text(5, 3, erasing_toggle_text);
    VGA_text(5, 4, frame_time_text);

    VGA_text(5, 5, mouse_enabled_text);

    // only display mouse mass label if mouse is enabled
    if (mouse_enabled == 1){
        VGA_text(5, 6, mouse_mass_text);
    }

    // now add the values via sprintf after defining them
    char num_bodies_value[40];
    char time_step_value[40];
    char erasing_toggle_value[40];
    char frame_time_value[40];

    char mouse_enabled_value[40];
    char mouse_mass_value[40];

    sprintf(num_bodies_value, "%d", num_bodies);
    sprintf(time_step_value, "%f", time_step);
    sprintf(erasing_toggle_value, "%d", erasing_toggle);
    sprintf(frame_time_value, "%d", frame_time);

    sprintf(mouse_enabled_value, "%d", mouse_enabled);

	// Display data next to label
    VGA_text (25, 1, num_bodies_value);
    VGA_text (25, 2, time_step_value);
    VGA_text (25, 3, erasing_toggle_value);
    VGA_text (25, 4, frame_time_value);

    VGA_text (25, 5, mouse_enabled_value);

    // only display mouse mass if mouse is enabled
    if (mouse_enabled == 1){
        sprintf(mouse_mass_value, "%f", mouse_mass);
        VGA_text (25, 6, mouse_mass_value);
    }
}
