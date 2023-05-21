// Modified from Professor Bruce Land's VGA course code
// https://people.ece.cornell.edu/land/courses/ece5760/DE1_SOC/HPS_peripherials/Examples_version_18.html
// GPU with FAST display from SRAM

#ifndef VGA_CODE_H
#define VGA_CODE_H

// video display
#define SDRAM_BASE            0xC0000000
#define SDRAM_END             0xC3FFFFFF
#define SDRAM_SPAN			  0x04000000
// characters
#define FPGA_CHAR_BASE        0xC9000000 
#define FPGA_CHAR_END         0xC9001FFF
#define FPGA_CHAR_SPAN        0x00002000
/* Cyclone V FPGA devices */
#define HW_REGS_BASE          0xff200000
//#define HW_REGS_SPAN        0x00200000 
#define HW_REGS_SPAN          0x00005000 

// 16-bit primary colors
#define red  (0+(0<<5)+(31<<11))
#define dark_red (0+(0<<5)+(15<<11))
#define green (0+(63<<5)+(0<<11))
#define dark_green (0+(31<<5)+(0<<11))
#define blue (31+(0<<5)+(0<<11))
#define dark_blue (15+(0<<5)+(0<<11))
#define yellow (0+(63<<5)+(31<<11))
#define cyan (31+(63<<5)+(0<<11))
#define magenta (31+(0<<5)+(31<<11))
#define black (0x0000)
#define gray (15+(31<<5)+(51<<11))
#define white (0xffff)

#define clear -1

// pixel macro
#define VGA_PIXEL(x,y,color) do{\
	int  *pixel_ptr ;\
	pixel_ptr = (int*)((char *)vga_pixel_ptr + (((y)*640+(x))<<1)) ; \
	*(short *)pixel_ptr = (color);\
} while(0)

// swap macro
#define SWAP(X,Y) do{int temp=X; X=Y; Y=temp;}while(0) 


// initialize the VGA
void initalizeVGA();

// graphics primitives
void VGA_text(int x, int y, char * text_ptr);
void VGA_text_clear();
void VGA_box(int x1, int y1, int x2, int y2, short pixel_color);
void VGA_rect(int x1, int y1, int x2, int y2, short pixel_color);
void VGA_line(int x1, int y1, int x2, int y2, short c) ;
void VGA_Vline(int x1, int y1, int y2, short pixel_color) ;
void VGA_Hline(int x1, int y1, int x2, short pixel_color);
void VGA_disc(int x, int y, int r, short pixel_color);
void VGA_circle (int x, int y, int r, int pixel_color);
void VGA_clear_screen();
void VGA_star(int x, int y, unsigned int pixel_color);
void VGA_super_star(int x, int y, unsigned int pixel_color);
void VGA_warped_world(int x, int y, unsigned int pixel_color);

#endif