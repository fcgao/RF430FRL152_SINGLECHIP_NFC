/*
 * timer.c
 *
 *  Created on: Sep 11, 2017
 *      Author: kiran
 */
#include <rf430frl152h.h>
#include "timer.h"

void initTimer(){
	TA0CTL |= TASSEL_1 + TAIE;	//aux clock and interrup enabled
	TA0CTL &= ~TAIFG;			//clear interrupt
	TA0CTL |= MC_1;				//count up to ta0ccr0
	TA0EX0 = TAIDEX_1;         // now couting 2 secs
	TA0CCR0 = 0xFA00;
	TA0CCTL0 |= CCIE;
}

