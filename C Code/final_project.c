///////////////////////////////////////
/// compile with
/// gcc utils.c vga_code.c mouse_code.c gravity_sim.c drawing.c buttons.c final_project.c -o finalRunMe -O2 -lm
///
///////////////////////////////////////

#include "final_project.h"

// if the mouse should pretend to be a planet
#define MOUSE_PLANET_ENABLED_DEFAULT 0
int mouse_planet_enabled = MOUSE_PLANET_ENABLED_DEFAULT;

#define MOUSE_PLANET_MASS_DEFAULT 10
float mouse_planet_mass = MOUSE_PLANET_MASS_DEFAULT; // is in earth masses, 16.744 is max

#define MOUSE_PLANET_RADIUS_DEFAULT 5
int mouse_planet_radius = MOUSE_PLANET_RADIUS_DEFAULT; // in pixels

////////////////////////////////////////////////////////////////////////
/////////////////////////// GLOBAL VARIABLES ///////////////////////////
////////////////////////////////////////////////////////////////////////

// enable/disable delaying the code to match 30 FPS
#define DELAY_FRAME 1

// override waiting for the screen to go blank before drawing
#define overrideBlanking 0

// print out debug informations
#define DEBUG 0

// print out the timing infomration for each frame and decrement
// the number of celestial bodies until the frame rate is 30 fps
#define TIMING_INFO 0

// print out the code needed to test the HPS output/FPGA input instructions in modelsim
#define INPUT_INST_DEBUG 0

// if the FPGA is being used for the gravity calculations
#define FPGA_ON_DEFAULT 1
int fpga_on = FPGA_ON_DEFAULT;

// width and height of the screen
#define WIDTH 640
#define HEIGHT 480

// Mass is in earths, 5.972e24 # kg
// distance is in AU, 1.496e11, which is one pixel # m

// how many bodies are in the simulation
// 465 is current max for 30 fps on HPS
// 2200 is current max for 30 fps on FPGA
#define NUM_BODIES_MAX 4000 
int NUM_BODIES = 0;

// the celestial bodies, +1 added for mouse body
celestial_body celestial_bodies[NUM_BODIES_MAX+1];

// the pixel to meters conversion
#define PIXEL_TO_AU_DEFAULT 1 // 1 is 1 AU
int pixel_to_au = PIXEL_TO_AU_DEFAULT;

// the time step
#define TIME_STEP_DEFAULT .1 // in seconds
float time_step = TIME_STEP_DEFAULT;

// if the simulation is running
int simulation_running = 1;

// current map index
int current_map_index = 0;

// erase toggle - if the celestial bodies should be erased before drawing
// setting this at 0 will cause the bodies to leave a trail
#define ERASE_TOGGLE_DEFAULT 1
int erase_toggle = ERASE_TOGGLE_DEFAULT;

///////////////////// BUTTONS /////////////////////

#define NUM_BUTTONS 9

#define button_y 20
#define button_x 345
#define button_width 25
#define button_height 18
#define button_space 50 // button width + 20

#define text_x 41
#define text_y 2
#define text_spacer 6

// set up the reset, speed up, slow down, play, pause, quit buttons
button_struct buttons[NUM_BUTTONS] = {
    // button x center              button y center button width        button height, clear, text enable, text,   text x,                       text y,   symbol enable, shape, color, id,       enabled, corner information, computer during intialization
    {button_x                     , button_y    ,   button_width + 18,  button_height, clear,           1, "RESET",text_x                      , text_y,   0,            CIRCLE, green, RESET    , 1,      0, 0, 0, 0, 0},
    {button_x+2 + button_space * 1, button_y    ,   button_width + 9 ,  button_height, clear,           1, "FAST" ,text_x + 1 + 1 * text_spacer, text_y,   0,            CIRCLE, green, SPEED_UP , 1,      0, 0, 0, 0, 0},
    {button_x+2 + button_space * 2, button_y    ,   button_width + 8 ,  button_height, clear,           1, "SLOW" ,text_x + 1 + 2 * text_spacer, text_y,   0,            CIRCLE, green, SLOW_DOWN, 1,      0, 0, 0, 0, 0},
    {button_x   + button_space * 3, button_y    ,   button_width     ,  button_height, clear,           0, "Play" ,text_x     + 3 * text_spacer, text_y,   1,            CIRCLE, white, PLAY     , 0,      0, 0, 0, 0, 0},
    {button_x   + button_space * 4, button_y    ,   button_width     ,  button_height, clear,           0, "Pause",text_x     + 4 * text_spacer, text_y,   1,            SQUARE, white, PAUSE    , 1,      0, 0, 0, 0, 0},
    {375   + button_space * 5 - 20, 445         ,   50               ,  button_height, clear,           1, "QUIT" ,text_x + 4 + 5 * text_spacer, 55    ,   0,            SQUARE, green, QUIT     , 1,      0, 0, 0, 0, 0},
    {button_x                     , button_y*2+3,   button_width + 18,  button_height, clear,           1, "CLEAR",text_x                      , text_y+3, 0,            CIRCLE, green, CLEAR    , 1,      0, 0, 0, 0, 0},
    {button_x+2 + button_space * 1, button_y*2+3,   button_width + 9 ,  button_height, clear,           1, "MAPS" ,text_x + 1 + 1 * text_spacer, text_y+3, 0,            CIRCLE, green, CYCLE_MAP, 1,      0, 0, 0, 0, 0},
    {button_x+2 + button_space * 2, button_y*2+3,   button_width + 8 ,  button_height, clear,           1, "FPGA" ,text_x + 1 + 2 * text_spacer, text_y+3, 0,            CIRCLE, green, FPGA_ON  , 1,      0, 0, 0, 0, 0},
};

