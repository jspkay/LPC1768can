#include "CAN.h"

CAN CAN::can1, CAN::can2;
int CAN::x = 0;

CAN::CAN(){
	Tx_frame = 0x00080000;
	received = false;

	x++;
	can = x==1 ? LPC_CAN1 : LPC_CAN2;
	//if(x >= 3) return;
	
	if(x == 1){ // CAN 1
		LPC_SC -> PCONP |= (0x1<<13); // power on
		LPC_PINCON->PINSEL0 &= ~0xF;  // pin function select
		LPC_PINCON->PINSEL0 |= 0x5;	  // function P0.0 = 01, P0.1 = 01
		CAN::can1 = *this;
	}
	if(x == 2){ // CAN 2
		LPC_SC -> PCONP |= (0x1 << 14);
		LPC_PINCON->PINSEL0 &= ~0xF00;  // pin function select
		LPC_PINCON->PINSEL0 |= 0xA00;	  // function P0.4 = 10, P0.5 = 10
		CAN::can2 = *this;
	}
	
	can->MOD = 1; // reset mode
	can->IER = 0; // disable all types of interrupt (16.7.5)
	can->GSR = 0; // reset GSR.RXERR e GSR.TXERR (16.7.3)
	
	can->BTR = 0x7F0009; // 100kbps (16.7.6)
	
	can->MOD = 0; // Normal mode

	NVIC_EnableIRQ(CAN_IRQn); // Enable interrupt
	can->IER = 0x1; // receiver interrupt activated;
}


int CAN::transmitFrame(CAN_MSG msg){
	while(!(can->GSR & (1<<3))); // wait until at least one buffer is free

	uint32_t status = can->SR;
	uint32_t tfi, tid, tda, tdb, bufferBit;
	
	if(status & (0x1 << 2)){ // first buffer free
		tfi = can->TFI1;
		tid = can->TID1;
		tda = can->TDA1;
		tdb = can->TDB1;
		bufferBit = 0x20;
	}else if(status & (0x1 << 10)){ // second buffer free
		tfi = can->TFI2;
		tid = can->TID2;
		tda = can->TDA2;
		tdb = can->TDB2;
		bufferBit = 0x40;
	}else if(status & (0x1 << 18)){ // third buffer free
		tfi = can->TFI3;
		tid = can->TID3;
		tda = can->TDA3;
		tdb = can->TDB3;
		bufferBit = 0x80;
		}else
	return NOT_OK;
	
	tfi = Tx_frame;
	tid = msg.id;
	tda = msg.data.parts[0];
	tdb = msg.data.parts[1];
	
	// send command
	can->CMR = 0x1 | bufferBit;
		
	return OK;
} 
	
CAN_MSG CAN::receiveFrame(){
	CAN_MSG res;

	Rx_frame = can->RFS;
	res.id = can->RID;
	res.data.parts[0] = can->RDA;
	res.data.parts[1] = can->RDB;
	
	received = true;
	rx_msg = res;

	can->CMR = 0x1 << 2;
	
	return res;
}

void CAN_IRQHandler (void){
	uint32_t status = LPC_CANCR->CANRxSR; // global receive status register
	CAN_MSG msg;

	if(status & 0x100){ // CAN1 received
		msg = CAN::can1.receiveFrame();
	}else if(status & 0x200){ // CAN2 received
		msg = CAN::can2.receiveFrame();
	}

	// do stuff with msg

	// ERROR MANAGEMENT
	if ( LPC_CAN1->GSR & (1 << 6 ) ){
		/* The error count includes both TX_ERR (MSBye) and RX_ERR (LSByte) */
//			CAN1_Error_Cnt = LPC_CAN1->GSR >> 16;
	}
	if ( LPC_CAN2->GSR & (1 << 6 ) ){
		/* The error count includes both TX_ERR (MSByte) and RX_ERR (LSByte) */
	//	CAN1_Error_Cnt = LPC_CAN2->GSR >> 16;
	}
 	
}

void setup_LUT(void)
{
	uint32_t address = 0;
  uint32_t i=0;
  uint32_t CAN2_ID, CAN1_ID;
 
	/* Explicit Standard IDs */
  LPC_CANAF->SFF_sa = address;
	
	CAN1_ID = (i << 29) | (EXP_STD_ID << 16);			/** Set up the ID for CAN1 **/ 
	CAN2_ID = ((i+1) << 13) | (EXP_STD_ID << 0);	       	       /** Set up the ID for CAN2 **/ 
	LPC_CANAF_RAM -> mask[0] = CAN1_ID | CAN2_ID  ; // *((volatile uint32_t *)(LPC_CANAF_RAM_BASE + address)) = CAN1_ID | CAN2_ID;
	address += 4; 
	
	
	CAN1_ID = (i << 29) | (0x123 << 16);		 		/** Set up the ID for CAN1 **/ 
	CAN2_ID = ((i+1) << 13) | (0x125 << 0);		               /** Set up the ID for CAN2 **/ 
	LPC_CANAF_RAM -> mask[1] = CAN1_ID | CAN2_ID  ; 	
	address += 4; 
	
	
  /* Group standard IDs */
  LPC_CANAF->SFF_GRP_sa = address;

	CAN1_ID = (i << 29) | (GRP_STD_ID << 16);		   /** Set up the ID for CAN1 **/ 
	CAN2_ID = ((i+1) << 13) | (GRP_STD_ID << 0);	          /** Set up the ID for CAN2 **/ 
	LPC_CANAF_RAM -> mask[2] = CAN1_ID | CAN2_ID;		//*((volatile uint32_t *)(LPC_CANAF_RAM_BASE + address)) = CAN1_ID | CAN2_ID;

 
 /** @NOTE : Set up extended IDs as per the following example : 
						 CANx_ID = ((x-1) << 29) | (EXP_EXT_ID << 0);
						 *((volatile uint32_t *)(LPC_CANAF_RAM_BASE + address)) = CANx_ID;
						 address += 4; 																												**/  
	
	/* Explicit & Group Extended IDs have no entries */ 
  LPC_CANAF->EFF_sa = LPC_CANAF->EFF_GRP_sa =  address;

  /* End of Table */
  LPC_CANAF->ENDofTable = address;
  return;
}	

void CONFIG_CAN_FILTER_MODE (uint8_t mode)
{
	switch(mode)
	{
		case AF_OFF : 
				LPC_CANAF -> AFMR = mode ; 
				LPC_CAN1 -> MOD = LPC_CAN2 -> MOD = 1 ; 
				LPC_CAN1 -> IER = LPC_CAN2 -> IER = 0 ; 
				LPC_CAN1 -> GSR = LPC_CAN2 -> GSR = 0 ; 
					
				break ; 
		
		case AF_BYPASS : 
									
				LPC_CANAF -> AFMR = mode ;
				break ; 
	
		case AF_ON : 
		case AF_FULLCAN : 
							
				LPC_CANAF -> AFMR = AF_OFF ; 
				setup_LUT() ; 
				LPC_CANAF -> AFMR = mode ;
				break ; 
		
		default : 	break ; 
	}

}	


