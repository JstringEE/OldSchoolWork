/**
 * main.c
 */
//ECE 4550 Lab 6.2
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
float32 duty = 0;

// state space stuff
float32 x1_curr;
float32 x1_prev = 0;
float32 x2_curr;
float32 x2_prev = 0;
float32 sigma_curr;
float32 sigma_prev = 0;
float32 u_curr;
float32 u_prev = 0;
float32 y_curr;
float32 y_prev = 0;
float32 r;


float32  lambda_r = 50.0;
float32  lambda_e = 200.0;
float32  alpha = 96.2076;
float32 beta = 549.3;

float32  L1;
float32  L2;

float32  K11;
float32  K12;
float32  K2;
int wind_up = 0;


float32 Umax = 24.0;
float32 Umin = -24.0;

float32 dt = 0.001; // 1 ms time difference
float32 u[4000];
int32 qep;
float32 y[4000];
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

    EPwm1Regs.CMPA.bit.CMPA = 0x7D0 * .5;// Duty Cycle 0<1
    EPwm2Regs.CMPA.bit.CMPA = 0x7D0 * .5;

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
    r = 10 * pi;
    L1 = (2 * (lambda_e)) - alpha;
    L2 = lambda_e * lambda_e - 2 * alpha * lambda_e + alpha*alpha;

    K11 = (1/beta)*3*lambda_r*lambda_r;
    K12 = (1/beta)*(3*lambda_r - alpha);
    K2 = (1/beta) * lambda_r * lambda_r * lambda_r;

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
    if (r == 0 && count2 == 1000) {
        r = 10 * pi;
        count2 = 0;
    } else if (r != 0 && count2 == 1000) {
        r = 0;
        count2 = 0;
    }

    x1_curr = x1_prev + (dt * x2_prev) - (dt * L1 * (x1_prev - y_prev));
    x2_curr = x2_prev - (dt * alpha * x2_prev) + (dt * beta * u_prev) - (dt * L2 * (x1_prev - y_prev));

    u_curr = (-K11 * x1_prev) - (K12 * x2_prev) - (K2 * sigma_prev);

    if (u_curr > Umax) {
        sigma_curr = sigma_prev;
        u_curr = Umax;
    } else if (u_curr < Umin) {
        sigma_curr = sigma_prev;
        u_curr = Umin;
    } else {
        sigma_curr = sigma_prev + dt * (y_prev - r);
    }

    if (wind_up == 0) {
        sigma_curr = sigma_prev + dt*(y_prev - r);
    }

    x1_prev = x1_curr;
    x2_prev = x2_curr;
    sigma_prev = sigma_curr;
    u_prev = u_curr;

    duty = 0.5 * (1+ (u_curr/Vdc));

    EPwm1Regs.CMPA.bit.CMPA = 0x7D0 * duty;// Duty Cycle 0<1
    EPwm2Regs.CMPA.bit.CMPA = 0x7D0 * duty;

    qep = (int32) EQep1Regs.QPOSCNT; // Read from this
    y_prev = qep * (2.0 * pi / 1000.0);

    if (count < 4000) {
        u[count] = u_curr;
        y[count] = y_prev;
        count++;

    }
    count2++;

    PieCtrlRegs.PIEACK.all = M_INT1;
    IER = M_INT1;
    EINT;
}

