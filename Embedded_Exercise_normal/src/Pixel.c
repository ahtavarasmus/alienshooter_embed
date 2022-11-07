/*
 * Pixel.c
 *
 *  Created on: -----
 *      Author: -----
 */

#include "Pixel.h"



//Table for pixel dots.
//				 dots[X][Y][COLOR]
volatile uint8_t dots[8][8][3]={0};


// Here the setup operations for the LED matrix will be performed
void setup(){

    uint8_t *channel;
    channel = 0x41220000;
    *channel = 0;

    uint8_t *ctrl;
    ctrl = 0x41220008;
    *ctrl = 0;


	//reseting screen at start is a MUST to operation (Set RST-pin to 1).

    *ctrl |= 1;
    usleep(500);
    *ctrl &= 0;
    usleep(500);
    *ctrl |= 1;
    
    // set sda bit to 1
    //*ctrl 1000
    *ctrl |= 16;




	//Write code that sets 6-bit values in register of DM163 chip. Recommended that every bit in that register is set to 1. 6-bits and 24 "bytes", so some kind of loop structure could be nice.
	//24*6 bits needs to be transmitted
    // 8*3*6

    int i,color,bit;
    for (row = 0; row < 8; ++row){
        for (color = 0; color < 3; ++color){
            for (bit = 0; bit < 6; ++bit){
                /* set only bit3 to zero (CLK bit)*/
                *ctrl &=~0x10;
                // transfer 1-bit to register
                *ctrl|=0x10;
                /* set only bit3 back to one (CLK bit)*/
            }
        }
    }

	//Final thing in this function is to set SB-bit to 1 to enable transmission to 8-bit register.
    *ctrl |= 4;
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



	//Write code that writes data to led matrix driver (8-bit data). Use values from dots array
	//Hint: use nested loops (loops inside loops)
	//Hint2: loop iterations are 8,3,8 (pixels,color,8-bitdata)


}

//Latch signal. See colorsshield.pdf or DM163.pdf in project folder on how latching works
void latch(){
    uint8_t *ctrl = 0x41220008;
    *ctrl |= 2;
    *ctrl &= 0;

}


//Set one line (channel) as active, one at a time.
void open_line(uint8_t x){
    uint8_t *channel = 0x41220000;
    *channel &= 0;
    Switch (x) {
        case 0: *channel |= 1; break;
        case 1: *channel |= 2; break;
        case 2: *channel |= 4; break;
        case 3: *channel |= 16; break;
        case 4: *channel |= 32; break;
        case 5: *channel |= 64; break;
        case 6: *channel |= 128; break;
        case 7: *channel |= 256; break;
        default: *channel &= 0;
    }

}