/**
 * Button to toggle clearing the screen
*/
void toggleClear(){
    erase_toggle = !erase_toggle;

    if (erase_toggle){
        printf("Erasing on\n");
        VGA_clear_screen();
    }
    else{
        printf("Erasing off\n");
    }
}


/**
 * Button function to pause the simulation
*/
void pauseSimulation(){
    // hide the pause button and show the play button
    buttons[PAUSE].active = 0;
    buttons[PLAY].active = 1;
    simulation_running = 0;
}

/**
 * Button function to play the simulation
*/
void playSimulation(){
    // hide the play button and show the pause button
    buttons[PAUSE].active = 1;
    buttons[PLAY].active = 0;
    simulation_running = 1;
}

/**
 * Button function to quit the simulation
*/
void quitSimulation(){
    simulation_running = 0;
    VGA_text_clear();
    VGA_clear_screen();
    VGA_text(1, 1, "Quitting...");
    usleep(100000);
    usleep(100000);
    VGA_text_clear();
    VGA_clear_screen();
    exit(0);
}

/**
 * Button function to speed up the simulation
 * Double the time step
*/
void speedUpSimulation(){
    
    time_step *= 2;

    // update the time step for the bodies
    updateCelestialBodyTimeStep(celestial_bodies, NUM_BODIES, time_step);
}

/**
 * Button function to slow down the simulation
 * Halve the time step
*/
void slowDownSimulation(){
    time_step /= 2;

    // avoid tiny values
    if (time_step < .0001){
        time_step = .0001;
    }

    // update the time step for the bodies
    updateCelestialBodyTimeStep(celestial_bodies, NUM_BODIES, time_step);
}

/**
 * Toggle if the FPGA is running
*/
void toggleFPGARunning(){

    if (fpga_on){
        fpga_on = 0;
    } else {

        // make sure there are enough bodies for FPGA to be used
        // pipeline requires at least 64 bodies
        if (NUM_BODIES < 64){
            printf("Minumum number of bodies for FPGA is 64, FPGA will not be used\n");
        }
        else{
            fpga_on = 1;
        }
    }
}

/**
 * Cycle through the maps
*/
void cycleMap(){

    // increment the map index
    current_map_index++;

    // wrap around
    if (current_map_index >= NUM_MAPS){
        current_map_index = 0;
    }

    printf("Loading map %d\n", current_map_index);
    printf("Map name: %s\n\n", map_file_names[current_map_index]);

    // now reset the simulation
    resetSimulation();
}

/**
 * Run the correct button function
*/
void runButtonFunction(int button_index){
    switch(button_index){
        case RESET:
            resetSimulation();
            break;
        case SPEED_UP:
            speedUpSimulation();
            break;
        case SLOW_DOWN:
            slowDownSimulation();
            break;
        case PLAY:
            playSimulation();
            break;
        case PAUSE:
            pauseSimulation();
            break;
        case QUIT:
            quitSimulation();
            break;
        case CLEAR:
            toggleClear();
            break;
        case CYCLE_MAP:
            cycleMap();
            break;
        case FPGA_ON:
            toggleFPGARunning();
            break;
        default:
            break;
    }
}

