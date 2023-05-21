#ifndef DRAWING_H
#define DRAWING_H

// used for drawing the celestial bodies, buttons, and mouse cursor
#include "vga_code.h"
#include "utils.h"
#include "celestial_body.h"
#include "buttons.h"
#include "mouse_code.h"

void eraseCelestialBodies(celestial_body c_b[], int num_bodies);
void drawCelestialBodies(celestial_body c_b[], int num_bodies);
void redrawMouseCursor(cursor_location * c_l);
void redrawCelestialBodies(celestial_body c_b[], int num_bodies, int pixel_to_au, int width, int height, int erase_toggle);
void drawButton(button_struct * button);
void redrawButtons(button_struct buttons[], int num_buttons);
void redrawText(int num_bodies, int mouse_enabled, float mouse_mass, float time_step, int fpga_cycle_time, int frame_time);

#endif
