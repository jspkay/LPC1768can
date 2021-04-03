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
#include "LPC17xx.h"
#include "GLCD/GLCD.h" 
#include "TouchPanel/TouchPanel.h"
#include "timer/timer.h"
#include "joystick/joystick.h"
#include "CAN_final/can.h"

#ifdef SIMULATOR
extern uint8_t ScaleFlag; // <- ScaleFlag needs to visible in order for the emulator to find the symbol (can be placed also inside system_LPC17xx.h but since it is RO, it needs more work)
#endif

CAN_MSG_type Tx1_Buff , Rx1_Buff , Tx2_Buff , Rx2_Buff  ;
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
	uint8_t string2[] = "Transmitting...";
	uint8_t string3[] = "Transmission done!";
	uint8_t string4[] = "...";
	uint8_t string5[] = "DONE";
	uint8_t string6[] = "ERROR";
	uint8_t receivedString[8];
	int i;
	
  SystemInit();  												/* System Initialization (i.e., PLL)  */
	
	init_CAN () ; 
	LCD_Initialization();
	LCD_Clear(Blue);
	GUI_Text(10, 10, string1, Black, Blue);
	
	Tx1_Buff.FRAME = 0x00080000; 		  /* 11-bit STD, no RTR (data frame) , Data Length = 8 bytes */
	Tx1_Buff.MSG_ID = EXP_STD_ID; 	 /* Try with MSG_ID = 0x125 and then MSG_ID = 0x123 */
																		/** @NOTE :CAN2 will not Rx ID 0x123 since it isn't setup in the Acceptance Filter **/  
	Tx1_Buff.Data_A = 1868654947;
	Tx1_Buff.Data_B = 3355185 ;

	Rx2_Buff.FRAME = 0x00;
	Rx2_Buff.MSG_ID = 0x00;
	Rx2_Buff.Data_A = 0x00;
	Rx2_Buff.Data_B = 0x00;
	CONFIG_CAN_FILTER_MODE( AF_ON );
	
	while (1)
  {
		/* Transmit message on CAN 1 */
		while ( !(LPC_CAN1->GSR & (1 << 3)) )
			GUI_Text(10, 50, string3, Black, Blue) ;
		GUI_Text(10, 60, string4, Yellow, Blue);
		if (CAN1_Tx( &Tx1_Buff ) == NOT_OK){
			continue;
		}else{
			GUI_Text(10, 50, string2, Black, Blue);
		}
		

		if ( CAN2RxDone == OK ){
			CAN2RxDone = NOT_OK;
			
			if ( (Tx1_Buff.MSG_ID != Rx2_Buff.MSG_ID ) ||
				( Tx1_Buff.Data_A != Rx2_Buff.Data_A ) ||
				( Tx1_Buff.Data_B != Rx2_Buff.Data_B ) )
			{
				GUI_Text(10, 80, string6, White, Blue) ; /// Print on LCD 
			}
			
			// Print received
			for(i=0; i<4; i++)
				receivedString[i] = Rx2_Buff.Data_A >> (8*i);
			for(i=0; i<4; i++)
				receivedString[i+4] = Rx2_Buff.Data_B >> (8*i);
			GUI_Text(10, 100, receivedString, White, Blue);
			
			Rx2_Buff.FRAME = 0x0;
			Rx2_Buff.MSG_ID = 0x0;
			Rx2_Buff.Data_A = 0x0;
			Rx2_Buff.Data_B = 0x0;
		} /* Message on CAN 2 received */
		break;
  }	
	
	/*
	joystick_init();
	TP_Init();
	TouchPanel_Calibrate();
	
	LPC_SC->PCON |= 0x1;									/* power-down	mode										
	LPC_SC->PCON &= ~(0x2);						
	
	 */
  while (1)	
  {
		__ASM("wfi");
  }
}

/*********************************************************************************************************
      END FILE
*********************************************************************************************************/
