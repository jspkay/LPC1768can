#include "LPC17xx.h"
#include "GLCD/GLCD.h" 
#include <highcan.h>
#include <string.h>
#include <stdio.h>
#include "security.h"
#include "trng/lib_adc.h"

#ifdef SIMULATOR
extern uint8_t ScaleFlag; // <- ScaleFlag needs to visible in order for the emulator to find the symbol (can be placed also inside system_LPC17xx.h but since it is RO, it needs more work)
#endif


unsigned char key[3][8];

uint8_t key_aes[16] = {'A', 'A', 'A', 'A', 'A', 'A', 'A', 'A', 'A', 'A', 'A', 'A', 'A', 'A', 'A', 'A'};
uint8_t iv[16] = {'B', 'B', 'B', 'B', 'B', 'B', 'B', 'B', 'B', 'B', 'B', 'B', 'B', 'B', 'B', 'B'};
struct AES_ctx ctx_dec[2];

int main(void){
	uint8_t string1[] = "Inizialization";
	uint8_t string2[] = "Transmitting...";
	uint8_t string3[] = "Transmission done!";
	uint8_t string4[] = "...";
	uint8_t string5[] = "DONE";
	uint8_t string6[] = "ERROR";
	uint8_t receivedString[8];
	int i;
	
	unsigned char *p = (unsigned char*) key;
	for(int i=0; i<24; i++){
		p[i] = 'A';
	}
	
  SystemInit();  												/* System Initialization (i.e., PLL)  */
	
	#ifdef ARDUINO_TEST
	hCAN_init(1, CAN_5Kbps);
	//hCAN_init(2, CAN_5Kbps);
	#else
	hCAN_init(1, CAN_1Mbps);
	#endif
	//NVIC_DisableIRQ(CAN_IRQn);
	
	ADC_init();
	LCD_Initialization();
	LCD_Clear(Blue);
	GUI_Text(10, 10, string1, Black, Blue);
	
	ctx_dec[0] = AES_init(key_aes, iv);
	ctx_dec[1] = AES_init(key_aes, iv);
	
	char msgBuff[] = "Stringa di lunghezza arbitraria che continua a camminare sui suoi piedi. Q1234";
	int ret = 0; // hCAN_sendMessage(1, msgBuff, strlen(msgBuff) );
	char bf[20];
	sprintf(bf, "%d", ret);
	GUI_Text(0, 0, (uint8_t*) bf, White, Blue); 

	int c = ADC_generate_random();

	
  while (1){
		__ASM("wfi");
  }
}

/*********************************************************************************************************
      END FILE
*********************************************************************************************************/
