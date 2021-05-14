#include "lpc17xx.h"
#include "./highcan.h"

void IRQ_CAN1(void);
void IRQ_CAN2(void);
extern int bus_mine;


void panic(){}
	
void CAN_IRQHandler (void)
{
	if( (LPC_CAN1->GSR & 1) == 1) IRQ_CAN1();
	else if( (LPC_CAN2->GSR & 1) == 1) IRQ_CAN2();

	LPC_CAN_TypeDef* can;
	can = LPC_CAN1;
	
	//gestione errori
	if((can->GSR & CAN_GSR_DOS) == CAN_GSR_DOS){ //OVERRUN
		panic();
	}
	if((can->ICR & CAN_ICR_WUI) == CAN_ICR_WUI){ //DETECTED BUS ACTIVITY
		panic();
	}
	if((can->ICR & CAN_ICR_EPI) == CAN_ICR_EPI){ //TOO MUCH ERRORS
		panic();
	}
	if((can->ICR & CAN_ICR_ALI) == CAN_ICR_ALI){ //ARBITRATION LOST
		bus_mine=0;
	}
}

void IRQ_CAN1(){
	if(hCAN_receiveMessage(1) == hCAN_SUCCESS && hCAN_recDone){
		// processa messagio in hCAN_recMessage
	}
}

void IRQ_CAN2(){
	
}
