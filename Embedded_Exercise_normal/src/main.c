/******************************************************************************
*
* Copyright (C) 2009 - 2014 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/

/*
 *
 *
 * This application configures UART 16550 to baud rate 9600.
 * PS7 UART (Zynq) is not initialized by this application, since
 * bootrom/bsp configures it to baud rate 115200
 *
 * ------------------------------------------------
 * | UART TYPE   BAUD RATE                        |
 * ------------------------------------------------
 *   uartns550   9600
 *   uartlite    Configurable only in HW design
 *   ps7_uart    115200 (configured by bootrom/bsp)
 */

// Main program for exercise

//****************************************************
//By default, every output used in this exercise is 0
//****************************************************
#include <stdio.h>
#include "platform.h"
#include "xil_printf.h"
#include "sleep.h"
#include "xgpiops.h"
#include "xttcps.h"
#include "xscugic.h"
#include "xparameters.h"
#include "Pixel.h"
#include "Interrupt_setup.h"

//********************************************************************
//***************TRY TO READ COMMENTS*********************************
//********************************************************************

//***Hint: Use sleep(x)  or usleep(x) if you want some delays.****
//***To call assembler code found in blinker.S, call it using: blinker();***


//Comment this if you want to disable all interrupts
#define enable_interrupts




/***************************************************************************************
Name: Rasmus Ähtävä
Student number:

Name: Joona Vihervaara
Student number:

Name:
Student number:

Tick boxes that you have coded

Led-matrix driver		Game		    Assembler
	[x]					[x]					[]

Brief description: 
        - Game is won if you hit alien 3 times and lost if you miss 10 times.
        - BTN0 moves the ship right
        - BTN1 moves the ship left
        - BTN2 shoots the lazer bullet
        - BTN3 resets the game

        - Hit score can be seen by ship's yellow tiles
        - Miss score can be seen from right side of the matrix in violet



*****************************************************************************************/

/* ------- Some global variables - all initialized at init_game() ------------ */

// cur_channel is knows which channel should be shown currently
uint8_t cur_channel;
// ship's pos
uint8_t ship_x,ship_y;
// alien's pos and direction it's heading: 1 means right, 0 means left
uint8_t alien_x,alien_dir;
// lazer bullet's pos and lazer_on keeps track if lazer on the board
uint8_t lazer_x,lazer_y,lazer_on;
// hit_score,miss_score keeps track of game points
uint8_t hit_score,miss_score;
// game_end tells TickHandler1 if it should show the end screen
uint8_t game_end = 0;
// game_won tells the end screen if you won or lost the game
uint8_t game_won = 0;




/* check_game_status - check if lazer has hit the alien */
/* This is called from move_lazer - function */
void check_game_status()
{
    // if game has ended
    if (hit_score == 3){
        game_end = 1;
        game_won = 1;
    } else if (miss_score == 8){
        game_end = 1;
    }

}



/* ----------------- DRAWING AND CLEARING PIXELS ---------------------------- */

/* draw_ship - updates dots array of the ships position */
/* Called from draw_board */
void draw_ship()
{
    SetPixel(ship_x,ship_y,255,0,0);

    // if at least one point change right pixel to yellow
    if (hit_score > 0) SetPixel(ship_x-1,ship_y,255,255,0);
    else SetPixel(ship_x-1,ship_y,255,0,0);

    // if at least two points change left pixel to yellow
    if (hit_score > 1) SetPixel(ship_x+1,ship_y,255,255,0);
    else SetPixel(ship_x+1,ship_y,255,0,0);

    // if goal points change top to yellow, it's not showing really tho
    if (hit_score > 2) SetPixel(ship_x,ship_y+1,255,255,0);
    else SetPixel(ship_x,ship_y+1,255,0,0);
}

/* clear_ship - clears the ship from the dots array */
void clear_ship()
{
    SetPixel(ship_x,ship_y,0,0,0);
    SetPixel(ship_x,ship_y+1,0,0,0);
    SetPixel(ship_x-1,ship_y,0,0,0);
    SetPixel(ship_x+1,ship_y,0,0,0);
}

/* drew_lazer - adds lazer to dots array */
void draw_lazer(){SetPixel(lazer_x,lazer_y,0,0,255);}

