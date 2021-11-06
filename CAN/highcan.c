#include <lpc17xx_can.h>
#include <stdio.h>
#include "../GLCD/GLCD.h"
#include "./highcan.h"

static int ID[2], busBlocked[2] = {0, 0};
volatile int busMine[2];

int hCAN_ActiveError;

/*void printTextOnDisplay(int x, int y, CAN_MSG_Type* msg1){
		unsigned char buff[100];
		union {
			uint32_t n[2];
			char string[9];
		} m;
		m.n[0] = msg1->dataA.number;
		m.n[1] = msg1->dataB.number;
		m.string[8] = 0;
		//sprintf((char*) buff, "0x%x - %s", msg1->id, m.string);
		GUI_Text(x, y, (uint8_t*) m.string, White, Black);
}*/

static int hCAN_AfEntry[2];
int hCAN_init(int peripheral, int speed){
	LPC_CAN_TypeDef *can;
	if(peripheral == 1) can = LPC_CAN1;
	else if(peripheral == 2) can = LPC_CAN2;
	else return ERROR;
	
	CAN_Init(can, speed);
	
	can->MOD = 1; // reset mode to set priority
	CAN_internalPrioMode(can, CAN_PRIO_FST);
	can->MOD &= ~0x1; // normal mode
	
	// Internal varialbes
	
	
	//Enable Interrupt (UM10360 16.7.4)
	CAN_IRQCmd(can, CANINT_RIE, ENABLE); // receive message 
	CAN_IRQCmd(can, CANINT_ALIE, ENABLE); // arbitration lost
	//CAN_IRQCmd(can, CANINT_EPIE, ENABLE); // Error passive interrupt
	CAN_IRQCmd(can, CANINT_BEIE, ENABLE); // Buss Error interrupt

	NVIC_EnableIRQ(CAN_IRQn); // enable interrupt
	
	NVIC_SetPriority(CAN_IRQn, 0);
	
	// Setup AF
	//CAN_SetAFMode(LPC_CANAF, CAN_AccBP);
	int l = CAN_AF_loadSTDRangelEntry(peripheral, 0x0, 0x3F); // accept only last frame
	CAN_AF_disableEntry(l, FALSE);
	CAN_SetAFMode(LPC_CANAF, CAN_AccBP);
	
	//hCAN_AfEntry[peripheral-1] = CAN_AF_loadSTDRangelEntry(peripheral, 0x0, 0x7FF); // accept everything
	//CAN_AF_disableEntry(hCAN_AfEntry[peripheral-1], FALSE);
}

void hCAN_setID(int newid){
		ID[0] = newid << hCAN_FIRST_ID_BIT;
		ID[1] = newid << hCAN_FIRST_ID_BIT;
}

