/**
 * main.c
 */
// ECE 4550 Lab 3.2.2
// Purpose: Display Constant Frequency Counting
// Author: Austin and Landan
#include "F2837xD_device.h"
#include "F2837xD_pievect.h"
#define pi 3.14159
// Put here global variable and function declarations.
Uint32 count = 0;
interrupt void timerISR(void);

void main(void)
{
    EALLOW;
    GpioCtrlRegs.GPADIR.bit.GPIO31 = 1; // configures blue LED as output
    GpioCtrlRegs.GPBDIR.bit.GPIO34 = 1; // configures red LED as ouput
    WdRegs.WDCR.all = 0b0000000010101000; // Disable watchdog timer
    GpioCtrlRegs.GPADIR.bit.GPIO6 = 1;
    ClkCfgRegs.CLKSRCCTL1.bit.OSCCLKSRCSEL = 01;
    ClkCfgRegs.SYSPLLCTL1.bit.PLLCLKEN = 0;
    ClkCfgRegs.SYSCLKDIVSEL.all = 0x0;
    ClkCfgRegs.SYSPLLMULT.all = 0x14;
    while(ClkCfgRegs.SYSPLLSTS.bit.LOCKS != 1);
    ClkCfgRegs.SYSCLKDIVSEL.all = 0x4;
    ClkCfgRegs.SYSPLLCTL1.bit.PLLCLKEN = 1;
    ClkCfgRegs.SYSCLKDIVSEL.all = 0x1;
    CpuTimer0Regs.PRD.all = 0x2FAF080;
    CpuTimer0Regs.TCR.bit.TSS = 0;
    CpuTimer0Regs.TCR.bit.TRB = 1;
    CpuTimer0Regs.TCR.bit.TIE = 1;
    PieCtrlRegs.PIECTRL.bit.ENPIE = 1;
    PieVectTable.TIMER0_INT = &timerISR;
    PieCtrlRegs.PIEIER1.bit.INTx7 = 1;
    PieCtrlRegs.PIEACK.all = M_INT1;
    IER = M_INT1;
    EINT;
    WdRegs.WDCR.all = 0b0000000000101000;
    EDIS;

    while(1) {
        EALLOW; // to allow access to protected registers
        WdRegs.WDKEY.all = 0x55; // to service the watchdog
        WdRegs.WDKEY.all = 0xAA; // to service the watchdog
        EDIS; // to disallow access to protected registers
    }
}
 interrupt void timerISR(void)
 {
    if (count == 0) {
        GpioDataRegs.GPASET.bit.GPIO31 = 1;
        GpioDataRegs.GPBSET.bit.GPIO34 = 1;
        count++;
    }
    else if (count == 1) {
        GpioDataRegs.GPACLEAR.bit.GPIO31 = 1;
        GpioDataRegs.GPBSET.bit.GPIO34 = 1;
        count++;
    }
    else if (count == 2) {
        GpioDataRegs.GPASET.bit.GPIO31 = 1;
        GpioDataRegs.GPBCLEAR.bit.GPIO34 = 1;
        count++;
    }
    else if (count == 3) {
        GpioDataRegs.GPACLEAR.bit.GPIO31 = 1;
        GpioDataRegs.GPBCLEAR.bit.GPIO34 = 1;
        count = 0;
    }
    PieCtrlRegs.PIEACK.all = M_INT1;
    IER = M_INT1;
    EINT;

 }
