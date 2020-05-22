//ECE 4550 Take Home Lab 1
// Jesse Austin Stringfellow
// Section L02, Due 15 February 2019

/**
 * main.c
 */


#include "F2802x_Device.h"
#include "F2802x_PieVect.h"
#include "math.h"
#define pi 3.14159

float32 y = 2;
Uint32 mode = 0;
Uint32 count = 1;
float32 x1[200];
float32 x2[200];
float32 x1_curr = 0;
float32 x2_curr = 0;
float32 x1_prev = 2;
float32 x2_prev = 0;
interrupt void timerISR(void);
interrupt void pushbutton(void);

void main(void)
{
    EALLOW;
    SysCtrlRegs.WDCR = 0b0000000001101000; // Disable Watchdog Timer
    SysCtrlRegs.PLLSTS.bit.DIVSEL = 0; // 00,01 divide by 4, 10 by 2, 11 by 1
    SysCtrlRegs.PLLSTS.bit.MCLKOFF = 1; // 0 enable oscillator fail-detect logic, 1 disable
    SysCtrlRegs.PLLCR.bit.DIV = 0b0100; // Multiply by 2
    while(SysCtrlRegs.PLLSTS.bit.PLLLOCKS != 1);
    SysCtrlRegs.PLLSTS.bit.MCLKOFF = 0; // Enable
    SysCtrlRegs.PLLSTS.bit.DIVSEL = 0b10;// Divide by 4
    PieVectTable.TINT0 = &timerISR;
    PieCtrlRegs.PIEIER1.bit.INTx7 = 1;
    PieCtrlRegs.PIEACK.all = M_INT1;


    GpioIntRegs.GPIOXINT1SEL.bit.GPIOSEL = 0b01100; // Setup external Interrrupt for Pushbutton
    XIntruptRegs.XINT1CR.bit.POLARITY = 01;
    XIntruptRegs.XINT1CR.bit.ENABLE = 1;

    CpuTimer0Regs.PRD.all = 0x4E1F; //19999 in Binary to give a frequency of 1Khz
    CpuTimer0Regs.TCR.bit.TSS = 0;
    CpuTimer0Regs.TCR.bit.TRB = 1;
    CpuTimer0Regs.TCR.bit.TIE = 1;
    PieCtrlRegs.PIECTRL.bit.ENPIE = 1;
    PieVectTable.XINT1 = &pushbutton;
    PieCtrlRegs.PIEIER1.bit.INTx4 = 1;
    PieCtrlRegs.PIEACK.all = M_INT1;
    IER = M_INT1;
    EINT;

    GpioCtrlRegs.GPADIR.bit.GPIO12 = 0; // Configure gpio12 as input
    GpioCtrlRegs.GPAPUD.bit.GPIO12 = 1; // 0 enable internal pullup, 1 disable
    GpioCtrlRegs.GPAQSEL1.bit.GPIO12 = 10;//01 3 samples, 10 6 samples, 11 async
    GpioCtrlRegs.GPACTRL.bit.QUALPRD1 =0x00;// 0x00 to 0xFF Specifies Sampling period

    GpioCtrlRegs.GPADIR.bit.GPIO0 = 1; //Configure GPIO0 as output LED 0
    GpioCtrlRegs.GPADIR.bit.GPIO1 = 1; //Configure GPIO1 as output LED 1
    GpioCtrlRegs.GPADIR.bit.GPIO2 = 1; //Configure GPIO2 as output LED 2
    GpioCtrlRegs.GPADIR.bit.GPIO3 = 1; //Configure GPIO3 as output LED 3
    GpioDataRegs.GPASET.bit.GPIO0 = 1; //LED Off
    GpioDataRegs.GPASET.bit.GPIO1 = 1; //LED Off
    GpioDataRegs.GPASET.bit.GPIO2 = 1; //LED Off
    GpioDataRegs.GPASET.bit.GPIO3 = 1; //LED Off
    SysCtrlRegs.WDCR = 0b0000000000101000; // Enable Watchdog Timer

    EDIS;
    while(1){
        EALLOW;
        SysCtrlRegs.WDKEY = 0x55; //Service the Watchdog timer
        SysCtrlRegs.WDKEY = 0xAA; //Service the Watchdog timer
        EDIS;
    }

    }
interrupt void timerISR(void)
{

    x1_curr = x1_prev + 0.001 * (x2_prev);
    x2_curr = x2_prev + 0.001 * (y * (1 - x1_prev * x1_prev) * x2_prev - x1_prev);

    if(x2_curr< -1.9){
        GpioDataRegs.GPACLEAR.bit.GPIO0 = 1; //LED 0 off
    }
    else{
        GpioDataRegs.GPASET.bit.GPIO0 = 1; //LED Off
    }
    if(x1_curr < -1.9){
        GpioDataRegs.GPACLEAR.bit.GPIO1 = 1; //LED1 on
    }
    else{
        GpioDataRegs.GPASET.bit.GPIO1 = 1; //LED Off
    }
    if(x2_curr > 1.9) {
        GpioDataRegs.GPACLEAR.bit.GPIO2 = 1; //LED2 on
    }
    else{
        GpioDataRegs.GPASET.bit.GPIO2 = 1; //LED Off
    }
    if(x1_curr> 1.9){
        GpioDataRegs.GPACLEAR.bit.GPIO3 = 1; //LED3 on
    }
    else{
        GpioDataRegs.GPASET.bit.GPIO3 = 1; //LED Off
    }
    x1_prev = x1_curr;
    x2_prev = x2_curr;

    PieCtrlRegs.PIEACK.all = M_INT1;
    IER = M_INT1;
    EINT;

}
interrupt void pushbutton(void){
    x1_prev = 2.0;
    x2_prev = 0.0;
    if (y==2){
        y =.02;
    }
    else if (y < .025){
        y = 2;
    }
    PieCtrlRegs.PIEACK.all = M_INT1;
    IER = M_INT1;
    EINT;

}