int hCAN_sendMessage(int canBus, char *buf, int lenght){
	if(busBlocked[canBus-1]) return hCAN_ERR_BUS_BLOCKED;
	
	LPC_CAN_TypeDef * can;
	if(canBus == 1) can = LPC_CAN1;
	else if(canBus == 2) can = LPC_CAN2;
	else return hCAN_ERR_NO_EXISTING_BUS;
	
	if(lenght <= 8){
		CAN_MSG_Type msg;
		msg.id = ID[canBus-1] | hCAN_SOT;
		msg.len = lenght;
		
		CAN_bufferFrame(can, msg, 1);
		CAN_sendFrames(can);
		
		while( 
			CAN_isTransmitting(can)	 // wait until either the messages are sent or interrupt is issued
			&& hCAN_ActiveError == hCAN_ERR_NO_ERR // or an error occours
		);
			
		if( hCAN_ActiveError != hCAN_ERR_NO_ERR)
			return hCAN_ActiveError;
		return hCAN_SUCCESS;
	}
	
	int frames = lenght/8;
	if( frames > hCAN_MAX_LENGHT ) return hCAN_ERR_MSG_TOO_LONG;
	
	if(lenght % 8 != 0) frames++;
	// Send in two frames per time
	int packets = frames / 2; // byte(lenght)/8 is the number of frames, so packets is the number of couple of frames to be sent
	if(frames % 2 != 0) packets++;
	
	CAN_MSG_Type msg1, msg2;
	
	msg1.id = ID[canBus-1] | hCAN_SOT | --frames << hCAN_FIRST_ENUM_BIT;
	msg2.id = ID[canBus-1] | --frames << hCAN_FIRST_ENUM_BIT;
	msg1.len = 8;
	msg2.len = lenght <= 16 ? lenght-8 : 8;
	
	int bufIndex = 0;
	// Send first two frames to get propriety
	busMine[canBus-1] = 1;
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
	while( 
		 hCAN_ActiveError == hCAN_ERR_NO_ERR // or an error occours
	); 
	
	if(hCAN_ActiveError == hCAN_ERR_BUS_ERROR ) return hCAN_ActiveError;
	if(hCAN_ActiveError == hCAN_ERR_COLLISION){ // busMine goes to 0 if interrupt handler for transmission problem is called
		busBlocked[canBus-1] = 1;
		busMine[canBus-1] = 0;
		return hCAN_ERR_COLLISION;
	}
	
	if(lenght <= 16) return hCAN_SUCCESS;
	
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
		msg1.id = ID[canBus-1] | --frames << hCAN_FIRST_ENUM_BIT;
		msg2.id = ID[canBus-1] | --frames << hCAN_FIRST_ENUM_BIT;
		
		// prepare buffer
		CAN_bufferFrame(can, msg1, 1);
		CAN_bufferFrame(can, msg2, 2);
		
		// send
		CAN_sendFrames(can);
		
		while( 
			CAN_isTransmitting(can)	 // wait until either the messages are sent or interrupt is issued
			&& hCAN_ActiveError == hCAN_ERR_NO_ERR // or an error occours
		); 
	}
	
	// send last packet
	msg1.id = ID[canBus-1] | --frames << hCAN_FIRST_ENUM_BIT;
	msg2.id = ID[canBus-1] | --frames << hCAN_FIRST_ENUM_BIT; // may be negative. In that case it's not sent
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
	
	while(
		CAN_isTransmitting(can)	 // wait until either the messages are sent or interrupt is issued
		&& hCAN_ActiveError == hCAN_ERR_NO_ERR // or an error occours
	);
	
	busBlocked[canBus-1] = 0;
	busMine[canBus-1] = 0;
	
	return hCAN_SUCCESS;
}

char hCAN_recMessage[2][hCAN_BUF_LENGHT], hCAN_recID[2], hCAN_recDone[2];
int hCAN_lenght[2];

int i = 0;

#ifdef DEBUG
	int counter = 0;
#endif

