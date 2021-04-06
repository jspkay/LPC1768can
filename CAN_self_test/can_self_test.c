/**********************************************************************
* $Id$		can_self_test.c			2010-06-18
*//**
* @file		can_self_test.c
* @brief	This example used to test Self-test mode
* @version	1.0
* @date		18. June. 2010
* @author	NXP MCU SW Application Team
*
* Copyright(C) 2010, NXP Semiconductor
* All rights reserved.
*
***********************************************************************
* Software that is described herein is for illustrative purposes only
* which provides customers with programming information regarding the
* products. This software is supplied "AS IS" without any warranties.
* NXP Semiconductors assumes no responsibility or liability for the
* use of the software, conveys no license or title under any patent,
* copyright, or mask work right to the product. NXP Semiconductors
* reserves the right to make changes in the software without
* notification. NXP Semiconductors also make no representation or
* warranty that such application will be suitable for the specified
* use without further testing or modification.
**********************************************************************/
#include "lpc17xx_can.h"
#include "lpc17xx_libcfg.h"
#include "TouchPanel.h"
#include "GLCD.h"
#include <stdio.h>

//GUI_Text(40, 140, (uint8_t *) "   Congratulations",White,Blue);

/* Example group ----------------------------------------------------------- */
/** @defgroup CAN_seft_test	CAN_self_test
 * @ingroup CAN_Examples
 * @{
 */
/************************** PRIVATE VARIABLES *************************/
uint8_t menu[]=
	"*******************************************************************************\n\r"
	"Hello NXP Semiconductors \n\r"
	"CAN Self-test demo \n\r"
	"\t - MCU: LPC17xx \n\r"
	"\t - Core: ARM CORTEX-M3 \n\r"
	"\t - Communicate via: UART0 - 115200 bps \n\r"
	"Use only CAN1 peripherals to test\n\r"
	"This example used to test Self test mode\n\r"
	"*******************************************************************************\n\r";

/** CAN variable definition **/
CAN_MSG_Type TXMsg, RXMsg; // messages for test Bypass mode
uint32_t CANRxCount, CANTxCount = 0;

/************************** PRIVATE FUNCTIONS *************************/
void CAN_IRQHandler(void);
void CAN_InitMessage(void);
void PrintMessage(CAN_MSG_Type* msg);
//void print_menu();
Bool Check_Message(CAN_MSG_Type* TX_Msg, CAN_MSG_Type* RX_Msg);


/*----------------- INTERRUPT SERVICE ROUTINES --------------------------*/
/*********************************************************************//**
 * @brief		CAN_IRQ Handler, control receive message operation
 * param[in]	none
 * @return 		none
 **********************************************************************/
void CAN_IRQHandler()
{
	uint8_t IntStatus;
//	uint32_t data1;
	/* Get CAN status */
	IntStatus = CAN_GetCTRLStatus(LPC_CAN1, CANCTRL_STS);
	//check receive buffer status
	if((IntStatus>>0)&0x01)
	{
		CAN_ReceiveMsg(LPC_CAN1,&RXMsg);
		//_DBG_("Received buffer:");
		GUI_Text(20, 20, (uint8_t *) "Received buffer: ",White,Blue);
		PrintMessage(&RXMsg);
		//Validate received and transmited message
		if(Check_Message(&TXMsg, &RXMsg))
			GUI_Text(20, 220, (uint8_t *) "Self test is SUCCESSFUL!!!",White,Blue);
			//_DBG_("Self test is SUCCESSFUL!!!");
		else
			//_DBG_("Self test is FAIL!!!");
			GUI_Text(20, 220, (uint8_t *) "Self test is FAIL!!!",White,Blue);
	}
}

/*-------------------------PRIVATE FUNCTIONS----------------------------*/
/*********************************************************************//**
 * @brief		Print Message via COM1
 * param[in]	msg: point to CAN_MSG_Type object that will be printed
 * @return 		none
 **********************************************************************/
