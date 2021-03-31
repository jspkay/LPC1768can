/**
  ******************************************************************************
  * @file    CAN.c 
  * @author  Pranjal Shrivastava
  * @version v2.0 - CAN  
  * @date    20-July-2019
  * @brief   Main CAN Drivers
  ******************************************************************************
 **/
 
#include "CAN.h"

// Declared in sample.c (main group)
extern CAN_MSG_type Tx1_Buff , Rx1_Buff , Tx2_Buff , Rx2_Buff  ;
extern uint32_t CAN1_Error_Cnt , CAN2_Error_Cnt ;
extern volatile uint8_t CAN2RxDone , CAN1RxDone ;



static uint32_t CAN1_Rx_cnt = 0 , CAN2_Rx_cnt = 0 ;


/** 
	@Brief 	     : Initialize CAN Modules, Powers ON both Modules, Configures Pins, Sets up Bitrate, Enables CAN Interupt
	@retval	     : None 
	@Argumemnts  : None 
**/ 

CAN::CAN(uint8_t x, uint32_t ID){
	can = x==1 ? LPC_CAN1 : LPC_CAN2;
	
	if(x == 1){ // CAN 1
		LPC_SC -> PCONP |= (0x1<<13); // power on
		LPC_PINCON->PINSEL0 &= ~0xF;  // pin function select
		LPC_PINCON->PINSEL0 |= 0x5;	  // function P0.0 = 01, P0.1 = 01
	}
	if(x == 2){ // CAN 2
		LPC_SC -> PCONP |= (0x1 << 14);
		LPC_PINCON->PINSEL0 &= ~0xF00;  // pin function select
		LPC_PINCON->PINSEL0 |= 0xA00;	  // function P0.4 = 10, P0.5 = 10
	}
	
	can->MOD = 1; // reset mode
	can->IER = 0; // disable all types of interrupt (16.7.5)
	can->GSR = 0; // reset GSR.RXERR e GSR.TXERR (16.7.3)
	
	can->BTR = 0x7F0009; // 100kbps (16.7.6)
	
	can->MOD = 0; // Normal mode
	NVIC_EnableIRQ(CAN_IRQn); // Enable interrupt
	can->IER = 0x1; // receiver interrupt activated;
}

/** 
	@Brief 	    : Data Transmission from CAN1 module  
	@retval	    : Status of Transmission ( OK / NOT_OK )  
	@Argumemnts : Ptr to data of CAN_MSG_Type structure 
**/
int CAN::transmitFrame(uint32_t dataA, uint32_t dataB){
	uint32_t status = can->SR;
	
	uint32_t tfi, tid, tda, tdb, buffer;
	
	if(status & (0x1 << 2)){
		tfi = can->TFI1;
		tid = can->TID1;
		tda = can->TDA1;
		tdb = can->TDB1;
		buffer = 0x20;
	}else if(status & (0x1 << 10)){
		tfi = can->TFI2;
		tid = can->TID2;
		tda = can->TDA2;
		tdb = can->TDB2;
		buffer = 0x40;
	}else if(status & (0x1 << 18)){
		tfi = can->TFI3;
		tid = can->TID3;
		tda = can->TDA3;
		tdb = can->TDB3;
		buffer = 0x80;
		}else
	return NOT_OK;
	
	tfi = frame.fi;
	tid = frame.id;
	tda = dataA;
	tdb = dataB;
	
	// send command
	can->CMR = 0x1 | buffer;
		
	return OK;
} 
	
void CAN::receiveFrame(uint32_t *id, uint32_t *dataA, uint32_t *dataB){
	*id = can->RID;
	*dataA = can->RDA;
	*dataB = can->RDB;
	
	can->CMR = 0x1 << 2;
}


