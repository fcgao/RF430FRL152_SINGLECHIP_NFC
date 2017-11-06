/*
 * timer.c
 *
 *  Created on: Sep 11, 2017
 *      Author: kiran
 */
#include <rf430frl152h.h>
#include "timer.h"
//timer is running at 64KHz at 1.0239seconds per interrupt
void initTimer(){
	TA0CTL |= TASSEL_1 + TAIE;	//aux clock and interrup enabled
	TA0CTL &= ~TAIFG;			//clear interrupt
	TA0CTL |= MC_2;				//count up to oxffffh
}