////////////////////////////////////////////////////////////////////////

/////////////////////////// MOUSE VARIABLES /////////////////////////////

// variables for the mouse
mouse_event mouse_data = {0, 0, 0, 0, 0};
cursor_location vga_cursor_location = {320, 240, 0, 0};

/**
 * Process the mouse inputs
*/
void processMouse(){

    // refresh the mouse
    updateCursorLocation(&mouse_data, &vga_cursor_location);

    // check mouse clicks
    if (mouse_data.left && mouse_planet_enabled == 0){
        int operation = check_button_press(buttons, NUM_BUTTONS, vga_cursor_location.x, vga_cursor_location.y);
        if (operation != -1){
            runButtonFunction(operation);
        }
    }

    // controls for mouse planet - increase mass
    if (mouse_data.left && mouse_planet_enabled == 1){
        // increase the mass
        mouse_planet_mass += mouse_planet_mass/5;
        if (mouse_planet_mass > 83.72){
            mouse_planet_mass = 83.72;
        }

        // rescale the radius
        mouse_planet_radius = (int) (mouse_planet_mass/1.6744+5);
        printf("mouse_planet_radius: %d\n", mouse_planet_radius);

        printf("Mouse planet mass INCREASED, new mass: %f\n", mouse_planet_mass);
    }

    // controls for mouse planet - decrease mass
    if (mouse_data.middle && mouse_planet_enabled == 1){

        // decrease the mass
        mouse_planet_mass -= mouse_planet_mass/5;
        if (mouse_planet_mass < .0001){
            mouse_planet_mass = .1;
        }

        // rescale the radius
        mouse_planet_radius = (int) (mouse_planet_mass/1.6744+5);
        if (mouse_planet_radius < 1){
            mouse_planet_radius = 1;
        }

        printf("Mouse planet mass DECREASED, new mass: %f\n", mouse_planet_mass);
    }

    // if mouse click is right, toggle the fake mouse planet
    if (mouse_data.right){

        // toggle the mouse planet
        mouse_planet_enabled = mouse_planet_enabled == 0 ? 1 : 0;

        if (mouse_planet_enabled == 0){
            // need to erase the planet since it is no longer there
            VGA_star(celestial_bodies[NUM_BODIES].old_x_pixel, 
            celestial_bodies[NUM_BODIES].old_y_pixel, 
            black);
            printf("Mouse planet OFF\n");
        }
        else{
            printf("Mouse planet ON\n");
        }
    }

    // if the mouse planet is enabled, update it
    if (mouse_planet_enabled){
        // update the mouse planet
        updateMousePlanet(&celestial_bodies[NUM_BODIES], pixel_to_au, mouse_planet_mass, vga_cursor_location.x, vga_cursor_location.y, mouse_planet_radius);
    }
    
}

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

// variables for measuring time for debugging
struct timeval cycle_time;
struct timeval start_time;

// for delaying to hit 30 FPS always
struct timeval delay_time_start;
struct timeval delay_time_end;

// current time for a single frame to be calculated, in us
int frame_time = 0;

////////////////////////////////////////////////////////////////////////
///////////////////// VALUES RECIEVED FROM FPGA /////////////////////
////////////////////////////////////////////////////////////////////////

// x acceleration
#define X_ACCELERATION_PIO 0x00000000
volatile unsigned int *x_acceleration_ptr = NULL;

// y acceleration
#define Y_ACCELERATION_PIO 0x00000010
volatile unsigned int *y_acceleration_ptr = NULL;

// FPGA done - when the FPGA is done calculating the gravity
#define FPGA_DONE_PIO 0x00000020
volatile unsigned int *fpga_done_ptr = NULL;
// no value since gets written to directly

// when VGA is blanked
#define VGA_BLANK_PIO 0x000000b0
volatile unsigned int *vga_blank_ptr = NULL;

///////////////////////////////////////////////////////////////
///////////////////// VALUES SENT TO FPGA /////////////////////
///////////////////////////////////////////////////////////////

// reset
#define RESET_PIO 0x00000030
volatile unsigned int *reset_ptr = NULL;
// no value since gets written to directly

// object count
#define OBJECT_COUNT_PIO 0x00000040
volatile unsigned int *object_count_ptr = NULL;
// no value since gets written to directly

// index - write address for the celestial bodies
//         read address for the celestial bodies when recieving
#define INDEX_OUT_PIO 0x00000050
volatile unsigned int *index_out_ptr = NULL;
// no value since gets written to directly

