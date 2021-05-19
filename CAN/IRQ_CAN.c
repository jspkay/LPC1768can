#include "lpc17xx.h"
#include "./highcan.h"
#include "../GLCD/GLCD.h"

void IRQ_CAN1(void);
void IRQ_CAN2(void);

void CAN_IRQHandler (void)
{
	if( (LPC_CAN1->GSR & 1) == 1) IRQ_CAN1();
	else if( (LPC_CAN2->GSR & 1) == 1) IRQ_CAN2();
	
	// TODO: clear interrupt request
}

void IRQ_CAN1(){
	int32_t icr = LPC_CAN1->ICR; // clear the interrupt
	if(hCAN_receiveMessage(1) == hCAN_SUCCESS && hCAN_recDone){
		hCAN_recMessage[hCAN_lenght] = 0;
		GUI_Text(0, 0, (uint8_t*) hCAN_recMessage, Black, Yellow);
	}
}

void IRQ_CAN2(){
	
}
