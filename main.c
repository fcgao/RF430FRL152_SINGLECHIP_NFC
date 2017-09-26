#include "NDEF.h"
#include "types.h"
#include "patch.h"
#include "timer.h"

#define INTERVAL 10

#include <rf430frl152h.h>


#pragma PERSISTENT (ndefcount)
unsigned char ndefcount = 0;

void DeviceInit(void);

unsigned int ADC_Value = 0;
unsigned char secondCTR = 0;
//volatile int ADC_Volts = 0;

extern u08_t NFC_NDEF_Message[];

void main(){
	WDTCTL = WDTPW + WDTHOLD;                   // Stop watchdog
	DS = 1; 									// ROM variable needs to be initialized here
	asm ( " CALL #0x5CDA "); 					// Call ROM function ( Initialize function pointers)
	asm ( " CALL #0x5CAC "); 					// Call ROM function ( Check part configuration)

	initISO15693(CLEAR_BLOCK_LOCKS); 			// clear all block locks
	DeviceInit();

	initTimer();

	while(1)	{
		__bis_SR_register(LPM3_bits + GIE);
	}
}

void DeviceInit(void){
		P1SEL0 = 0xF0; //keep JTAG
		P1SEL1 = 0xF0; //keep JTAG
//	P1SEL0 = 0x00; //no JTAG
//	P1SEL1 = 0x00; //no JTAG

//	P1DIR |= 0x10;
//	P1OUT &= ~0x10;

	CCSCTL0 = CCSKEY;                        // Unlock CCS
	CCSCTL1 = 0;                             // do not half the clock speed
	CCSCTL4 = SELA_1 + SELM_0 + SELS_0;      // Select VLO for ACLK and select HFCLK/DCO for MCLK, and SMCLK
	CCSCTL5 = DIVA_2 + DIVM_1 + DIVS_1;      // Set the Dividers for ACLK (4), MCLK, and SMCLK to 1
	CCSCTL6 = XTOFF;                         // Turns of the crystal if it is not being used
	CCSCTL8 = ACLKREQEN + MCLKREQEN + SMCLKREQEN; //disable clocks if they are not being used

	CCSCTL0_H |= 0xFF;                       // Lock CCS

	//setting up adc
	P1DIR &= ~0xEF;
	P1REN = 0;
	SD14CTL0 = SD14EN + VIRTGND + SD14IE + SD14SGL;
	SD14CTL1 = SD14UNI + SD14INCH_2 + SD14RBEN0 + SD14RBEN1;
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
		SD14CTL0 &= ~SD14IFG;  //clear the data available interrupt
		ADC_Value =  SD14MEM0; //Read the ADC Data		//sending the raw data to phone
//		ADC_Volts = ((ADC_Value >> 7) * 900)/(16383 >> 8);

		NFC_NDEF_Message[14+2*ndefcount+1] = ADC_Value;
		NFC_NDEF_Message[14+2*ndefcount] = ADC_Value>>8;
		NFC_NDEF_Message[5] = 0x0c + ndefcount*2;
		NFC_NDEF_Message[8] = 0x08 + ndefcount*2;

		ndefcount++;
//		P1OUT ^= 0x10;
//		__delay_cycles(400);
//		P1OUT &= ~0x10;
		__bic_SR_register_on_exit(LPM3_bits);
		break;}
	case SD14IV__OV: //Memory Overflow
	{
		break;}
	}

}

#pragma CODE_SECTION(RF13M_ISR, ".rf13m_rom_isr") 		// comment this line for creating a custom RF13M ISR that will exist in FRAM, bypassing ROM's, uncomment previous
#pragma vector = RF13M_VECTOR
__interrupt void RF13M_ISR(void)
{
}

#pragma vector = TIMER0_A1_VECTOR
__interrupt void TimerA1_ISR(void)
{
	TA0CTL &= ~TAIFG;
	secondCTR++;
	if(secondCTR == INTERVAL){
//		P1OUT ^= 0x10;
//		__delay_cycles(400);
//		P1OUT &= ~0x10;
//		__delay_cycles(400000);
		secondCTR = 0;
		SD14CTL0 |= SD14SC;
	}
	__bic_SR_register(LPM3_bits);
}
