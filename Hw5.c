/**
 * main.c
 */
//ECE 4550 Lab 5.2
// Purpose: Control a motor with PWM and measure the output with QEP modules.
// Author: Austin and Landan
#include "F2837xD_device.h"
#include "F2837xD_pievect.h"
//#include "math.h"
#define pi 3.14159
// Put here global variable and function declarations.
float32 t = 0;
Uint32 count = 0;
Uint32 count2 = 0;
float32 Vbar = 0;
float32 Vdc = 24.0;
float32 duty = 0;
float32 Vdesired[2000];
int32 qep;
float32 theta[2000];
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
    CpuTimer0Regs.PRD.all = 0x30D40;
    CpuTimer0Regs.TCR.bit.TSS = 0;
    CpuTimer0Regs.TCR.bit.TRB = 1;
    CpuTimer0Regs.TCR.bit.TIE = 1;
    PieCtrlRegs.PIECTRL.bit.ENPIE = 1;
    PieVectTable.TIMER0_INT = &timerISR;
    PieCtrlRegs.PIEIER1.bit.INTx7 = 1;
    PieCtrlRegs.PIEACK.all = M_INT1;
    IER = M_INT1;
    EINT;

    GpioCtrlRegs.GPDGMUX2.bit.GPIO124 = 0b00; // GPIO for En_GATE
    GpioCtrlRegs.GPDMUX2.bit.GPIO124 = 0b00;
    GpioCtrlRegs.GPDDIR.bit.GPIO124 = 1;
    GpioDataRegs.GPDSET.bit.GPIO124 = 1;
    int i;
    for (i = 0; i < 10000; i++) {}


    GpioCtrlRegs.GPAGMUX1.bit.GPIO0 = 0b00;
    GpioCtrlRegs.GPAMUX1.bit.GPIO0 = 0b01;
    GpioCtrlRegs.GPAGMUX1.bit.GPIO1 = 0b00;
    GpioCtrlRegs.GPAMUX1.bit.GPIO1 = 0b01;
    GpioCtrlRegs.GPAGMUX1.bit.GPIO2 = 0b00;
    GpioCtrlRegs.GPAMUX1.bit.GPIO2 = 0b01;
    GpioCtrlRegs.GPAGMUX1.bit.GPIO3 = 0b00;
    GpioCtrlRegs.GPAMUX1.bit.GPIO3 = 0b01;

    ClkCfgRegs.PERCLKDIVSEL.bit.EPWMCLKDIV = 0b10; // Scale Factor of fclk,pwm 100MHz divide by 2
    CpuSysRegs.PCLKCR2.bit.EPWM1 = 0x1; // feeds clock to PWM modules to be used
    asm(" NOP"); asm(" NOP");
    CpuSysRegs.PCLKCR2.bit.EPWM2 = 0x1; // feeds clock to PWM modules to be used
    asm(" NOP"); asm(" NOP");

    EPwm1Regs.TBCTL.bit.CTRMODE = 0b10;// Shape of periodic signal
    EPwm1Regs.TBPRD = 0x7D0; // Sets range and Frequency
    EPwm1Regs.TBCTL.bit.HSPCLKDIV = 0b001;
    EPwm1Regs.TBCTL.bit.CLKDIV = 0b000;

    EPwm1Regs.AQCTLA.bit.CAU = 0b01;
    EPwm1Regs.AQCTLA.bit.CAD = 0b10;
    EPwm1Regs.AQCTLB.bit.CAU = 0b10;
    EPwm1Regs.AQCTLB.bit.CAD = 0b01;

    EPwm2Regs.TBCTL.bit.CTRMODE = 0b10;// Shape of periodic signal
    EPwm2Regs.TBPRD = 0x7D0; // Sets range and Frequency
    EPwm2Regs.TBCTL.bit.HSPCLKDIV = 0b001;
    EPwm2Regs.TBCTL.bit.CLKDIV = 0b000;

    EPwm2Regs.AQCTLA.bit.CAU = 0b10;
    EPwm2Regs.AQCTLA.bit.CAD = 0b01;
    EPwm2Regs.AQCTLB.bit.CAU = 0b01;
    EPwm2Regs.AQCTLB.bit.CAD = 0b10;

    CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 0b1;

    GpioCtrlRegs.GPAGMUX2.bit.GPIO20 = 0b00; // EQEP1A
    GpioCtrlRegs.GPAMUX2.bit.GPIO20 = 0b01;
    GpioCtrlRegs.GPAGMUX2.bit.GPIO21 = 0b00; // EQEP1B
    GpioCtrlRegs.GPAMUX2.bit.GPIO21 = 0b01;
    CpuSysRegs.PCLKCR4.bit.EQEP1 = 0x1; // QEP
    asm(" NOP"); asm(" NOP");
    EQep1Regs.QPOSMAX = 0xFFFFFFFF;
    EQep1Regs.QPOSINIT = 0x0;
    EQep1Regs.QEPCTL.bit.QPEN = 0x1;
    EQep1Regs.QEPCTL.bit.SWI = 0x1;

    WdRegs.WDCR.all = 0b0000000000101000;
    EDIS;

while(1) {
        EALLOW; // to allow access to protected registers

        WdRegs.WDKEY.all = 0x55; // to service the watchdog
        WdRegs.WDKEY.all = 0xAA; // to service the watchdog
        EDIS; // to disallow access to protected registers
    }
}
interrupt void timerISR(void){
    if (count >= 0 && count <= 200) {
        Vbar = 20;
        Vdesired[count2] = 20.0;
        count++;
    } else {
        if ((count > 200 && count <= 400) || (count > 600 && count <= 800)) {
            Vbar = 0;
            Vdesired[count2] = 0;
            count++;
        } else {
            if (count > 400 && count <= 600) {
                Vbar = -20;
                Vdesired[count2] = -20.0;
                count++;
            }
            else {
//                if (count > 800 && count <= 1000) {
//                    Vbar = 0;
//                    Vdesired[count2] = 0;
//                    count++;
//                } else {
                    count = 0;
                }
            }
        }



    duty = 0.5 * (1+ (Vbar/Vdc));
    EPwm1Regs.CMPA.bit.CMPA = 0x7D0 * duty;// Duty Cycle 0<1
    EPwm2Regs.CMPA.bit.CMPA = 0x7D0 * duty;
    qep = (int32) EQep1Regs.QPOSCNT; // Read from this
    theta[count2] = qep * (2.0*pi/1000.0);
    if (count2 < 2000) {
        count2++;
    }
    PieCtrlRegs.PIEACK.all = M_INT1;
    IER = M_INT1;
    EINT;
}