/* clear_lazer - clears lazer from dots array */
void clear_lazer(){SetPixel(lazer_x,lazer_y,0,0,0);}

/* draw_alien - adds alien to dots array */
void draw_alien(){SetPixel(alien_x,0,0,255,0);}

/* clear_alien - clears alien from dots array */
void clear_alien(){SetPixel(alien_x,0,0,0,0);}

/* draw_miss_score - draws rightside's miss score */
void draw_miss_score()
{
    // draw current score to the right side column
    int i,y = 7;
    for (i = 0; i < miss_score; ++i){
        // violet
        SetPixel(7,y,238,130,238);
        --y;
    }
}

/* clear_miss_score - clears rightside's miss score */
void clear_miss_score()
{
    int i;
    for (i = 0; i < 7; ++i) SetPixel(7,i,0,0,0);
}


/* draw_board - draws the board */
void draw_board()
{
    draw_miss_score();
    draw_ship();
    draw_alien();
    if (lazer_on)
        draw_lazer();
}

/* clear_board - clears the whole board */
int clear_board()
{
    clear_miss_score();
    clear_ship();
    clear_alien();
    clear_lazer();
    return 0;
}

/* show_end - this is called if TickHandler1 notices game_end==1*/
void show_end(uint8_t game_won)
{
    clear_board();
    if (game_won){
        // eyes
        SetPixel(2, 3,255,0,0);
        SetPixel(5, 3,255,0,0);
        // mouth
        SetPixel(1, 5,255,0,0);
        SetPixel(2, 6,255,0,0);
        SetPixel(3, 6,255,0,0);
        SetPixel(4, 6,255,0,0);
        SetPixel(5, 6,255,0,0);
        SetPixel(6, 5,255,0,0);
    } else {
        // eyes
        SetPixel(2, 3,255,0,0);
        SetPixel(5, 3,255,0,0);
        // mouth
        SetPixel(1, 7,255,0,0);
        SetPixel(2, 6,255,0,0);
        SetPixel(3, 6,255,0,0);
        SetPixel(4, 6,255,0,0);
        SetPixel(5, 6,255,0,0);
        SetPixel(6, 7,255,0,0);
    }

}
/* clear_end_screen - this is called if BTN3 is pressed */
void clear_end_screen()
{
    // eyes
    SetPixel(2, 3,0,0,0);
    SetPixel(5, 3,0,0,0);
    // mouth - clears both smiley and not smiley
    SetPixel(1, 7,0,0,0);
    SetPixel(1, 5,0,0,0);
    SetPixel(2, 6,0,0,0);
    SetPixel(3, 6,0,0,0);
    SetPixel(4, 6,0,0,0);
    SetPixel(5, 6,0,0,0);
    SetPixel(6, 5,0,0,0);
    SetPixel(6, 7,0,0,0);

}
/* ------------------- CHANGING PIXELS -------------------------- */

/* move_lazer - clears board, moves lazer, checks if we won, draws board */
void move_lazer()
{
    clear_board();
    if (!lazer_on)
        return;
    // move lazer one up
    --lazer_y;
    if (lazer_y == 0){
        lazer_on = 0;
        // hitting increases the hit_score and missing the miss_score
        if (lazer_x == alien_x)
            ++hit_score;
        else
            ++miss_score;
        check_game_status();
    }
    draw_board();
}

/* move_alien - clears board, moves alien, draws board */
void move_alien()
{
    clear_board();
    // clear old alien
    // if alien going to right
    if (alien_dir == 1){
        // if on the right - change dir to left
        if (alien_x == 6){
            alien_dir = 0;
            --alien_x;
        } else {
            ++alien_x;
        }
    } else { // going to the left
        // if on the left - change dir to right
        if (alien_x == 0){
            alien_dir = 1;
            ++alien_x;
        } else {
            --alien_x;
        }
    }
    draw_board();
}

/* move_ship - clears board, moves ship to given dir if can, draws board */
void move_ship(uint8_t direction)
{
    clear_board();
    // move left if 0, right if 1
    if (direction == 0){
        // if already at most left
        if (ship_x-1 == 0)
            return;
        // clear old ship position
        --ship_x;
    } else {
        // if already at most right
        if (ship_x+1 == 6)
            return;
        // clear old ship position
        ++ship_x;
    }
    draw_board();
}

