/*********************************************************************************************************
**--------------File Info---------------------------------------------------------------------------------
** File name:           IRQ_adc.c
** Last modified Date:  20184-12-30
** Last Version:        V1.00
** Descriptions:        functions to manage A/D interrupts
** Correlated files:    adc.h
**--------------------------------------------------------------------------------------------------------       
*********************************************************************************************************/

#include "lpc17xx.h"
#include "lib_adc.h"
#include "../timer/timer.h"
#include "../GLCD/GLCD.h"
#include <stdio.h>
#include <stdlib.h>
#include "string.h"

/*----------------------------------------------------------------------------
  A/D IRQ: Executed when A/D Conversion is ready (signal from ADC peripheral)
 *----------------------------------------------------------------------------*/

unsigned short AD_current;   
unsigned short AD_last = 0xFF;     /* Last converted value               */
char converted[5];

int check=0;
static int cont = 0;
extern unsigned char adc_random, adc_done;

void ADC_IRQHandler(void) {
/*	char s[20];
  if(cont+1 == 13)
		cont=1;
	else
		cont++; */

	adc_random = ((LPC_ADC->ADGDR>>4) & 0xFFF);/* Read Conversion Result             */  
	adc_done = 1;
  //AD_current = ((LPC_ADC->ADGDR>>4) & 0xFFF);/* Read Conversion Result             */  
	
	//sprintf(s,"ciao val %x", AD_current);
	//GUI_Text(10, cont*20, (unsigned char*)s, White, Blue);
	
	//AD_last = AD_current;
}
