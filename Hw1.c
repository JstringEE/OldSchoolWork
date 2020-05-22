
/**
 * main.c
 */
// ECE 4550 Lab 1
// Purpose: To Set the Watchdog Timer and Log data
// Author: Austin and Landon
#include "F2837xD_device.h"
#include "math.h"
#define pi 3.14159
// Put here global variable and function declarations.
Uint32 slow = 0;
Uint32 fast = 0;

float32 x[5000];
float32 y[5000];

int main(void)
{
    EALLOW; // to allow access to protected registers
    WdRegs.WDCR.all = 0b0000000010101000;
    //WdRegs.WDCR.all = 0b0000000010000000; // to disable the watchdog
    // Put here the code that should run only once.
    WdRegs.WDCR.all = 0b0000000000101000;
    //WdRegs.WDCR.all = 0b0000000000000000; // to enable the watchdog
    EDIS; // to disallow access to protected registers
    while(1) {
        fast += 1;
        if ((fast % 100000) == 0) {
            slow += 1;
        }

        if (fast < 5000){
            x[fast] = fast/5000;
            y[fast] = cos(2 * pi * fast/5000);
        }
        EALLOW; // to allow access to protected registers
        WdRegs.WDKEY.all = 0x55; // to service the watchdog
        WdRegs.WDKEY.all = 0xAA; // to service the watchdog
        EDIS; // to disallow access to protected registers
        // Put here the code that should run repeatedly.
    }
    return 0;
}

