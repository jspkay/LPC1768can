/****************************************Copyright (c)****************************************************
**                                      
**                                 http://www.powermcu.com
**
**--------------File Info---------------------------------------------------------------------------------
** File name:               main.c
** Descriptions:            The GLCD application function
**
**--------------------------------------------------------------------------------------------------------
** Created by:              AVRman
** Created date:            2010-11-7
** Version:                 v1.0
** Descriptions:            The original version
**
**--------------------------------------------------------------------------------------------------------
** Modified by:             Paolo Bernardi
** Modified date:           03/01/2020
** Version:                 v2.0
** Descriptions:            basic program for LCD and Touch Panel teaching
**
*********************************************************************************************************/

/* Includes ------------------------------------------------------------------*/
extern "C"{
	#include "LPC17xx.h"
	#include "GLCD/GLCD.h" 
	#include "TouchPanel/TouchPanel.h"
	#include "timer/timer.h"
	#include "joystick/joystick.h"
	//#include "CAN_final/can.c.h"
}

#include "CAN_final/can.h"

#ifdef SIMULATOR
extern uint8_t ScaleFlag; // <- ScaleFlag needs to visible in order for the emulator to find the symbol (can be placed also inside system_LPC17xx.h but since it is RO, it needs more work)
#endif

CAN_MSG Tx1_Buff , Rx1_Buff , Tx2_Buff , Rx2_Buff  ;
uint32_t CAN1_Error_Cnt = 0, CAN2_Error_Cnt = 0;
volatile uint8_t CAN2RxDone = 0 , CAN1RxDone = 0; 

void delay(int c){
	int a = 0;
	while(a <= c){
		a++;
	}
}


int main(void){
	uint8_t string1[] = "Inizialization";
	uint8_t string4[] = "...";
	uint8_t string6[] = "ERROR";
	uint8_t receivedString[8];
	
  SystemInit();  												/* System Initialization (i.e., PLL)  */
	
//	init_CAN () ; 
	LCD_Initialization();
	LCD_Clear(Blue);
	GUI_Text(10, 10, string1, Black, Blue);
	
	CAN can1=CAN::can1, can2=CAN::can2;

	//Tx1_Buff.FRAME = 0x00080000; 		  /* 11-bit STD, no RTR (data frame) , Data Length = 8 bytes */
	//Tx1_Buff.MSG_ID = EXP_STD_ID; 	 /* Try with MSG_ID = 0x125 and then MSG_ID = 0x123 */
																		/** @NOTE :CAN2 will not Rx ID 0x123 since it isn't setup in the Acceptance Filter **/  
	//Tx1_Buff.Data_A = 1868654947;
	//Tx1_Buff.Data_B = 3355185 ;

	CONFIG_CAN_FILTER_MODE( AF_ON );
	
	CAN_MSG msg;
	msg.id = 0x100;
	msg.data.full = 1868654947 + 3355185;

	while( can1.transmitFrame(msg) != OK ); // send the message

	while(!can2.received); // wait until can2 receives the message
	if(can2.received){
		GUI_Text(10, 80, string6, White, Blue) ; /// Print on LCD 
		
		for(int i=0; i<4; i++) receivedString[i] = can2.rx_msg.data.parts[0] >> (8*i);
		for(int i=0; i<4; i++) receivedString[i+4] = can2.rx_msg.data.parts[1] >> (8*i);

		GUI_Text(10, 100, receivedString, White, Blue);
		can2.received = false;
	}
					
	
	while (true){
		__ASM("wfi");
	}
}

/*********************************************************************************************************
      END FILE
*********************************************************************************************************/
