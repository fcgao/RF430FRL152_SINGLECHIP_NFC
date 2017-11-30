#include "NDEF.h"
#include "types.h"
#include "patch.h"
#include "timer.h"

#define INTERVAL 5
#define MAXDATA 3
#define DATAWIDTH 24

#include <rf430frl152h.h>

#pragma PERSISTENT (ndefcount)
u08_t ndefcount = 0;


void DeviceInit(void);

unsigned char secondCTR = 0;
//volatile int ADC_Volts = 0;

inline u08_t giveones(u08_t num){
	return num%10 + 48;
}

inline u08_t givetens(u08_t num){
	return num/10 + 48;
}

extern u08_t NFC_NDEF_Message[];


//#pragma RETAIN(UID_4);
//#pragma location = 0x1A0C
//const u08_t UID_4;


unsigned char sec=0,min=59,hr=24,mon=12,yr=17,day = 30;


void main(){
	WDTCTL = WDTPW + WDTHOLD;                   // Stop watchdog
	DS = 1; 									// ROM variable needs to be initialized here
	asm ( " CALL #0x5CDA "); 					// Call ROM function ( Initialize function pointers)
	asm ( " CALL #0x5CAC "); 					// Call ROM function ( Check part configuration)

	initISO15693(CLEAR_BLOCK_LOCKS); 			// clear all block locks
	DeviceInit();

	//initTimer();

	while(1)	{
		__bis_SR_register(LPM3_bits + GIE);
	}
}

