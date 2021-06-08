#include "./can.h"
#include "../GLCD/GLCD.h"

#define BUFFER1 1
#define BUFFER2 2
static uint8_t CAN_state;

void CAN_internalPrioMode(LPC_CAN_TypeDef*can, int mode){
	if(mode == CAN_PRIO_OFF) can->MOD &= ~CAN_MOD_TPM;
	else{
		can->TFI1 &= ~0xFF;
		can->TFI2 &= ~0xFF;
		can->TFI3 &= ~0xFF;
		can->MOD |= CAN_MOD_TPM;
		switch(mode){
			case CAN_PRIO_FST:
				can->TFI1 |= 0;
				can->TFI2 |= 1;
				can->TFI3 |= 2;
				break;    
			case CAN_PRIO_FTS:   
				can->TFI1 |= 0;
				can->TFI2 |= 2;
				can->TFI3 |= 1;
				break;
			case CAN_PRIO_SFT:   
				can->TFI1 |= 1;
				can->TFI2 |= 0;
				can->TFI3 |= 2;
				break;
		}
	}
}	

// TODO: check if transmission is possibile (i.e. periphera is ready)
void CAN_bufferFrame(LPC_CAN_TypeDef *can, CAN_MSG_Type msg, int buffer){
	uint32_t data;
	if(buffer == 1){
		while( (can->SR & CAN_SR_TBS1) == 0); // wait until buffer is ready
		can->TID1 = msg.id;
		can->TFI1 &= ~(0xFF << 16); // no RTR, std format (11bits)
		can->TFI1 |= msg.len << 16;
		can->TDA1 = msg.dataA.number;
		can->TDB1 = msg.dataB.number;
		CAN_state |= BUFFER1;
	}else{
		while( (can->SR & CAN_SR_TBS2) == 0); // wait until buffer is ready
		can->TID2 = msg.id;
		can->TFI2 &= ~(0xFF << 16);
		can->TFI2 |= msg.len << 16;
		can->TDA2 = msg.dataA.number;
		can->TDB2 = msg.dataB.number;
		CAN_state |= BUFFER2;
	}
}

static unsigned char counter[2] = "1";
// TODO: check if transmission is possibile (i.e. peripheral is ready)
void CAN_sendFrames(LPC_CAN_TypeDef*can){
	int cmd = 0;
	
	while( (can->GSR & CAN_GSR_TCS) == 0); // wait for can ready to send
	
	// send only prepared frames
	if( (CAN_state & BUFFER1) == BUFFER1) cmd |= CAN_CMR_STB1; // transmit 1
	if( (CAN_state & BUFFER2) == BUFFER2) cmd |= CAN_CMR_STB2; // transmit 2
	
	cmd |= CAN_CMR_TR;
	can->CMR |= cmd;
	
	CAN_state = 0;
	
	//GUI_Text(0, 20 * (counter[0]++-'0'), counter, White, Blue);
}
inline int CAN_allTXok(LPC_CAN_TypeDef* can){
	return can->GSR | CAN_GSR_TCS;
}

static int tableLen = 0;
int CAN_AF_loadSTDRangelEntry(int canBus, int lowerBound, int upperBound){
	
	LPC_CANAF->AFMR = 1; // CAN receive completely disabled
	LPC_CANAF_RAM->mask[tableLen] = (canBus-1) << 29 | lowerBound << 16
																 |(canBus-1) << 13 | upperBound;
	
	LPC_CANAF->SFF_sa 		= 0;
	LPC_CANAF->SFF_GRP_sa = 0;
	
	int inc = 1 << 2;
	LPC_CANAF->EFF_sa 		+= inc;
	LPC_CANAF->EFF_GRP_sa += inc;
	LPC_CANAF->ENDofTable += inc;
	
	LPC_CANAF->AFMR = 0; // operating mode enabled.
	
	return tableLen++;
}

#define CAN_AF_bitsEnable 1 << 28 | 1 << 12
void CAN_AF_disableEntry(int pos, int value){
	if(value) LPC_CANAF_RAM->mask[pos] |= CAN_AF_bitsEnable; // entry is disabled
	else LPC_CANAF_RAM->mask[pos] &= ~(CAN_AF_bitsEnable); // entry works
}

