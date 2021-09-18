#include "lpc17xx.h"
#include "./highcan.h"
#include "../GLCD/GLCD.h"
#include <security.h>
#include "trng/lib_adc.h"

void IRQ_CAN(int canBus);

extern unsigned char key[3][8];
extern struct AES_ctx ctx_dec[2];

void CAN_IRQHandler (void)
{
	int32_t icr = LPC_CAN1->ICR; // clear the interrupt
	
//	if( (icr & CAN_ICR_EPI) != 0 )
//		GUI_Text(0, 20*counter++, (uint8_t*) "EPI ERROR!", Black, Yellow);
	
	int canBus = 0;
	if( (LPC_CAN1->GSR & 1) == 1) canBus = 1;
	//else if( (LPC_CAN2->GSR & 1) == 1) canBus = 2;
	
	if(canBus != 0)
		IRQ_CAN(canBus);
	
	
	uint8_t c[2];
 	c[0] = ADC_generate_random();
	c[1] = 0;
	GUI_Text(10, 240, c, Black, Yellow);
}

void IRQ_CAN(int canBus){
	
	unsigned char finestrino[16] = {0};
	
	if(hCAN_receiveMessage(canBus) == hCAN_SUCCESS && hCAN_recDone){
		hCAN_recMessage[hCAN_lenght] = 0;
		GUI_Text(10, 50, (uint8_t*) "criptato: ", Black, Yellow);
		GUI_Text(10, 70, (uint8_t*) hCAN_recMessage, Black, Yellow);
		
		
		if( hCAN_recID == 0x1 ){
			GUI_Text(10, 120, (uint8_t*) "livello finestrino: ", Black, Yellow);
			
			for(int i=0;i<16;i++)
				finestrino[i] = hCAN_recMessage[i];
			
			//DES3((unsigned char*) finestrino, key, DECRYPT);
			AES(&ctx_dec[hCAN_recID-1], (unsigned char*) finestrino);
			for(int i=0; i<100; i++);
			
			finestrino[1] = finestrino[0] + '0';
			finestrino[0] = '0';
			finestrino[2] = 0;
			if(finestrino[1] > 9 + '0'){
				finestrino[1] = finestrino[1] - 10;
				finestrino[0] = '1';
			}
			
			GUI_Text(10, 140, (uint8_t*) finestrino, Black, Yellow);
		}
		
		if( hCAN_recID == 0x2 ){
			AES(&ctx_dec[hCAN_recID-1], (unsigned char*) hCAN_recMessage);
			for(int i=0; i<100; i++);
			GUI_Text(10, 180, (uint8_t*) "luci: ", Black, Yellow);
			for(int i=0; i<6; i++){
				hCAN_recMessage[i] += '0';
			}
			hCAN_recMessage[6] = 0;
			GUI_Text(10, 200, (uint8_t*) hCAN_recMessage, Black, Yellow);
		}
		
	}
	
	if(hCAN_arbitrationLost(canBus)){
		GUI_Text(0, 0, (uint8_t*) "ARBITRATION LOST! SOMEONE IS TRASMIITTING", Yellow, Red);
	};
	
}
