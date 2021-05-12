#include <lpc17xx_can.h>
#include "./highcan.h"

static int busBlocked = 0, ID;
volatile int busMine;

extern char hCAN_recMessage[];
extern int hCAN_lenght;

int hCAN_init(int peripheral, int speed){
	LPC_CAN_TypeDef *can;
	if(peripheral == 1) can = LPC_CAN1;
	else if(peripheral == 2) can = LPC_CAN2;
	else return ERROR;
	
	CAN_Init(can, speed);
	
	can->MOD = 1; // reset mode to set priority
	CAN_internalPrioMode(can, CAN_PRIO_FST);
	can->MOD &= ~0x1; // normal mode
	
	//Enable Interrupt
	//CAN_IRQCmd(LPC_CAN1, CANINT_RIE, ENABLE);
	//CAN_IRQCmd(LPC_CAN1, CANINT_TIE1, ENABLE);

	//NVIC_EnableIRQ(CAN_IRQn); // enable interrupt
	CAN_SetAFMode(LPC_CANAF,CAN_AccBP); // Acceptance filter
}

void hCAN_setID(int newid){
		ID = newid << 5;
	// 10 09 08 07 06 05 04 03 02 01 00
	// -------ID-------- --Numerazione--
}

int hCAN_sendMessage(int canBus, char *buf, int lenght){
	if(busBlocked) return hCAN_ERR_BUS_BLOCKED;
	
	LPC_CAN_TypeDef * can;
	if(canBus == 1) can = LPC_CAN1;
	else if(canBus == 2) can = LPC_CAN2;
	else return hCAN_ERR_NO_EXISTING_BUS;
	
	int frames = lenght/8;
	if( frames > hCAN_MAX_LENGHT ) return hCAN_ERR_MSG_TOO_LONG;
	
	// TODO: check if the transmission can be completed with a single packet
	
	
	if(lenght % 8 != 0) frames++;
	// Send in two frames per time
	int packets = frames / 2; // byte(lenght)/8 is the number of frames, so packets is the number of couple of frames to be sent
	if(frames % 2 != 0) packets++;
	
	CAN_MSG_Type msg1, msg2;
	msg1.id = ID | hCAN_SOT | --frames;
	msg2.id = ID | --frames;
	msg1.len = 8;
	msg2.len = 8;
	
	int bufIndex = 0;
	// Send first two frames to get propriety
	busMine = 1;
	for(int ii=0; ii<4; ii++){
			msg1.dataA.string[ii] = buf[ii];
			msg1.dataB.string[ii] = buf[ii+4];
			msg2.dataA.string[ii] = buf[ii+8];
			msg2.dataB.string[ii] = buf[ii+12];
	}
	CAN_bufferFrame(can, msg1, 1);
	CAN_bufferFrame(can, msg2, 2);
	bufIndex += 16;
	CAN_sendFrames(can); // send messages
		
	// processor stays in while until transsion is put in idle
	while( (can->GSR & CAN_GSR_TS) == CAN_GSR_TS ); // wait until either the messages are sent or interrupt is issued
		
	if(!busMine){ // busMine goes to 0 if interrupt handler for transmission problem is called
		busBlocked = 1;
		return hCAN_ERR_COLLISION;
	}
	
	packets--; // we already sent the firt packet
	for(int i=0; i<packets-1; i++){ // send normal packets
		// prepare frames
		for(int ii = 0; ii<4; ii++){
			msg1.dataA.string[ii] = buf[bufIndex+ii];
			msg1.dataB.string[ii] = buf[bufIndex+ii+4];
			msg2.dataA.string[ii] = buf[bufIndex+ii+8];
			msg2.dataB.string[ii] = buf[bufIndex+ii+12];
		}
		bufIndex+=16;
		msg1.id = ID | --frames;
		msg2.id = ID | --frames;
		
		// prepare buffer
		CAN_bufferFrame(can, msg1, 1);
		CAN_bufferFrame(can, msg2, 2);
		
		// send
		CAN_sendFrames(can);
	}
	
	// send last packet
	
	msg1.id = ID | --frames;
	msg2.id = ID | --frames; // may be negative. In that case it's not sent
	// assert(lenght-bufIndex == lenght%16)
	int remaining = lenght-bufIndex;
	char *txBuffers[4] = {msg1.dataA.string, msg1.dataB.string, msg2.dataA.string, msg2.dataB.string};
	int i = 0, j;
	while(bufIndex < lenght){
		for(j=0; j<4 && bufIndex<lenght; j++) txBuffers[i][j] = buf[bufIndex++];
		i++;
	}
	
	if(remaining > 8){
		msg1.len = 8;
		remaining -= 8;
		msg2.len = remaining;
		CAN_bufferFrame(can, msg2, 2);
	}
	else msg1.len = remaining;
	CAN_bufferFrame(can, msg1, 1);
	
	CAN_sendFrames(can);
	
	while(!CAN_allTXok(can));
	
	busBlocked = 0;
	busMine = 0;
	
	return hCAN_SUCCESS;
}

