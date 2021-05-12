#include "lpc17xx.h"
#include "./highcan.h"

void IRQ_CAN1(void);
void IRQ_CAN2(void);

void CAN_IRQHandler (void)
{
	if( (LPC_CAN1->GSR & 1) == 1) IRQ_CAN1();
	else if( (LPC_CAN2->GSR & 1) == 1) IRQ_CAN2();
	
	// TODO: clear interrupt request
}

void IRQ_CAN1(){
	if(hCAN_receiveMessage(1) == hCAN_SUCCESS && hCAN_recDone){
		// processa messagio in hCAN_recMessage
	}
}

void IRQ_CAN2(){
	
}