// x - float
#define X_PIO 0x00000060
volatile unsigned int *x_loc_ptr = NULL;
// no value since gets written to directly

// y - float
#define Y_PIO 0x00000070
volatile unsigned int *y_loc_ptr = NULL;
// no value since gets written to directly

// mass - float
#define MASS_PIO 0x00000080
volatile unsigned int *mass_ptr = NULL;
// no value since gets written to directly

// write enable - true starts fill phase in FPGA
#define WRITE_ENABLE_PIO 0x00000090
volatile unsigned int *write_enable_ptr = NULL;
// no value since gets written to directly

// send enable - send index, FPGA sends the data back, read data, increment index
#define SEND_ENABLE_PIO 0x000000a0
volatile unsigned int *send_enable_ptr = NULL;
// no value since gets written to directly

///////////////////////////////////////////////////////////////
//////////////////// SETUP PIO POINTERS ///////////////////////
///////////////////////////////////////////////////////////////

/**
 * Setup the pointers to the PIO ports
*/
void FPGA_PIO_SETUP(){

    // the light weight bus base
    void *h2p_lw_virtual_base;

	// /dev/mem file id
	int fd;

    // === get FPGA addresses ==================
    // Open /dev/mem
	if( ( fd = open( "/dev/mem", ( O_RDWR | O_SYNC ) ) ) == -1 ) 	{
		printf( "ERROR: could not open \"/dev/mem\"...\n" );
		return;
	}
    
    // get virtual addr that maps to physical
	// for light weight bus
	h2p_lw_virtual_base = mmap( NULL, HW_REGS_SPAN, ( PROT_READ | PROT_WRITE ), MAP_SHARED, fd, HW_REGS_BASE );	
	if( h2p_lw_virtual_base == MAP_FAILED ) {
		printf( "ERROR: mmap1() failed...\n" );
		close( fd );
		return;
	}

    //assign the actual addresses to each pointer

    /////////////////////// VALUES RECIEVED FROM FPGA /////////////////////

    x_acceleration_ptr = (volatile unsigned int *)(h2p_lw_virtual_base + X_ACCELERATION_PIO);
    y_acceleration_ptr = (volatile unsigned int*)(h2p_lw_virtual_base + Y_ACCELERATION_PIO);
    fpga_done_ptr = (volatile unsigned int *)(h2p_lw_virtual_base + FPGA_DONE_PIO);

    // does not work, kept in in case want to use later/if is fixed
    vga_blank_ptr = (volatile unsigned int *)(h2p_lw_virtual_base + VGA_BLANK_PIO);

    /////////////////////// VALUES SENT TO FPGA /////////////////////

    reset_ptr = (volatile unsigned int *)(h2p_lw_virtual_base + RESET_PIO);
    object_count_ptr = (volatile unsigned int *)(h2p_lw_virtual_base + OBJECT_COUNT_PIO);
    index_out_ptr = (volatile unsigned int *)(h2p_lw_virtual_base + INDEX_OUT_PIO);
    x_loc_ptr = (volatile unsigned *)(h2p_lw_virtual_base + X_PIO);
    y_loc_ptr = (volatile unsigned *)(h2p_lw_virtual_base + Y_PIO);
    mass_ptr = (volatile unsigned *)(h2p_lw_virtual_base + MASS_PIO);
    write_enable_ptr = (volatile unsigned int *)(h2p_lw_virtual_base + WRITE_ENABLE_PIO);
    send_enable_ptr = (volatile unsigned int *)(h2p_lw_virtual_base + SEND_ENABLE_PIO);
}

///////////////////////////////////////////////////////////////


