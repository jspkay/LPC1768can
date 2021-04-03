/**
  ******************************************************************************
  * @file    CAN.h 
  * @author  Pranjal Shrivastava
  * @version v2.0 - CAN  
  * @date    20-July-2019
  * @brief   CAN Header File
  ******************************************************************************
 **/


#ifndef _CAN_H
#define _CAN_H

#include "LPC17xx.h"                    // Device header
#include "../utilities/string.h"

#define AF_OFF			0x01
#define AF_BYPASS		0x02
#define AF_ON			0x00
#define AF_FULLCAN		0x04

/* Identifiers for EXP STD, GRP STD */
#define EXP_STD_ID		0x100
#define GRP_STD_ID		0x100

#define OK 1 
#define NOT_OK 0 

typedef struct{
	uint32_t id;
	union{
		uint64_t full;
		uint32_t parts[2];
	}data;
} CAN_MSG;

class CAN{
	static int x;

	LPC_CAN_TypeDef *can;
	uint32_t Rx_frame, Tx_frame;
	
public:
	static CAN can1, can2;
	CAN_MSG rx_msg;
	bool received;

	CAN();
	//CAN& operator=(const CAN& src);
	
	int transmitFrame(CAN_MSG data);
	CAN_MSG receiveFrame(void);
};

void setup_LUT(void);
void CONFIG_CAN_FILTER_MODE (uint8_t mode);

//void init_CAN(void)  ;
//uint8_t CAN1_Tx (CAN_MSG_type* tx_data) ;
//uint8_t CAN2_Tx (CAN_MSG_type* tx_data) ;
//void do_CAN1_rx(void) ; 
//void do_CAN2_rx(void) ;
//void setup_LUT(void) ; 
//void CONFIG_CAN_FILTER_MODE (uint8_t mode) ; 


#endif

