

/**
 * main.c
 */
//ECE 4550 Lab 4.2.2
// Purpose: Input analog voltage, filter, and output analog voltage.
// Author: Austin and Landan
#include "F2837xD_device.h"
#include "F2837xD_pievect.h"
#include "math.h"
#define pi 3.14159

// Put here global variable and function declarations.
float32 t = 0;
float32 t2 = 0;
Uint32 count = 0;
Uint32 test[400];
float32 Vmea[400];
float32 Vcmd[400];
float32 vi[3] = {0,0,0};
float32 vo[2] = {0,0};

float32 alpha = pi/12.5;
float32 beta = .97;
interrupt void command(void);


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
    CpuTimer0Regs.PRD.all = 0x7CF;
    CpuTimer0Regs.TCR.bit.TSS = 0;
    CpuTimer0Regs.TCR.bit.TRB = 1;
    CpuTimer0Regs.TCR.bit.TIE = 1;

    CpuSysRegs.PCLKCR16.bit.DAC_A = 1;
    asm(" NOP"); asm(" NOP");
    DacaRegs.DACCTL.bit.DACREFSEL = 1;
    DacaRegs.DACOUTEN.bit.DACOUTEN = 1;


    CpuSysRegs.PCLKCR16.bit.DAC_B = 1;
    asm(" NOP"); asm(" NOP");
    DacbRegs.DACCTL.bit.DACREFSEL = 1;
    DacbRegs.DACOUTEN.bit.DACOUTEN = 1;

    CpuSysRegs.PCLKCR13.bit.ADC_A = 1;
    asm(" NOP"); asm(" NOP");
    AdcaRegs.ADCCTL2.bit.PRESCALE = 0b0110;
    AdcaRegs.ADCCTL1.bit.ADCPWDNZ = 1;
    Uint32 i;
    for (i = 0; i < 800,000; i++) {}
    AdcaRegs.ADCSOC4CTL.bit.TRIGSEL = 0x01;
    AdcaRegs.ADCSOC4CTL.bit.CHSEL = 0x4;
    AdcaRegs.ADCSOC4CTL.bit.ACQPS = 0x27;

    AdcaRegs.ADCSOC2CTL.bit.TRIGSEL = 0x01;
    AdcaRegs.ADCSOC2CTL.bit.CHSEL = 0x2;
    AdcaRegs.ADCSOC2CTL.bit.ACQPS = 0x27;

    AdcaRegs.ADCCTL1.bit.INTPULSEPOS = 1;
    AdcaRegs.ADCINTSEL3N4.bit.INT4SEL = 0x4;
    AdcaRegs.ADCINTSEL3N4.bit.INT4E = 1;

    AdcaRegs.ADCINTSEL1N2.bit.INT2SEL = 0x4;
    AdcaRegs.ADCINTSEL1N2.bit.INT2E = 1;
    Vcmd[count] = 2.1;

    PieCtrlRegs.PIECTRL.bit.ENPIE = 1;
    PieVectTable.ADCA4_INT = &command;
    PieCtrlRegs.PIEIER10.bit.INTx4 = 1;
    PieVectTable.ADCA2_INT = &command;
    PieCtrlRegs.PIEIER10.bit.INTx2 = 1;


    IER = M_INT10;
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
interrupt void command(void){
    if (count < 400) {
        vi[2] = vi[1];
        vi[1] = vi[0];
        DacaRegs.DACVALS.all = 4096*Vcmd[count]/3;
        vi[0] = AdcaResultRegs.ADCRESULT2*3.0/4096;
        float32 a1 = -2*beta*cos(alpha);
        float32 a2 = beta*beta;
        float32 b1 = -2*cos(alpha);
        float32 b2 = 1.0;
        float32 vo_1 = vo[0];
        vo[0] = vi[0] + b1*vi[1] + b2*vi[2] - a1*vo[0] - a2*vo[1];
        vo[1] = vo_1;
        DacbRegs.DACVALS.all = 4096 * vo[0] / 3;
        Vmea[count] = AdcaResultRegs.ADCRESULT4*3.0/4096;
        count++;
        t += 0.00001;
        Vcmd[count] = 1.5 + 0.5*cos(2*pi*1000*t) + 0.1*cos(2*pi*4000*t);
    }

    AdcaRegs.ADCINTFLGCLR.bit.ADCINT4 = 1;
    AdcaRegs.ADCINTFLGCLR.bit.ADCINT2 = 1;
    PieCtrlRegs.PIEACK.all = M_INT10;
    IER = M_INT10;
    EINT;
}