/**
 * Button function to reset the simulation
*/
void resetSimulation(){

    // reset the pixel to AU conversion default
    pixel_to_au = PIXEL_TO_AU_DEFAULT;
    time_step = TIME_STEP_DEFAULT;

    // clear screen
    VGA_clear_screen();
    VGA_text_clear();

    // if updated the size of num bodies
    int updatedSize = 0;

    // fine to use large with FPGA and HPS
    // for small, HPS can handle < 64 but FPGA cannot
    if (NUM_BODIES != map_num_bodies[current_map_index]){

        // Map changed, modfiy the FPGA On if needed
        // make sure only using valid sizes of FPGA
        NUM_BODIES = map_num_bodies[current_map_index];

        updatedSize = 1;

        if (NUM_BODIES > 64){
            fpga_on = 1;
            printf("FPGA will be used\n");
        }
        else{
            fpga_on = 0;
            printf("Minumum number of bodies for FPGA is 64, FPGA will not be used\n");
        }
    }

    if (updatedSize != 0){

        // reset the bodies
        NUM_BODIES = map_num_bodies[current_map_index];
        
        // make sure only using valid sizes of FPGA
        if (NUM_BODIES <= 64){
            fpga_on = 0;
            printf("Minumum number of bodies for FPGA is 64, FPGA will not be used\n");
        }
    }


    // import the bodies
    importCelestialBodies(map_file_names[current_map_index],
        celestial_bodies, NUM_BODIES, pixel_to_au, time_step);

    // reset the mouse planet
    mouse_planet_enabled = MOUSE_PLANET_ENABLED_DEFAULT;
    mouse_planet_mass = MOUSE_PLANET_MASS_DEFAULT;
    mouse_planet_radius = MOUSE_PLANET_RADIUS_DEFAULT;

    // redraw the screen
    redraw();

    // reset the FPGA
    if (fpga_on){
        // set the reset to 1
        *(reset_ptr) = 1;

        // delay for .1 seconds
        usleep(100000);

        // set the reset back to 0
        *(reset_ptr) = 0;
        
        // delay for .1 seconds
        usleep(100000);
    }

    printf("Simulation reset\n");
}


/**
 * Clear the screen and redraw the elements
*/
void redraw(){

    if (*(vga_blank_ptr) == 0 || overrideBlanking){
        // redraw celestial bodies
        redrawCelestialBodies(celestial_bodies, NUM_BODIES + mouse_planet_enabled, pixel_to_au, WIDTH, HEIGHT, erase_toggle);

        // draw the text
        redrawText(NUM_BODIES, mouse_planet_enabled, mouse_planet_mass, time_step, erase_toggle, frame_time);

        // draw the buttons
        redrawButtons(buttons, NUM_BUTTONS);
    }

    // draw the mouse - always doing this means will not get weird artifacts with the mouse
    redrawMouseCursor(&vga_cursor_location);

    // black border around the screen
    // if a button is drawn at the edge, it will get slightly cut off
    VGA_rect(640, 480, 0, 0, black);
}

/**
 * Initialize the simulation
*/
void initializeSimulation(){

    // initialize the mouse
    initalizeMouse();

    //initialize the VGA
    initalizeVGA();

    // initialize the pio ports
    FPGA_PIO_SETUP();

    // import the celestial bodies
    NUM_BODIES = map_num_bodies[current_map_index];

    importCelestialBodies(map_file_names[current_map_index],
    celestial_bodies, NUM_BODIES, PIXEL_TO_AU_DEFAULT, time_step);

    // prepare buttons
    prepareButtons(buttons, NUM_BUTTONS);

    // clear screen
    VGA_clear_screen();
    VGA_text_clear();

    // draw the display
    redraw();

    // reset the FPGA
    resetSimulation();
}

/**
 * Main function
*/
void main(){

    // use the linux signal handler to catch ctrl-c and exit gracefully
    signal(SIGINT, sigintHandler);

    initializeSimulation();

    printf("-------------------- Simulation started --------------------\n");

    //run the loop for the program
    while(1){

        cycleSimulation();

        processMouse();

        redraw();

        if (simulation_running){

            timeDebuggingEnd();

            timeDelaying();
        }
    }
}

/**
 * Simulation cycle
*/
void cycleSimulation(){

    if (simulation_running){
        if (TIMING_INFO){
            // get time of day in nanoseconds using linux time of day
            gettimeofday(&start_time, NULL);
        }

        if (DELAY_FRAME){
            gettimeofday(&delay_time_start, NULL);
        }

        // update the forces
        if (fpga_on){

            sendToFPGA();

            receiveFromFPGA();

            //do an update for the frame, mouse planet does not need this
            acceleration_timestep(celestial_bodies, NUM_BODIES, time_step);
        }
        else{
            hps_accel_update(celestial_bodies, NUM_BODIES + mouse_planet_enabled, DEBUG);

            //do an update for the frame, mouse planet does not need this
            acceleration_timestep(celestial_bodies, NUM_BODIES, time_step);
        }

        // redraw celestial bodies
        redrawCelestialBodies(celestial_bodies, NUM_BODIES + mouse_planet_enabled, pixel_to_au, WIDTH, HEIGHT, erase_toggle);
    }
}

