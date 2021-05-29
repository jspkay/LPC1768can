#include "lpc17xx.h"
#include "./highcan.h"
#include "../GLCD/GLCD.h"

void IRQ_CAN(int canBus);

extern int counter;

void CAN_IRQHandler (void)
{
	int32_t icr = LPC_CAN1->ICR; // clear the interrupt
	
	if( (icr & CAN_ICR_EPI) != 0 )
		GUI_Text(0, 20*counter++, (uint8_t*) "EPI ERROR!", Black, Yellow);
	
	int canBus = 0;
	if( (LPC_CAN1->GSR & 1) == 1) canBus = 1;
	//else if( (LPC_CAN2->GSR & 1) == 1) canBus = 2;
	
	if(canBus != 0)
		IRQ_CAN(canBus);
	// TODO: clear interrupt request
}

void IRQ_CAN(int canBus){
	
	if(hCAN_receiveMessage(canBus) == hCAN_SUCCESS && hCAN_recDone){
		hCAN_recMessage[hCAN_lenght] = 0;
		GUI_Text(0, 0, (uint8_t*) hCAN_recMessage, Black, Yellow);
	}
	
	if(hCAN_arbitrationLost(canBus)){
	
	};
	
}