static inline void putMessageInBuffer(CAN_MSG_Type* msg, int canBus);
static enum {IDLE, RECEIVING} recStatus = IDLE;
static int recNext, buffIndex;
int hCAN_receiveMessage(int canBus){
	CAN_MSG_Type msg1;
	
	int res;
	
	LPC_CAN_TypeDef * can;
	if(canBus == 1) can = LPC_CAN1;
	else if (canBus == 2) can = LPC_CAN2;
	else return hCAN_ERR_NO_EXISTING_BUS;
	
	CAN_ReceiveMsg(can, &msg1);
	
	#ifdef DEBUG
	//printTextOnDisplay(0, 20*counter++, &msg1);
	#endif
	
	#ifdef hCAN_CONTENT_INTERESTED // The message will be completely read
	
	// if it first frame
	if(recStatus == IDLE){
		if( (msg1.id & hCAN_SOT) == hCAN_SOT ){
			busBlocked[canBus-1] = 1;
			hCAN_recDone[canBus-1] = 0;
			recNext = ((msg1.id & hCAN_ENUM) >> hCAN_FIRST_ENUM_BIT) - 1;
			hCAN_lenght[canBus-1] = msg1.len;
			
			buffIndex = 0;
			putMessageInBuffer(&msg1, canBus);
			
			recStatus = RECEIVING;
			
			res = hCAN_ONGOING_RECEVING;
		}
		if( (msg1.id & hCAN_ENUM) == 0){ // last frame
			busBlocked[canBus-1] = 0;
			res = hCAN_SUCCESS;
			recStatus = IDLE;
			hCAN_recDone[canBus-1] = 1;
		} else { // lost connection
			busBlocked[canBus-1] = 1;
			res = hCAN_ERR_LOST_FRAME;
		}
	}
	else if(recStatus == RECEIVING){
		if( (msg1.id & hCAN_ENUM) == recNext << hCAN_FIRST_ENUM_BIT ){
			hCAN_lenght[canBus-1] += msg1.len;
			if(recNext == 0){
				putMessageInBuffer(&msg1, canBus-1);
				
				busBlocked[canBus-1] = 0;
				hCAN_recDone[canBus-1] = 1;
				res = hCAN_SUCCESS;
				recStatus = IDLE;
			}
			else{
				recNext--;
				putMessageInBuffer(&msg1, canBus);
				res = hCAN_ONGOING_RECEVING;
			}							
		}
		else{
			if((msg1.id & hCAN_ENUM) == 0){
				busBlocked[canBus-1] = 0;
				hCAN_recDone[canBus-1] = 1;
				res = hCAN_ERR_LOST_FRAME;
			}
			recStatus = IDLE;
		}
	}
	
	if(res == hCAN_SUCCESS) hCAN_recID[canBus-1] = msg1.id;
	return res;
	#else // only the first and last frames are read
	
	if( (msg1.id & hCAN_ENUM) != 0 ){ // if it is not the last frame
		//CAN_AF_disableEntry(hCAN_AfEntry[canBus-1], TRUE); // enable rejection
		LPC_CANAF->AFMR = 0;
		#ifdef DEBUG
		GUI_Text(0, counter++*20, (uint8_t *) "Disabled", White, Blue);
		#endif
		return hCAN_REJECTION_ENABLED;
	}
	// the next time we'll receive the last one
	// so we disable the rejection again
	//CAN_AF_disableEntry(hCAN_AfEntry[canBus-1], FALSE);
	CAN_SetAFMode(LPC_CANAF, CAN_AFMR_AccBP);
	return hCAN_SUCCESS;
	
	#endif
	
}

inline void putMessageInBuffer(CAN_MSG_Type* msg, int canBus){
	int l = msg->len;
	if(l == 8){
			for(int i=0; i<4; i++){
				hCAN_recMessage[canBus-1][buffIndex+i] = msg->dataA.string[i];
				hCAN_recMessage[canBus-1][buffIndex+i+4] = msg->dataB.string[i];
			}
			buffIndex += 8;
	}
	else{ // l < 8
		if(l > 4){
			for(int i=0; i<4; i++){
				hCAN_recMessage[canBus-1][buffIndex+i] = msg->dataA.string[i];
			}
			for(int i=0; i<l-4; i++){
				hCAN_recMessage[canBus-1][buffIndex+i] = msg->dataB.string[i];
			}
		} // end if l > 4;
		else{
			for(int i=0; i<l; i++){
				hCAN_recMessage[canBus-1][buffIndex+i] = msg->dataA.string[i];
			}
		}
		buffIndex += l;
	} // else l < 8
}


inline int hCAN_arbitrationLost(int canBus){
	LPC_CAN_TypeDef *can;
	if(canBus == 1) can = LPC_CAN1;
	else can = LPC_CAN2;
	
	if( (can->ICR & CAN_ICR_ALI) != 0){
		busMine[canBus-1] = 0;
		return 1;
	}
	return 0;
}

inline int hCAN_busError(int canBus){
		LPC_CAN_TypeDef *can;
	if(canBus == 1) can = LPC_CAN1;
	else can = LPC_CAN2;
	
	if( (can->ICR & CAN_ICR_ALI) != 0)
		return 1;
	return 0;
}