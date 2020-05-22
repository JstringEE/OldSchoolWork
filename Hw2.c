
/**
 * main.c
 */
// ECE 4550 Lab 2 4.2.2
// Purpose: To toggle an LED on with flash memory
// Author: Austin and Landan
#include "F2837xD_device.h"
#define pi 3.14159
// Put here global variable and function declarations.
Uint32 fast = 0;

// Initialize flash (provided by lab manual
#pragma CODE_SECTION(InitFlash, "RamFuncs")
void InitFlash(void)
{
Flash0CtrlRegs.FPAC1.bit.PMPPWR = 0x1;
Flash0CtrlRegs.FBFALLBACK.bit.BNKPWR0 = 0x3;
Flash0CtrlRegs.FRD_INTF_CTRL.bit.DATA_CACHE_EN = 0;
Flash0CtrlRegs.FRD_INTF_CTRL.bit.PREFETCH_EN = 0;
Flash0CtrlRegs.FRDCNTL.bit.RWAIT = 0x3;
Flash0CtrlRegs.FRD_INTF_CTRL.bit.DATA_CACHE_EN = 1;
Flash0CtrlRegs.FRD_INTF_CTRL.bit.PREFETCH_EN = 1;
Flash0EccRegs.ECC_ENABLE.bit.ENABLE = 0xA;
asm(" RPT #6 || NOP");
}
extern Uint16 RamFuncs_loadstart;
extern Uint16 RamFuncs_loadsize;
extern Uint16 RamFuncs_runstart;

int main(void)
{
    EALLOW; // to allow access to protected registers
    WdRegs.WDCR.all = 0b0000000010101000; // Disable watchdog timer
    memcpy(&RamFuncs_runstart, &RamFuncs_loadstart, (Uint32) &RamFuncs_loadsize); // copy from flash to RAM
    InitFlash(); // run function from lab
    // Change to 10 MHz
    ClkCfgRegs.SYSPLLMULT.all = 0;
    ClkCfgRegs.SYSCLKDIVSEL.all = 0;
    GpioCtrlRegs.GPADIR.bit.GPIO31 = 1; // configures blue LED as output
    GpioDataRegs.GPASET.bit.GPIO31 = 1; // turn off blue LED
    GpioCtrlRegs.GPBDIR.bit.GPIO34 = 1; // configures red LED as ouput
    WdRegs.WDCR.all = 0b0000000000101000; // enable watchdog
    EDIS; // to disallow access to protected registers
    while(1) {
        fast++;
        EALLOW; // to allow access to protected registers
        if ((fast % 100000) == 0) {
            GpioDataRegs.GPBTOGGLE.bit.GPIO34 = 1; // toggle red LED
        }
        WdRegs.WDKEY.all = 0x55; // to service the watchdog
        WdRegs.WDKEY.all = 0xAA; // to service the watchdog
        EDIS; // to disallow access to protected registers
        // Put here the code that should run repeatedly.
    }
    return 0;
}
