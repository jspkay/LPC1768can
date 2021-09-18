#include "lpc17xx.h"
#include "lib_adc.h"

unsigned char adc_random;
unsigned char adc_done = 0;

/*----------------------------------------------------------------------------
  Function that initializes ADC
 *----------------------------------------------------------------------------*/
void ADC_init (void) {

  LPC_PINCON->PINSEL3 |=  (3UL<<30);      /* P1.31 is AD0.5                     */

  LPC_SC->PCONP       |=  (1<<12);      /* Enable power to ADC block          */

  LPC_ADC->ADCR        =  (1<< 5) |     /* select AD0.5 pin                   */
                          (8<< 8) |     /* ADC clock is 25MHz/5               */
                          (1<<21);      /* enable ADC                         */ 

  LPC_ADC->ADINTEN     =  (1<< 8);      /* global enable interrupt            */
	
  NVIC_EnableIRQ(ADC_IRQn);             /* enable ADC Interrupt               */

	/* these lines are used to enable DAC */
	LPC_PINCON->PINSEL1 |= (1<<21);
	LPC_PINCON->PINSEL1 &= ~(1<<20);
	LPC_GPIO0->FIODIR |= (1<<26);

}

void ADC_start_conversion (void) {
	LPC_ADC->ADCR |=  (1<<24);            /* Start A/D Conversion 				*/
}				


unsigned char ADC_generate_random(void){
	ADC_start_conversion();
	
	while(adc_done == 0); // wait for interrupt
	
	adc_done = 0;
	
	return adc_random;
}