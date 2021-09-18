#include <string.h>

/* lib_adc.c */
void ADC_init (void);
static void ADC_start_conversion (void);
unsigned char ADC_generate_random(void);

/* IRQ_adc.c */
void ADC_IRQHandler(void);