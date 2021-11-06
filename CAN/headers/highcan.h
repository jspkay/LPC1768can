#ifndef HIGH_CAN_H
#define HIGH_CAN_H

#include "lpc17xx_can.h"
#include "./can.h"

#define hCAN_BUF_LENGHT 200

#define CAN_1Mbps  1000000
#define CAN_500Kbps 500000
#define CAN_250Kbps 250000
#define CAN_100Kbps 100000
#define CAN_5Kbps     5000

#define hCAN_SUCCESS 0x1337
#define hCAN_ONGOING_RECEVING 2
#define hCAN_REJECTION_ENABLED 3

#define hCAN_ERR_NO_ERR 0
#define hCAN_ERR_BUS_BLOCKED -1
#define hCAN_ERR_NO_EXISTING_BUS -2
#define hCAN_ERR_COLLISION -3
#define hCAN_ERR_MSG_TOO_LONG -4
#define hCAN_ERR_LOST_FRAME -5
#define hCAN_ERR_BUS_ERROR -6

#define hCAN_SOT (0x1 << 10)
#define hCAN_MAX_LENGHT (int) (1 << 5)
#define hCAN_ENUM (0xF << 6)
#define hCAN_FIRST_ENUM_BIT 6
#define hCAN_FIRST_ID_BIT 0
//SOT ---ENUM---- 
// 10 09 08 07 06 05 04 03 02 01 00
// --Numerazione- --------ID--------


#define hCAN_CONTENT_INTERESTED

// Received message interface
extern char hCAN_recMessage[2][hCAN_BUF_LENGHT], hCAN_recDone[2], hCAN_recID[2];
extern int hCAN_lenght[2];

// hCAN status variablse
extern int hCAN_ActiveError;

// functions
int hCAN_init(int peripheral, int speed);
void hCAN_setID(int ID);

int hCAN_sendMessage(int peripheral, char *buffer, int lenght);
int hCAN_receiveMessage(int peripheral);

int hCAN_arbitrationLost(int canBus);
int hCAN_busError(int canBus);

#endif // HIGH_CAN_H