/**
 * Show information about the time taken to do the calculations
*/
void timeDebuggingEnd(){
    if (TIMING_INFO){
        gettimeofday(&cycle_time, NULL);
        
        // calculate elapsed time
        double elapsed_us = (cycle_time.tv_sec - start_time.tv_sec) * 1e6 + (cycle_time.tv_usec - start_time.tv_usec);

        // print elapsed time
        printf("Elapsed time: %f us\n", elapsed_us);

        // used for tuning max number of bodies
        if (elapsed_us > 33366){
            printf("WARNING: Elapsed time is less than 10 ms, reducing number of bodies by 1\n");
            NUM_BODIES -= 1;
        }
        else{
            printf("Stable number of bodies: %d\n", NUM_BODIES);
        }

        // used to delay to get 30 FPS
        if (elapsed_us > 33366){
            printf("WARNING: Elapsed time is less than 1/30 ms\n");
        }
        else{
            printf("Delaying by %f us\n", 33366 - elapsed_us);
            usleep(33366 - elapsed_us);
        }
    }
}

/**
 * Handle the delay for 30 FPS
*/
void timeDelaying(){
    if (DELAY_FRAME){
        // calculate elapsed time and delay if needed
        gettimeofday(&delay_time_end, NULL);
        
        // calculate elapsed time
        double elapsed_us = (delay_time_end.tv_sec - delay_time_start.tv_sec) * 1e6 + (delay_time_end.tv_usec - delay_time_start.tv_usec);

        frame_time = elapsed_us;

        // used to delay to get 30 FPS
        if (elapsed_us < 33333){
            usleep(33333 - elapsed_us);
        }
    }
}

/**
 * Send celestial body data to the FPGA
 * - how many objects there are now - just before the write enable, once every cycle before data is sent
- write enable
    loop
    - index of object
    - Float Position X
    - Float Position y
    - Float Mass
*/
void sendToFPGA(){

    // send the number of objects
    *(object_count_ptr) = NUM_BODIES + mouse_planet_enabled;

    // send the write enable, starts fill phase
    *(write_enable_ptr) = 1;

    if (INPUT_INST_DEBUG){
        // print the data to be input to modelsim
        printf("object_count = 32'd%d;\n", NUM_BODIES + mouse_planet_enabled);
        printf("# 20\n");
        printf("write_enable = 1'd1;\n");
        printf("# 20\n");
    }

    int i;

    // send the data
    for (i = 0; i < NUM_BODIES + mouse_planet_enabled; i++){
        // send the index
        *(index_out_ptr) = i;

        // send the data
        *x_loc_ptr = floatToReg27(celestial_bodies[i].x);
        *y_loc_ptr = floatToReg27(celestial_bodies[i].y);
        *mass_ptr  = floatToReg27(celestial_bodies[i].m);

        if (INPUT_INST_DEBUG){
            // print the data to be input to modelsim
            printf("index = 32'd%d;\n", i);
            printf("# 20\n");
            printf("x_loc = %f;\n", celestial_bodies[i].x);
            printf("# 20\n");
            printf("y_loc = %f;\n", celestial_bodies[i].y);
            printf("# 20\n");
            printf("mass = %f;\n", celestial_bodies[i].m);
            printf("# 20\n");
        }
    }

    // send the write disable
    *(write_enable_ptr) = 0;

    if (INPUT_INST_DEBUG){
        // print the data to be input to modelsim
        printf("write_enable = 1'd0;\n");
        printf("# 20\n");
    }
}

/**
 * Receive celestial body data from the FPGA
 * - wait until index of object changes
 */
void receiveFromFPGA(){

    while (*(fpga_done_ptr) == 0){
        // do nothing, just wait
    }

    // great, now the FPGA is done, can say to send
    *(send_enable_ptr) = 1;

    int i;
    // no + mouse_planet_enabled because the mouse planet is controlled by the user
    for (i = 0; i < NUM_BODIES; i++){

        *(index_out_ptr) = i;
        
        // get the data
        celestial_bodies[i].x_accel = reg27ToFloat(*(x_acceleration_ptr));
        celestial_bodies[i].y_accel = reg27ToFloat(*(y_acceleration_ptr));
    }

    // done sending
    *(send_enable_ptr) = 0;
}


/**
 * Signal handler for ctrl-c
*/
void sigintHandler(int sig_num){

    // Reset handler to catch SIGINT
    printf("\n Exiting simulation \n");
    // f flush
    fflush(stdout);
    // reset the FPGA
    resetSimulation();
    exit(0);
}