void PrintMessage(CAN_MSG_Type* CAN_Msg)
{
	char data[5]="";
	uint32_t id;
	uint8_t format;
	uint8_t len;
	char dataA[4];
	char dataB[4];
	char s[30];
	GUI_Text(20, 40, (uint8_t *) "*************************",White,Blue);
  sprintf(s,"Message ID: %u",CAN_Msg->id);
	GUI_Text(20, 60, (uint8_t *) s,White,Blue);
	sprintf(s,"Message length: %u BYTES",CAN_Msg->len);
	GUI_Text(20, 80, (uint8_t *)s,White,Blue);
	GUI_Text(20, 100, (uint8_t *) "Message format: ",White,Blue);
	if(CAN_Msg->format==STD_ID_FORMAT)
	{
		GUI_Text(20, 120, (uint8_t *) "STANDARD ID FRAME FORMAT",White,Blue);
	}
	else
		GUI_Text(20, 120, (uint8_t *) "EXTENDED ID FRAME FORMAT",White,Blue);
	data[0] = CAN_Msg->dataA[0];
	data[1] = CAN_Msg->dataA[1];
	data[2] = CAN_Msg->dataA[2];
	data[3] = CAN_Msg->dataA[3];
	//sprintf(s,"%s",data);
	sprintf(s,"Message dataA: %s",data);
	GUI_Text(20, 140, (uint8_t *)s,White,Blue);
	data[0] = CAN_Msg->dataB[0];
	data[1] = CAN_Msg->dataB[1];
	data[2] = CAN_Msg->dataB[2];
	data[3] = CAN_Msg->dataB[3];
	sprintf(s,"Message dataB: %s",data);
	GUI_Text(20, 160, (uint8_t *)s,White,Blue);
	GUI_Text(20, 180, (uint8_t *) "*************************",White,Blue);
	/*
	_DBG("Message ID:     ");
	_DBH32(CAN_Msg->id);_DBG_("");
	_DBG("Message length: ");
	_DBH32(CAN_Msg->len);_DBG_(" BYTES");
	_DBG("Message type:   ");
	if(CAN_Msg->type==DATA_FRAME)
	{
		_DBG_("DATA FRAME ");
	}
	else
		_DBG_("REMOTE FRAME ");
	_DBG("Message format: ");
	if(CAN_Msg->format==STD_ID_FORMAT)
	{
		_DBG_("STANDARD ID FRAME FORMAT");
	}
	else
		_DBG_("EXTENDED ID FRAME FORMAT");
	_DBG("Message dataA:  ");
	data = (CAN_Msg->dataA[0])|(CAN_Msg->dataA[1]<<8)|(CAN_Msg->dataA[2]<<16)|(CAN_Msg->dataA[3]<<24);
	_DBH32(data);_DBG_("");
	data = (CAN_Msg->dataB[0])|(CAN_Msg->dataB[1]<<8)|(CAN_Msg->dataB[2]<<16)|(CAN_Msg->dataB[3]<<24);
	_DBG("Message dataB:  ");
	_DBH32(data);_DBG_("");
	_DBG_("");*/
	
}

/*********************************************************************//**
 * @brief		Initialize transmit and receive message for Bypass operation
 * @param[in]	none
 * @return 		none
 **********************************************************************/
void CAN_InitMessage(void) {
	TXMsg.format = EXT_ID_FORMAT;
	TXMsg.id = 0x00001234;
	TXMsg.len = 8;
	TXMsg.type = DATA_FRAME;
	TXMsg.dataA[0] = 0x48; //"H"
	TXMsg.dataA[1] = 0x45; //"E";
	TXMsg.dataA[2] = 0x4c;//"L";
	TXMsg.dataA[3] = 0x4c;//"L";
	TXMsg.dataB[0] = 0x4F;//"O";
	TXMsg.dataB[1] = 0x41;//"A";
	TXMsg.dataB[2] = 0x53;//"S";
	TXMsg.dataB[3] = 0x45;//"E";

	RXMsg.format = 0x00;
	RXMsg.id = 0x00;
	RXMsg.len = 0x00;
	RXMsg.type = 0x00;
	RXMsg.dataA[0] = RXMsg.dataA[1] = RXMsg.dataA[2] = RXMsg.dataA[3] = 0x00000000;
	RXMsg.dataB[0] = RXMsg.dataA[1] = RXMsg.dataA[2] = RXMsg.dataA[3] = 0x00000000;
}

