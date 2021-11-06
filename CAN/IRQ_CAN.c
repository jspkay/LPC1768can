#include "lpc17xx.h"
#include "./highcan.h"
#include "../GLCD/GLCD.h"
#include <security.h>
#include "trng/lib_adc.h"

extern char msg[16];
extern struct AES_ctx break_dec_ctx;
extern struct AES_ctx dec_ctx;
extern struct AES_ctx ctx;

extern uint8_t lock;

void IRQ_CAN_RECEIVE(int canBus);
void IRQ_CAN_BEI_HANDLER(LPC_CAN_TypeDef * can);
void IRQ_CAN_ALI_HANDLER(LPC_CAN_TypeDef * can);

void CAN_IRQHandler (void)
{
	int32_t icr = LPC_CAN1->ICR; // clear the interrupt
	LPC_CAN_TypeDef *can = LPC_CAN1;
	
	for(int i=1; i<=2; i++){
		if(i==1) can = LPC_CAN1;
		else if(i==2) can = LPC_CAN2;
		
		int canBusReceive = 0;
		if( (can->GSR & 1) == 1) IRQ_CAN_RECEIVE(i);
		
		// Bus Error Interrupt
		if( (icr & CAN_ICR_BEI) != 0)
			IRQ_CAN_BEI_HANDLER(can);
		
		// Arbitration lost interrupt
		if( (icr & CAN_ICR_ALI) != 0 )
			IRQ_CAN_ALI_HANDLER(can);
		
	}
}

void IRQ_CAN_RECEIVE(int canBus){
	
	unsigned char finestrino[16] = {0};
	unsigned char freno_acc[16] = {0};
	
	if(hCAN_receiveMessage(canBus) == hCAN_SUCCESS && hCAN_recDone[canBus-1]){
		hCAN_recMessage[canBus-1][hCAN_lenght[canBus-1]] = 0;
		GUI_Text(10, 50, (uint8_t*) "criptato: ", Black, Yellow);
		GUI_Text(10, 70, (uint8_t*) hCAN_recMessage, Black, Yellow);
		
		
		if( hCAN_recID[canBus-1] == 0x1 ){
			GUI_Text(10, 120, (uint8_t*) "livello finestrino: ", Black, Yellow);
			
			for(int i=0;i<16;i++)
				finestrino[i] = hCAN_recMessage[canBus-1][i];
			
			//DES3((unsigned char*) finestrino, key, DECRYPT);
			AES(&ctx_dec[hCAN_recID[canBus-1]-1], (unsigned char*) finestrino);
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
		
		if( hCAN_recID[canBus-1] == 0x2 ){
			AES(&ctx_dec[hCAN_recID[canBus-1]-1], (unsigned char*) hCAN_recMessage);
			for(int i=0; i<100; i++);
			GUI_Text(10, 180, (uint8_t*) "luci: ", Black, Yellow);
			for(int i=0; i<6; i++){
				hCAN_recMessage[canBus-1][i] += '0';
			}
			hCAN_recMessage[canBus-1][6] = 0;
			GUI_Text(10, 200, (uint8_t*) hCAN_recMessage, Black, Yellow);
		}
		if( hCAN_recID[canBus-1] == 0x3 ){
			for(int i=0;i<16;i++)
				freno_acc[i] = hCAN_recMessage[canBus-1][i];
			
			AES(&ctx_dec[hCAN_recID[canBus-1]-1], (unsigned char*) hCAN_recMessage);
			for(int i=0; i<100; i++);
			GUI_Text(10, 200, (uint8_t*) "freno: ", Black, Yellow);
			for(int i=0; i<6; i++){
				hCAN_recMessage[canBus-1][i] += '0';
			}
			freno_acc[1] = freno_acc[0] + '0';
			freno_acc[0] = '0';
			freno_acc[2] = 0;
			if(freno_acc[1] > 9 + '0'){
				freno_acc[1] = freno_acc[1] - 10;
				freno_acc[0] = '1';
			}			GUI_Text(10, 200, (uint8_t*) hCAN_recMessage, Black, Yellow);
		}
	}
	
	if(hCAN_arbitrationLost(canBus)){
		GUI_Text(0, 0, (uint8_t*) "ARBITRATION LOST! SOMEONE IS TRASMIITTING", Yellow, Red);
	};
	
}

void IRQ_CAN_BEI_HANDLER(LPC_CAN_TypeDef* can){
	CAN_resetTXERR(can);
	CAN_abortTransmission(can);
	hCAN_ActiveError = hCAN_ERR_BUS_ERROR;
}

void IRQ_CAN_ALI_HANDLER(LPC_CAN_TypeDef* can){
	CAN_resetTXERR(can);
	CAN_abortTransmission(can);
	hCAN_ActiveError = hCAN_ERR_COLLISION;
}
