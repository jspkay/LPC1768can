#ifndef LOW_CAN_H
#define LOW_CAN_H

#include <LPC17xx.h>
#include <lpc17xx_can.h>

#define CAN_PRIO_OFF 0
#define CAN_PRIO_FST 1 // first second third
#define CAN_PRIO_FTS 2 // first thrid second
#define CAN_PRIO_SFT 3

inline void CAN_internalPrioMode(LPC_CAN_TypeDef*can, int value);

void CAN_bufferFrame(LPC_CAN_TypeDef*can, CAN_MSG_Type msg, int buffer);
void CAN_sendFrames(LPC_CAN_TypeDef*can);
int CAN_allTXok(LPC_CAN_TypeDef* can);

int CAN_AF_loadSTDRangelEntry(int canBus, int lowerBound, int upperBound);
void CAN_AF_disableEntry(int pos, int value);

#endif // LOW_CAN_H