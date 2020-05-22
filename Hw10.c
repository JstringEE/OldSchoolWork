/**
 * main.c
 */
//ECE 4550 Lab 10.2.2
// Purpose: To provide position control to an AC motor
// Author: Austin and Landan
#include "F2837xD_device.h"
#include "F2837xD_pievect.h"
extern void DelayUs(Uint16);
#include "math.h"
#define pi 3.14159
// Put here global variable and function declarations.
Uint32 count = 1;
Uint32 count2 = 0;
Uint32 count3 = 0;
float32 Vdc = 24.0;
float32 dutyA = 0.5;
float32 dutyB = 0.5;
float32 dutyC = 0.5;
float32 vA = 12;
float32 vB = 12;
float32 vC = 12;
float32 N = 4.0;
float32 Vm = 1.4;
float32 theta;
float32 theta_ref = 0;
float32 J = 4.8e-6;
float32 F = 5e-5;
float32 R = 1.2;
float32 flux = 11e-3;
float32 lambda_r = 125;
float32 lambda_e = 500;
float32 Vmax = 25.46;
float32 T = 2e-4;

// parameters
float32 K11;
float32 K12;
float32 K2;

float32 L1;
float32 L2;

float32 K;
float32 alpha;
float32 beta;

float32 alpha_v;
float32 phi_v;

// state-space stuff
float32 x1_curr = 0;
float32 x2_curr = 0;
float32 sigma_curr = 0;
float32 y;
float32 u_prev = 0;
float32 u_star = 0;
float32 u_curr = 0;
float32 r = 0;


Uint32 i = 0;

