/**
 * main.c
 */
//ECE 4550 Lab 8.2 Obtain Measurements from Intertial Sensor
// Purpose:
// Author: Austin and Landan
#include "F2837xD_device.h"
#include "F2837xD_pievect.h"
//#include "math.h"
#define pi 3.14159
// Put here global variable and function declarations.
float32 t = 0;
Uint32 count = 1;
Uint32 count2 = 0;
float32 Vbar = 0;
float32 Vdc = 24.0;
Uint16 chip_id = 0x00FF;

int16 x1 = 0;
int16 x2 = 0;
int16 y1 = 0;
int16 y2 = 0;
int16 z1 = 0.0;
int16 z2 = 0.0;

float32 x = 0.0;
float32 y = 0.0;
float32 z = 0.0;

interrupt void timerISR(void);

void main(void)
{
    EALLOW;
    WdRegs.WDCR.all = 0b0000000010101000; // Disable watchdog timer
    ClkCfgRegs.CLKSRCCTL1.bit.OSCCLKSRCSEL = 01;
    ClkCfgRegs.SYSPLLCTL1.bit.PLLCLKEN = 0;
    ClkCfgRegs.SYSCLKDIVSEL.all = 0x0;
    ClkCfgRegs.SYSPLLMULT.all = 0x14;
    while(ClkCfgRegs.SYSPLLSTS.bit.LOCKS != 1);
    ClkCfgRegs.SYSCLKDIVSEL.all = 0;
    ClkCfgRegs.SYSPLLCTL1.bit.PLLCLKEN = 1;
    ClkCfgRegs.SYSCLKDIVSEL.all = 0;
    CpuTimer0Regs.PRD.all = 0x5F5E100; // Timer Interrupt
    CpuTimer0Regs.TCR.bit.TSS = 0;
    CpuTimer0Regs.TCR.bit.TRB = 1;
    CpuTimer0Regs.TCR.bit.TIE = 1;
    PieCtrlRegs.PIECTRL.bit.ENPIE = 1;
    PieVectTable.TIMER0_INT = &timerISR;
    PieCtrlRegs.PIEIER1.bit.INTx7 = 1;
    PieCtrlRegs.PIEACK.all = M_INT1;
    IER = M_INT1;
    EINT;

    GpioCtrlRegs.GPDGMUX1.bit.GPIO104 = 0b00; // SCL
    GpioCtrlRegs.GPDMUX1.bit.GPIO104 = 0b01;
    GpioCtrlRegs.GPDPUD.bit.GPIO104 = 0;

    GpioCtrlRegs.GPDGMUX1.bit.GPIO105 = 0b00; // SDA
    GpioCtrlRegs.GPDMUX1.bit.GPIO105 = 0b01;
    GpioCtrlRegs.GPDPUD.bit.GPIO105 = 0;

    CpuSysRegs.PCLKCR9.bit.I2C_A = 1; //
    asm(" NOP"); asm(" NOP");
    I2caRegs.I2CMDR.bit.IRS = 0;
    I2caRegs.I2CPSC.bit.IPSC = 0x13;
    I2caRegs.I2CCLKL = 0x5F;
    I2caRegs.I2CCLKH = 0x5F;
    I2caRegs.I2CMDR.bit.IRS = 1;

    WdRegs.WDCR.all = 0b0000000000101000;
    EDIS;

    while(I2caRegs.I2CMDR.bit.STP == 1);
    I2caRegs.I2CSAR.bit.SAR = 0x69; // Sensor Address
    I2caRegs.I2CCNT = 2; // Transmit one data byte
    I2caRegs.I2CDXR.bit.DATA = 0x7E; // CMD
    I2caRegs.I2CMDR.all = 0x2E20;
    //while(I2caRegs.I2CSTR.bit.ARDY == 0);

    while(I2caRegs.I2CSTR.bit.XRDY == 0);
    I2caRegs.I2CDXR.bit.DATA = 0x11;
    count2 = 100;

while(1) {
        EALLOW; // to allow access to protected registers

        WdRegs.WDKEY.all = 0x55; // to service the watchdog
        WdRegs.WDKEY.all = 0xAA; // to service the watchdog
        EDIS; // to disallow access to protected registers
    }
}
interrupt void timerISR(void){

    while(I2caRegs.I2CMDR.bit.STP == 1);
    I2caRegs.I2CSAR.bit.SAR = 0x69;
    I2caRegs.I2CCNT = 1; // Transmit one data byte
    I2caRegs.I2CDXR.bit.DATA = 0x12;
    I2caRegs.I2CMDR.all = 0x2620;
    while(I2caRegs.I2CSTR.bit.ARDY == 0);
    I2caRegs.I2CCNT = 6;
    I2caRegs.I2CMDR.all =  0x2C20;
    while(I2caRegs.I2CSTR.bit.RRDY == 0);
    x1 = I2caRegs.I2CDRR.bit.DATA;

    count++;

       while(I2caRegs.I2CSTR.bit.RRDY == 0);
       x2 = I2caRegs.I2CDRR.bit.DATA;

       while(I2caRegs.I2CSTR.bit.RRDY == 0);
       y1 = I2caRegs.I2CDRR.bit.DATA;

       while(I2caRegs.I2CSTR.bit.RRDY == 0);
       y2 = I2caRegs.I2CDRR.bit.DATA;

       while(I2caRegs.I2CSTR.bit.RRDY == 0);
       z1 = I2caRegs.I2CDRR.bit.DATA;

       while(I2caRegs.I2CSTR.bit.RRDY == 0);
       z2 = I2caRegs.I2CDRR.bit.DATA;
x1 = (int16)x1;
x2 = (int16)x2;
y1 = (int16)y1;
y2 = (int16)y2;
z1 = (int16)z1;
z2 = (int16)z2;

    x = (x2 << 8) | x1;
    y = (y2 << 8) | y1;
    z = (z2 << 8) | z1;

    x /= 16384.0;
    y /= 16384.0;
    z /= 16384.0;

    PieCtrlRegs.PIEACK.all = M_INT1;
    IER = M_INT1;
    EINT;
}
