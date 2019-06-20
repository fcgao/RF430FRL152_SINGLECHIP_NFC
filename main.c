#include "NDEF.h"
#include "types.h"
#include "patch.h"
#include "timer.h"

#define INTERVAL 60
#define MAXDATA 60
#define DATAWIDTH 3

#include <rf430frl152h.h>
/////////this code is for the thermistor change the pin with comment below/////////////////
/////////this code is merged to master after finishing////////////////////////////////////
//////////sends out three analog digits /////////////////////////////////////////////////

#include <rf430frl152h.h>

#pragma PERSISTENT (ndefcount)
unsigned char ndefcount = 0;

void DeviceInit(void);

unsigned char secondCTR = 0;
volatile int ADC_Volts = 0;

extern u08_t NFC_NDEF_Message[];

void main(){
    WDTCTL = WDTPW + WDTHOLD;                   // Stop watchdog
    DS = 1;                                     // ROM variable needs to be initialized here
    asm ( " CALL #0x5CDA ");                    // Call ROM function ( Initialize function pointers)
    asm ( " CALL #0x5CAC ");                    // Call ROM function ( Check part configuration)

    initISO15693(CLEAR_BLOCK_LOCKS);            // clear all block locks
    DeviceInit();

    initTimer();

    while(1)    {
        __bis_SR_register(LPM3_bits + GIE);
    }
}

void DeviceInit(void){
    P1SEL0 = 0xF0; //keep JTAG
    P1SEL1 = 0xF0; //keep JTAG
    //  P1SEL0 = 0x00; //no JTAG
    //  P1SEL1 = 0x00; //no JTAG
    /*Internal HFCLK = 4MHz      LFCLK = 256KHz*/
    CCSCTL0 = CCSKEY;                               // Unlock CCS
    CCSCTL1 = 0;                                    // 0b = CLKIN or X-OSC is directly used for clock generation
                                                    // 1b = CLKIN or X-OSC is divided by two for clock generation
    CCSCTL4 = SELA_1 + SELM_0 + SELS_0;             // Select CLK source => ACLK = LFCLK    MCLK = SMCLK = HFCLK
    CCSCTL5 = DIVA_2 + DIVM_1 + DIVS_1;             // Set the Dividers for
                                                    // ACLK= /2    MCLK = SMCLK = /1
    CCSCTL6 = XTOFF;                                // Turns off the crystal if it is not being used
    CCSCTL8 = ACLKREQEN + MCLKREQEN + SMCLKREQEN;   //disable clocks if they are not being used
    CCSCTL0_H |= 0xFF;                              // Lock CCS


    SD14CTL0 = SD14SSEL__ACLK + SD14DIV__64;         //Set the clk and divider for SD ADC
    SD14CTL0 = SD14EN + SD14IE + SD14SGL;

    //  SD14CTL1 = SD14UNI + SD14INCH_3 +  SD14RBEN0;    //ref resistor  pin 17
    /*  SD14FILT => enables 14 bit ADC (Moving Average filter selected over CIC filter)
    *  SD14UNI enables conversion only in +ve range. -ve voltage detection not possible*/
    // SD14CTL1 = SD14UNI + SD14INCH_3 +  SD14RBEN1 + SD14GAIN__1 + SD14FILT__AVG + SD14RATE__AVG16384; // thermistor   pin 17
    //  SD14CTL1 = SD14UNI + SD14INCH_2 +  SD14RBEN1 + SD14GAIN__1 + SD14FILT__CIC + SD14RATE__CIC128; // thermistor   pin 18

    //  SD14CTL1 = SD14UNI + SD14INCH_2 +  SD14RBEN1 + SD14GAIN__1 + SD14FILT__AVG + SD14RATE__AVG16384; // thermistor   pin 18/ADC2/TEMP2 //SDIGAIN_1
    //       SD14CTL1 = SD14UNI + SD14INCH_2 +  SD14RBEN1 + SD14GAIN__8 + SD14FILT__AVG + SD14RATE__AVG16384; // thermistor   pin 18/ADC2/TEMP2 //SDIGAIN_8
    SD14CTL1 = SD14UNI + SD14INCH_3 +  SD14RBEN0 + SD14GAIN__8 + SD14FILT__AVG + SD14RATE__AVG16384; // thermistor   pin 17/ADC1/TEMP1 //SDIGAIN_8
    //      SD14CTL1 = SD14UNI + SD14INCH_3 +  SD14RBEN0 + SD14GAIN__1 + SD14FILT__AVG + SD14RATE__AVG16384; // thermistor   pin 17/ADC1/TEMP1 //SDIGAIN_1
    //    SD14CTL1 = SD14UNI + SD14INCH_2 +  SD14RBEN1 + SD14GAIN__1 + SD14FILT__CIC + SD14RATE__CIC128; // thermistor   pin 18

}


#pragma vector = SD_ADC_VECTOR
__interrupt void SD_ADC_ISR(void)
{
    switch(__even_in_range(SD14IV,4))
    {
    case SD14IV__NONE: // no interrupt pending
    {
        break;}
    case SD14IV__RES: //ADC Data available
    {
        unsigned int ADC_Value = 0;
        volatile unsigned int i = 0;

        ADC_Value =  SD14MEM0; //Read the ADC Data      //sending the raw data to phone
//        ADC_Volts = ((ADC_Value >> 7) * 900)/(16383 >> 8);
        ADC_Volts = (ADC_Value* 900)/16384;

        SD14CTL0 &= ~SD14IFG;  //clear the data available interrupt

        ndefcount++;

        if(ndefcount > MAXDATA){
            ndefcount = MAXDATA;
            for( i = NDEFSTART;i<= NDEFSTART+ MAXDATA*DATAWIDTH- DATAWIDTH;i++){
                NFC_NDEF_Message[i] = NFC_NDEF_Message[i+DATAWIDTH];
            }
        }

        NFC_NDEF_Message[NDEFSTART-DATAWIDTH+DATAWIDTH*ndefcount] = ADC_Value;
        NFC_NDEF_Message[NDEFSTART-DATAWIDTH+DATAWIDTH*ndefcount+1] = ADC_Value>>8;
        NFC_NDEF_Message[NDEFSTART-DATAWIDTH+DATAWIDTH*ndefcount+2] = ',';

        NFC_NDEF_Message[NLENPOS] = NLEN + ndefcount*DATAWIDTH;
        NFC_NDEF_Message[PLENPOS] = PLEN + ndefcount*DATAWIDTH;

        __bic_SR_register_on_exit(LPM3_bits);
        break;}
    case SD14IV__OV: //Memory Overflow
    {
        break;}
    }

}

#pragma CODE_SECTION(RF13M_ISR, ".rf13m_rom_isr")       // comment this line for creating a custom RF13M ISR that will exist in FRAM, bypassing ROM's, uncomment previous
#pragma vector = RF13M_VECTOR
__interrupt void RF13M_ISR(void)
{
}

#pragma vector = TIMER0_A0_VECTOR
__interrupt void TimerA0_ISR(void)
{
    TA0CTL &= ~TAIFG;
    secondCTR++;
    NFC_NDEF_Message[NDEFSTART-1] = secondCTR;
    if(secondCTR == INTERVAL){
        secondCTR = 0;
        SD14CTL0 |= SD14SC;
    }
    __bic_SR_register(LPM3_bits);
}
