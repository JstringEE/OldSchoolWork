// Jesse Austin Stringfellow
// ECE 4550 L02 Work at Home Lab 2
/**
 * main.c
 */
#include "F2802x_Device.h"
#include "F2802x_PieVect.h"
#include "math.h"
#define pi 3.14159

Uint32 count = 0;
Uint32 baud = 9600;
Uint32 BRR;
Uint32 sysclk = 60e6;
Uint32 lspclk;
char recC = '0';
interrupt void sciRec(void);
int main(void)
{
    EALLOW;
    SysCtrlRegs.SCSR = 0x0001; // allow disabling watchdog timer
    SysCtrlRegs.WDCR = 0x0068; // disable watchdog timer
    SysCtrlRegs.PLLSTS.bit.DIVSEL = 0; // required before PLL locks
    SysCtrlRegs.PLLSTS.bit.MCLKOFF = 1;
    SysCtrlRegs.PLLCR.bit.DIV = 6; // CPU clock needs to be at least 50 MHz
    while(SysCtrlRegs.PLLSTS.bit.PLLLOCKS != 1); // wait for PLL to stabilize
    SysCtrlRegs.PLLSTS.bit.MCLKOFF = 0; // allow for oscillator errors
    SysCtrlRegs.PLLSTS.bit.DIVSEL = 3; // divide by 1


    GpioCtrlRegs.GPAMUX2.bit.GPIO28 = 01; // Set SCI-A transmit output
    GpioCtrlRegs.GPAMUX2.bit.GPIO29 = 01; // Set SCI-A recieve input

    SysCtrlRegs.PCLKCR0.bit.SCIAENCLK = 1; // Enables SCI clock
    asm(" NOP"); asm(" NOP");
    SciaRegs.SCICTL1.bit.SWRESET = 0;

    // Baud Calculations
    lspclk = sysclk/4;
    BRR = (lspclk/(baud*8)-1);
    // SCI Setup
    SciaRegs.SCIHBAUD = 0x0;
    SciaRegs.SCILBAUD = BRR;
    SciaRegs.SCICCR.bit.SCICHAR = 0x7; // SCI Character length
    SciaRegs.SCICTL1.bit.SWRESET = 1;
    SciaRegs.SCICTL1.bit.TXENA = 1; // Turn on Transmit
    SciaRegs.SCICTL1.bit.RXENA = 1; // Turn on Receive
    SciaRegs.SCICTL2.bit.RXBKINTENA = 1;

    //Setup PIE Interrupt
    PieCtrlRegs.PIECTRL.bit.ENPIE = 1;
    PieVectTable.SCIRXINTA = &sciRec;
    PieCtrlRegs.PIEIER9.bit.INTx1 = 1;
    PieCtrlRegs.PIEACK.all = M_INT9;
    IER = M_INT9;
    EINT;

    // Set LEDs to outputs
    GpioCtrlRegs.GPADIR.bit.GPIO0 = 1;
    GpioCtrlRegs.GPADIR.bit.GPIO1 = 1;
    GpioCtrlRegs.GPADIR.bit.GPIO2 = 1;
    GpioCtrlRegs.GPADIR.bit.GPIO3 = 1;

    // Turn LEDs off
    GpioDataRegs.GPASET.bit.GPIO0 = 1;
    GpioDataRegs.GPASET.bit.GPIO1 = 1;
    GpioDataRegs.GPASET.bit.GPIO2 = 1;
    GpioDataRegs.GPASET.bit.GPIO3 = 1;


    SysCtrlRegs.WDCR = 0x0028; // enable the watchdog timer
    EDIS;
    while(1) {
        EALLOW;
        // Service Watchdog Timer
        SysCtrlRegs.WDKEY = 0x55;
        SysCtrlRegs.WDKEY = 0xAA;
        EDIS;
    }
}
interrupt void sciRec(void)
{
    recC = SciaRegs.SCIRXBUF.bit.RXDT;
    count++;
    if (recC == '0') {
            // LED 0 on
            GpioDataRegs.GPACLEAR.bit.GPIO0 = 1;
        } else {
            // LED 0 off
            GpioDataRegs.GPASET.bit.GPIO0 = 1;
        }
    if (recC == '1') {
                // LED 1 on
                GpioDataRegs.GPACLEAR.bit.GPIO1 = 1;
        } else {
                // LED 1 off
                GpioDataRegs.GPASET.bit.GPIO1 = 1;
        }
    if (recC == '2') {
                // LED 2 on
                GpioDataRegs.GPACLEAR.bit.GPIO2 = 1;
        } else {
                // LED 2 off
                GpioDataRegs.GPASET.bit.GPIO2 = 1;
        }
    if (recC == '3') {
                // LED 3 on
                GpioDataRegs.GPACLEAR.bit.GPIO3 = 1;
        } else {
                // LED 3 off
                GpioDataRegs.GPASET.bit.GPIO3 = 1;
        }
    SciaRegs.SCITXBUF = recC;
    PieCtrlRegs.PIEACK.all = M_INT9;
    IER = M_INT9;
    EINT;
}
