/*
 * Pixel.c
 *
 *  Created on: -----
 *      Author: -----
 */
#define led *(uint8_t*) 0x41200000
#define inputs *(uint8_t*) 0xE000A068
#define ctrl_s *(uint8_t*) 0x41220008
#define chnl *(uint8_t*) 0x41220000
#define rgb *(uint8_t*) 0x41240000

#include "Pixel.h"
#include <stdint.h>
#include <unistd.h>




//Table for pixel dots.
//				 dots[X][Y][COLOR]
volatile uint8_t dots[8][8][3]={0};


// Here the setup operations for the LED matrix will be performed
void setup(){

    // channel to zero
    chnl = 0;

    // control signals to zero
    ctrl_s = 0;

	//reseting screen at start is a MUST to operation (Set RST-pin to 1).

    ctrl_s &= 0;
    usleep(500);
    ctrl_s |= 1;
    
    // set sda bit to 1
    ctrl_s |= 1 << 4;


	//Write code that sets 6-bit values in register of DM163 chip. Recommended that every bit in that register is set to 1. 6-bits and 24 "bytes", so some kind of loop structure could be nice.
	//24*6 bits needs to be transmitted
    // 8*3*6

    uint8_t i,a,b;
    // for leds
    for (i = 0; i < 8; ++i){
        // for colors
        for (a = 0; a < 3; ++a){
            // for bits in color
            for (b = 0; b < 6; ++b){
                    // change clock bit to zero
                    ctrl_s &= ~(1 << 3);
                    // change clock bit to one
                    ctrl_s |= 1 << 3;

                    // now it read sda bit
            }
        }
    }
    

	//Final thing in this function is to set SB-bit to 1 to enable transmission to 8-bit register.
    ctrl_s |= 1 << 2;

}

//Change value of one pixel at led matrix. This function is only used for changing values of dots array
void SetPixel(uint8_t x,uint8_t y, uint8_t r, uint8_t g, uint8_t b){

	//Hint: you can invert Y-axis quite easily with 7-y
	dots[x][y][0]=b;
	dots[x][y][1]=g;
	dots[x][y][2]=r;
	//Write rest of two lines of code required to make this function work properly (green and red colors to array).


}


//Put new data to led matrix. Hint: This function is supposed to send 24-bytes and parameter x is for channel x-coordinate.
void run(uint8_t x){
    // ensure that lat is zero
    ctrl_s &= ~(1 << 1);


	//Write code that writes data to led matrix driver (8-bit data). Use values from dots array
	//Hint: use nested loops (loops inside loops)
	//Hint2: loop iterations are 8,3,8 (pixels,color,8-bitdata)
    uint8_t y,c,b,color;
    // for pixel in column x
    for (y = 0; y < 8; ++y){
        // for color in pixel x,y
        for (c = 0; c < 3; ++c){
            // color bits
            color = dots[x][y][c];

            // for bit in color c
            for (b = 0; b < 8; ++b){
                // if msb in color is one - set DSA to 1 else to 0 
                if (color & 0x80)
                    ctrl_s |= 1 << 4;
                else
                    ctrl_s &= ~(1 << 4);
                // send it with SCK bit change
                ctrl_s &= ~(1 << 3);
                color <<= 1; // change msb bit in color to next msb
                ctrl_s |= 1 << 3;
            }
        }
    }
    // send register info
    latch();
    // make sure SCK is zero
    ctrl_s &= ~(1 << 3);


}

//Latch signal. See colorsshield.pdf or DM163.pdf in project folder on how latching works
void latch(){
    ctrl_s |= 1 << 1;
    ctrl_s &= ~(1 << 1);

}


//Set one line (channel) as active, one at a time.
void open_line(uint8_t x){
    chnl &= 0;
    switch (x) {
        case 0: chnl |= 1; break;
        case 1: chnl |= 1 << 1; break;
        case 2: chnl |= 1 << 2; break;
        case 3: chnl |= 1 << 3; break;
        case 4: chnl |= 1 << 4; break;
        case 5: chnl |= 1 << 5; break;
        case 6: chnl |= 1 << 6; break;
        case 7: chnl |= 1 << 7; break;
        default: chnl &= 0;
    }

}