void DeviceInit(void){
	P1SEL0 = 0xF0; //keep JTAG
	P1SEL1 = 0xF0; //keep JTAG
	//	P1SEL0 = 0x00; //no JTAG
	//	P1SEL1 = 0x00; //no JTAG

	CCSCTL0 = CCSKEY;                        // Unlock CCS
	CCSCTL1 = 0;                             // do not half the clock speed
	CCSCTL4 = SELA_1 + SELM_0 + SELS_0;      // Select VLO for ACLK and select HFCLK/DCO for MCLK, and SMCLK
	CCSCTL5 = DIVA_2 + DIVM_1 + DIVS_1;      // Set the Dividers for ACLK (4), MCLK, and SMCLK to 1
	CCSCTL6 = XTOFF;                         // Turns of the crystal if it is not being used
	CCSCTL8 = ACLKREQEN + MCLKREQEN + SMCLKREQEN; //disable clocks if they are not being used

	CCSCTL0_H |= 0xFF;                       // Lock CCS

	SD14CTL0 = SD14EN + SD14IE + SD14SGL + VIRTGND;
	//	SD14CTL1 = SD14UNI + SD14INCH_3 +  SD14RBEN0;	//ref resistor	pin 17
	//	SD14CTL1 = SD14UNI + SD14INCH_2 +  SD14RBEN1;	// thermistor	pin 18
	SD14CTL1 = SD14UNI + SD14INCH_1;// + SD14RBEN0 + SD14RBEN1;	//internal temp
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
		u08_t i = 0;

		SD14CTL0 &= ~SD14IFG;  //clear the data available interrupt
		ADC_Value =  SD14MEM0; //Read the ADC Data		//sending the raw data to phone
		//ADC_Volts = ((ADC_Value >> 7) * 900)/(16383 >> 8);
		ADC_Value = ADC_Value/36 - 168;
		ndefcount++;

		if(ndefcount > MAXDATA){
			ndefcount = MAXDATA;
			for( i = NDEFSTART;i<= NDEFSTART+ MAXDATA*DATAWIDTH- DATAWIDTH;i++){
				NFC_NDEF_Message[i] = NFC_NDEF_Message[i+DATAWIDTH];
			}
		}

		NFC_NDEF_Message[NDEFSTART-DATAWIDTH+DATAWIDTH*ndefcount] =' ';
		NFC_NDEF_Message[NDEFSTART-DATAWIDTH+DATAWIDTH*ndefcount+1] = givetens(ADC_Value);
		NFC_NDEF_Message[NDEFSTART-DATAWIDTH+DATAWIDTH*ndefcount+2] = giveones(ADC_Value);
		NFC_NDEF_Message[NDEFSTART-DATAWIDTH+DATAWIDTH*ndefcount+3] = '.';
		NFC_NDEF_Message[NDEFSTART-DATAWIDTH+DATAWIDTH*ndefcount+4] = '0';
		NFC_NDEF_Message[NDEFSTART-DATAWIDTH+DATAWIDTH*ndefcount+5] = '0';
		NFC_NDEF_Message[NDEFSTART-DATAWIDTH+DATAWIDTH*ndefcount+6] = ',';
		NFC_NDEF_Message[NDEFSTART-DATAWIDTH+DATAWIDTH*ndefcount+7] = givetens(hr);
		NFC_NDEF_Message[NDEFSTART-DATAWIDTH+DATAWIDTH*ndefcount+8] = giveones(hr);
		NFC_NDEF_Message[NDEFSTART-DATAWIDTH+DATAWIDTH*ndefcount+9] = ':';
		NFC_NDEF_Message[NDEFSTART-DATAWIDTH+DATAWIDTH*ndefcount+10] = givetens(min);
		NFC_NDEF_Message[NDEFSTART-DATAWIDTH+DATAWIDTH*ndefcount+11] = giveones(min);
		NFC_NDEF_Message[NDEFSTART-DATAWIDTH+DATAWIDTH*ndefcount+12] = ',';
		NFC_NDEF_Message[NDEFSTART-DATAWIDTH+DATAWIDTH*ndefcount+13] = '2';
		NFC_NDEF_Message[NDEFSTART-DATAWIDTH+DATAWIDTH*ndefcount+14] = '0';
		NFC_NDEF_Message[NDEFSTART-DATAWIDTH+DATAWIDTH*ndefcount+15] = givetens(yr);
		NFC_NDEF_Message[NDEFSTART-DATAWIDTH+DATAWIDTH*ndefcount+16] = giveones(yr);
		NFC_NDEF_Message[NDEFSTART-DATAWIDTH+DATAWIDTH*ndefcount+17] = '/';
		NFC_NDEF_Message[NDEFSTART-DATAWIDTH+DATAWIDTH*ndefcount+18] = givetens(mon);
		NFC_NDEF_Message[NDEFSTART-DATAWIDTH+DATAWIDTH*ndefcount+19] = giveones(mon);
		NFC_NDEF_Message[NDEFSTART-DATAWIDTH+DATAWIDTH*ndefcount+20] = '/';
		NFC_NDEF_Message[NDEFSTART-DATAWIDTH+DATAWIDTH*ndefcount+21] = givetens(day);
		NFC_NDEF_Message[NDEFSTART-DATAWIDTH+DATAWIDTH*ndefcount+22] = giveones(day);
		NFC_NDEF_Message[NDEFSTART-DATAWIDTH+DATAWIDTH*ndefcount+23] = 0x0d;


		NFC_NDEF_Message[5] = NLEN + ndefcount*DATAWIDTH;
		NFC_NDEF_Message[8] = PLEN + ndefcount*DATAWIDTH;

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

#pragma vector = TIMER0_A0_VECTOR
__interrupt void TimerA0_ISR(void)
{
	TA0CTL &= ~TAIFG;
	secondCTR++;
	sec++;
	if(sec > 59){
		sec = 0;
		min++;
		if(min > 59){
			min = 0;
			hr++;
			if(hr > 24){
				hr = 0;
				day++;
				if(day > 30){
					day = 1;
					mon++;
					if(mon>12){
						mon = 1;
						yr++;
					}
				}
			}
		}

	}
	if(secondCTR == INTERVAL){
		secondCTR = 0;
		SD14CTL0 |= SD14SC;
	}
	__bic_SR_register(LPM3_bits);
}