int32 qep;
float32 y_data[5000];
float32 u_data[5000];
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
    ClkCfgRegs.SYSCLKDIVSEL.all = 1;
    ClkCfgRegs.SYSPLLCTL1.bit.PLLCLKEN = 1;
    ClkCfgRegs.SYSCLKDIVSEL.all = 0;

    // EPWM1 A and B muxing
    GpioCtrlRegs.GPAGMUX1.bit.GPIO0 = 0b00;
    GpioCtrlRegs.GPAMUX1.bit.GPIO0 = 0b01;
    GpioCtrlRegs.GPAGMUX1.bit.GPIO1 = 0b00;
    GpioCtrlRegs.GPAMUX1.bit.GPIO1 = 0b01;

    // EPWM2 A and B muxing
    GpioCtrlRegs.GPAGMUX1.bit.GPIO2 = 0b00;
    GpioCtrlRegs.GPAMUX1.bit.GPIO2 = 0b01;
    GpioCtrlRegs.GPAGMUX1.bit.GPIO3 = 0b00;
    GpioCtrlRegs.GPAMUX1.bit.GPIO3 = 0b01;

    // EPWM3 A and B muxing
    GpioCtrlRegs.GPAGMUX1.bit.GPIO4 = 0b00;
    GpioCtrlRegs.GPAMUX1.bit.GPIO4 = 0b01;
    GpioCtrlRegs.GPAGMUX1.bit.GPIO5 = 0b00;
    GpioCtrlRegs.GPAMUX1.bit.GPIO5 = 0b01;

    ClkCfgRegs.PERCLKDIVSEL.bit.EPWMCLKDIV = 1; // Scale Factor of fclk,pwm 100MHz divide by 2
    CpuSysRegs.PCLKCR2.bit.EPWM1 = 0x1; // feeds clock to PWM modules to be used
    asm(" NOP"); asm(" NOP");
    CpuSysRegs.PCLKCR2.bit.EPWM2 = 0x1; // feeds clock to PWM modules to be used
    asm(" NOP"); asm(" NOP");
    CpuSysRegs.PCLKCR2.bit.EPWM3 = 0x1; // feeds clock to PWM modules to be used
    asm(" NOP"); asm(" NOP");

    EPwm1Regs.TBCTL.bit.CTRMODE = 0b10;// Shape of periodic signal
    EPwm1Regs.TBPRD = 0x7D0; // Sets range and Frequency
    EPwm1Regs.TBCTL.bit.HSPCLKDIV = 0;
    EPwm1Regs.TBCTL.bit.CLKDIV = 0b000;

    EPwm1Regs.AQCTLA.bit.CAU = 0b01;
    EPwm1Regs.AQCTLA.bit.CAD = 0b10;
    EPwm1Regs.AQCTLB.bit.CAU = 0b10;
    EPwm1Regs.AQCTLB.bit.CAD = 0b01;

    EPwm2Regs.TBCTL.bit.CTRMODE = 0b10;// Shape of periodic signal
    EPwm2Regs.TBPRD = 0x7D0; // Sets range and Frequency
    EPwm2Regs.TBCTL.bit.HSPCLKDIV = 0;
    EPwm2Regs.TBCTL.bit.CLKDIV = 0b000;

    EPwm2Regs.AQCTLA.bit.CAU = 0b01;
    EPwm2Regs.AQCTLA.bit.CAD = 0b10;
    EPwm2Regs.AQCTLB.bit.CAU = 0b10;
    EPwm2Regs.AQCTLB.bit.CAD = 0b01;

    EPwm3Regs.TBCTL.bit.CTRMODE = 0b10;// Shape of periodic signal
    EPwm3Regs.TBPRD = 0x7D0; // Sets range and Frequency
    EPwm3Regs.TBCTL.bit.HSPCLKDIV = 0;
    EPwm3Regs.TBCTL.bit.CLKDIV = 0b000;

    EPwm3Regs.AQCTLA.bit.CAU = 0b01;
    EPwm3Regs.AQCTLA.bit.CAD = 0b10;
    EPwm3Regs.AQCTLB.bit.CAU = 0b10;
    EPwm3Regs.AQCTLB.bit.CAD = 0b01;

    EPwm1Regs.CMPA.bit.CMPA = 0x7D0 * .5;// Duty Cycle 0<1
    EPwm2Regs.CMPA.bit.CMPA = 0x7D0 * .5;
    EPwm3Regs.CMPA.bit.CMPA = 0x7D0 * .5;

    GpioCtrlRegs.GPDGMUX2.bit.GPIO124 = 0b00; // GPIO for En_GATE
    GpioCtrlRegs.GPDMUX2.bit.GPIO124 = 0b00;
    GpioCtrlRegs.GPDDIR.bit.GPIO124 = 1;
    GpioDataRegs.GPDSET.bit.GPIO124 = 1;
    DelayUs(2000);

    dutyA = 13.2/Vdc;
    dutyB = 10.8/Vdc;
    dutyC = 12.0/Vdc;
    EPwm1Regs.CMPA.bit.CMPA = 0x7D0 * dutyA;// Duty Cycle 0<1
    EPwm2Regs.CMPA.bit.CMPA = 0x7D0 * dutyB;
    EPwm3Regs.CMPA.bit.CMPA = 0x7D0 * dutyC;

    for(i = 0; i < 20; i++){
        DelayUs(65535);
    }
    //EQep1Regs.QEPCTL.bit.SWI = 0x1;

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


    CpuTimer0Regs.PRD.all = 40000-1;
    CpuTimer0Regs.TCR.bit.TSS = 1;
    CpuTimer0Regs.TCR.bit.TRB = 1;
    CpuTimer0Regs.TCR.bit.TIE = 1;

    PieCtrlRegs.PIECTRL.bit.ENPIE = 1;
    PieVectTable.TIMER0_INT = &timerISR;
    PieCtrlRegs.PIEIER1.bit.INTx7 = 1;
    PieCtrlRegs.PIEACK.all = M_INT1;
    IER = M_INT1;
    EINT;



    K = flux * N;
    alpha = (K*K + F*R) / (J*R);
    beta = (K/(J*R));

    K11 = (1/beta) * 3 * lambda_r * lambda_r;
    K12 = (1/beta) * (3 * lambda_r - alpha);
    K2 = (1/beta) * lambda_r * lambda_r * lambda_r;

    L1 = (2 * lambda_e) - alpha;
    L2 = (lambda_e * lambda_e) - (2 * alpha * lambda_e) + (alpha * alpha);
    CpuTimer0Regs.TCR.bit.TSS = 0;

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
    count++;
    if (count == 2500) {
        count3++;
        if (r == 0) {
            r =  2*pi;
        } else {
            r = 0;
        }
        count = 0;
    }
    qep = (int32) EQep1Regs.QPOSCNT; // Read from this
    y = qep * (2.0 * pi / 4000.0);

    EPwm1Regs.CMPA.bit.CMPA = 0x7D0 * dutyA;// Duty Cycle 0<1
    EPwm2Regs.CMPA.bit.CMPA = 0x7D0 * dutyB;
    EPwm3Regs.CMPA.bit.CMPA = 0x7D0 * dutyC;

    u_star = (-K11 * x1_curr) - (K12 * x2_curr) - (K2 * sigma_curr);
    if (u_star > Vmax) {
        u_curr = Vmax;
    } else if (u_star < -Vmax) {
        u_curr = -Vmax;
    } else {
        u_curr = u_star;
    }

    x1_curr = x1_curr + (T * x2_curr) - (T * L1*(x1_curr - y));
    x2_curr = x2_curr - (T * alpha * x2_curr) + (T * beta * u_curr) - (T * L2 * (x1_curr - y));



    if (u_star > Vmax || u_star < -Vmax) {
        sigma_curr = sigma_curr;
    } else {
        sigma_curr = sigma_curr + T * (y - r);
    }



    if (abs(u_curr) <= Vmax){
        alpha_v = abs(u_curr);
    } else{
        alpha_v = Vmax;
    }

    if (u_curr >= 0){
            phi_v = pi/2;
        } else{
            phi_v = -pi/2;
        }
    vA = 12.0+0.4714*alpha_v*cos(N*y+phi_v-pi/6);
    vB = 12.0+0.4714*alpha_v*cos(N*y+phi_v-pi/6.0-2*pi/3.0);
    vC = 12.0+0.4714*alpha_v*cos(N*y+phi_v-pi/6.0+2*pi/3.0);

    dutyA = vA/Vdc;
    dutyB = vB/Vdc;
    dutyC = vC/Vdc;


    if (count2 < 5000) {
        y_data[count2] = y;
        u_data[count2] = u_curr;
        count++;
        count2++;
    }



    PieCtrlRegs.PIEACK.all = M_INT1;
    IER = M_INT1;
    EINT;
}