/* ----------------------- INITIALIZING VARIABLES -------------- */
/* init_game - clears board, inits global variables, draws board */
void init_game()
{
    clear_board();
    // TickHandler changes this
    cur_channel = 0;
    ship_x = 3;
    ship_y = 6;
    alien_x = 3;
    alien_dir = 1;
    hit_score = 0;
    miss_score = 0;
    // lazer is not on screen
    lazer_on = 0;
    game_end = 0;
    game_won = 0;
    draw_board();
}




int main()
{
	//**DO NOT REMOVE THIS****
	    init_platform();
	//************************


#ifdef	enable_interrupts
	    init_interrupts();
#endif


	    //setup screen
	    setup();

        init_game();
   
	    Xil_ExceptionEnable();



	    //Try to avoid writing any code in the main loop.
		while(1){


		}


		cleanup_platform();
		return 0;
}


//Timer interrupt handler for led matrix update. Frequency is 800 Hz
void TickHandler(void *CallBackRef){
	//Don't remove this
	uint32_t StatusEvent;

	//exceptions must be disabled when updating screen
	Xil_ExceptionDisable();



	//****Write code here ****

    // make sure no channels are open with calling open_line
    // with too big channel so it goes to default which closes all:


    open_line(10);

    // run cur_channel bits to driver
    run(cur_channel);
    // open cur_channel to show them
    open_line(cur_channel);

    // change cur_channel to next one
    if (cur_channel == 7)
        cur_channel = 0;
    else
        ++cur_channel;




	//****END OF OWN CODE*****************

	//*********clear timer interrupt status. DO NOT REMOVE********
	StatusEvent = XTtcPs_GetInterruptStatus((XTtcPs *)CallBackRef);
	XTtcPs_ClearInterruptStatus((XTtcPs *)CallBackRef, StatusEvent);
	//*************************************************************
	//enable exceptions
	Xil_ExceptionEnable();
}


//Timer interrupt for moving alien, shooting... Frequency is 10 Hz by default
void TickHandler1(void *CallBackRef){

	//Don't remove this
	uint32_t StatusEvent;

	//****Write code here ****

    if (game_end){
        // clearing it just in case
        clear_end_screen();
        show_end(game_won);
    }


    // moves the alien back and forth
    move_alien();
    
    // moves the lazer if it's been shot
    if (lazer_on){
        move_lazer();
    }



	//****END OF OWN CODE*****************
	//clear timer interrupt status. DO NOT REMOVE
	StatusEvent = XTtcPs_GetInterruptStatus((XTtcPs *)CallBackRef);
	XTtcPs_ClearInterruptStatus((XTtcPs *)CallBackRef, StatusEvent);

}


//Interrupt handler for switches and buttons.
//Reading Status will tell which button or switch was used
//Bank information is useless in this exercise
void ButtonHandler(void *CallBackRef, u32 Bank, u32 Status){
	//****Write code here ****

	//Hint: Status==0x01 ->btn0, Status==0x02->btn1, Status==0x04->btn2, Status==0x08-> btn3, Status==0x10->SW0, Status==0x20 -> SW1

	//If true, btn0 was used to trigger interrupt
	if (Status==0x01){
        // BTN0
        // move ship right 
        move_ship(1);

	} else if (Status == 0x02){
        // BTN1
        // move ship left 
        move_ship(0);

    } else if (Status == 0x04){
        // BTN2
        // if lazer on the screen we have to wait for it to pass
        if (lazer_on)
            return;
        clear_board();
        // shoot lazer
        lazer_on = 1;
        // put lazer above ship
        lazer_x = ship_x;
        lazer_y = 4;
        draw_board();

    } else if (Status == 0x08){
        // BTN3
        // start new game
        clear_end_screen();
        init_game();

    } else if (Status == 0x10){
        // SW0 change. inputs is used to read the position of the slider 

    } else if (Status == 0x20){
        // SW1 change. inputs is used to read the position of the slider

    }








	//****END OF OWN CODE*****************
}