char hCAN_recMessage[hCAN_BUF_LENGHT], hCAN_recDone;
int hCAN_lenght;

static inline void putMessageInBuffer(CAN_MSG_Type* msg);
static enum {IDLE, RECEIVING} recStatus = 0;
static int recNext, buffIndex;
int hCAN_receiveMessage(int peripheral){
	CAN_MSG_Type msg1;
	
	int res;
	
	LPC_CAN_TypeDef * can;
	if(peripheral == 1) can = LPC_CAN1;
	else if (peripheral == 2) can = LPC_CAN2;
	else return hCAN_ERR_NO_EXISTING_BUS;
	
	CAN_ReceiveMsg(can, &msg1);
	
	#ifdef hCAN_CONTENT_INTERESTED // The message will be completely read
	
	// if it first frame
	if(recStatus == IDLE){
		if( (msg1.id & hCAN_SOT) == hCAN_SOT ){
			busBlocked = 1;
			hCAN_recDone = 0;
			hCAN_lenght = msg1.id & hCAN_ENUM;
			recNext = hCAN_lenght-1;
			
			buffIndex = 0;
			putMessageInBuffer(&msg1);
			
			recStatus = RECEIVING;
			
			res = hCAN_ONGOING_RECEVING;
		}
		else if( (msg1.id & hCAN_ENUM) == 0){ // last frame
			busBlocked = 0;
			res = hCAN_SUCCESS;
		} else { // lost connection
			busBlocked = 1;
			res = hCAN_ERR_LOST_FRAME;
		}
	}
	
	if(recStatus == RECEIVING){
		if( (msg1.id & hCAN_ENUM) == recNext ){
			if(recNext == 0){
				putMessageInBuffer(&msg1);
				
				busBlocked = 0;
				hCAN_recDone = 1;
				res = hCAN_SUCCESS;
				recStatus = IDLE;
			}
			else{
				recNext--;
				putMessageInBuffer(&msg1);
				res = hCAN_ONGOING_RECEVING;
			}							
		}
		else{
			if((msg1.id & hCAN_ENUM) == 0){
				busBlocked = 0;
				hCAN_recDone = 1;
				res = hCAN_ERR_LOST_FRAME;
			}
			recStatus = IDLE;
		}
	}
	
	return res;
	#else // only the first and last frames are read
	
	// TODO:
	// - if enumeration of frame is not 0, then we ignore all the frames in between
	// - - to do this, we simply put the AF so that it will accept only 00000 termining
	// - - frames.
	// - if the received packet has 00000 termining ID, then we remove filter on AF and 
	// - - set busBlocked to false
	
	#endif
	
}

static inline void putMessageInBuffer(CAN_MSG_Type* msg){
	int l = msg->len;
	if(l == 8){
			for(int i=0; i<4; i++){
				hCAN_recMessage[buffIndex+i] = msg->dataA.string[i];
				hCAN_recMessage[buffIndex+i+4] = msg->dataB.string[i];
			}
			buffIndex += 8;
	}
	else{ // l < 8
		if(l > 4){
			for(int i=0; i<4; i++){
				hCAN_recMessage[buffIndex+i] = msg->dataA.string[i];
			}
			for(int i=0; i<l-4; i++){
				hCAN_recMessage[buffIndex+i] = msg->dataB.string[i];
			}
		} // end if l > 4;
		else{
			for(int i=0; i<l; i++){
				hCAN_recMessage[buffIndex+i] = msg->dataA.string[i];
			}
		}
		buffIndex += l;
	} // else l < 8
}