/*********************************************************************//**
 * @brief		print menu
 * @param[in]	none
 * @return 		none
 **********************************************************************/
/*void print_menu()
{
	_DBG_(menu);
}*/

/*********************************************************************//**
 * @brief		Compare two message
 * @param[in]	Tx_Msg transmit message
 * @param[in]	Rx_Msg receive message
 * @return 		Bool	should be:
 * 				- TRUE: if two message is the same
 * 				- FALSE: if two message is different
 **********************************************************************/
Bool Check_Message(CAN_MSG_Type* TX_Msg, CAN_MSG_Type* RX_Msg)
{
	uint8_t i;
	if((TXMsg.format != RXMsg.format)|(TXMsg.id != RXMsg.id)|(TXMsg.len != RXMsg.len)\
		|(TXMsg.type != RXMsg.type))
		return FALSE;
	for(i=0;i<4;i++)
	{
		if((TXMsg.dataA[i]!=RXMsg.dataA[i])|(TXMsg.dataB[i]!=RXMsg.dataB[i]))
			return FALSE;
	}
	return TRUE;
}
/*-------------------------MAIN FUNCTION------------------------------*/
/*********************************************************************//**
 * @brief		c_entry: Main CAN program body
 * @param[in]	none
 * @return 		int
 **********************************************************************/
int c_entry(void) { /* Main Program */

	/* Initialize debug via UART0
	 * – 115200bps
	 * – 8 data bit
	 * – No parity
	 * – 1 stop bit
	 * – No flow control
	 */
	//debug_frmwrk_init();
	//print_menu();

	/* Initialize CAN1 peripheral
	 * Note: Self-test mode doesn't require pin selection
	 */
	CAN_Init(LPC_CAN1, 125000);

	//Enable self-test mode
	CAN_ModeConfig(LPC_CAN1, CAN_SELFTEST_MODE, ENABLE);

	//Enable Interrupt
	CAN_IRQCmd(LPC_CAN1, CANINT_RIE, ENABLE);
	CAN_IRQCmd(LPC_CAN1, CANINT_TIE1, ENABLE);

	//Enable CAN Interrupt
	NVIC_EnableIRQ(CAN_IRQn);
	CAN_SetAFMode(LPC_CANAF,CAN_AccBP);
	CAN_InitMessage();
	GUI_Text(20, 20, (uint8_t *) "Transmitted buffer:",White,Blue);
	//_DBG_("Transmitted buffer:");
	PrintMessage(&TXMsg);

	/** To test Bypass Mode: we send infinite messages to CAN2 and check
	 * receive process via COM1
	 */
	LCD_Clear(Blue);
	CAN_SendMsg(LPC_CAN1, &TXMsg);
	LPC_CAN1->CMR |=(1<<4); //Self Reception Request

	while (1);
}

/* With ARM and GHS toolsets, the entry point is main() - this will
 allow the linker to generate wrapper code to setup stacks, allocate
 heap area, and initialize and copy code and data segments. For GNU
 toolsets, the entry point is through __start() in the crt0_gnu.asm
 file, and that startup code will setup stacks and data */
int main(void) {
	LCD_Initialization();
	LCD_Clear(Blue);
	return c_entry();
}

#ifdef  DEBUG
/*******************************************************************************
 * @brief		Reports the name of the source file and the source line number
 * 				where the CHECK_PARAM error has occurred.
 * @param[in]	file Pointer to the source file name
 * @param[in]    line assert_param error line source number
 * @return		None
 *******************************************************************************/
void check_failed(uint8_t *file, uint32_t line) {
	/* User can add his own implementation to report the file name and line number,
	 ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

	/* Infinite loop */
	while (1)
		;
}
#endif

/*
 * @}
 */
