// demo: CAN-BUS Shield, receive data with check mode
// send data coming to fast, such as less than 10ms, you can use this way
// loovee, 2014-6-13

// Documentazione: https://github.com/Seeed-Studio/Seeed_Arduino_CAN

#include <SPI.h>
#include "mcp_can.h"
#include "mcp2515_can.h"

#define CAN_2515

const int SPI_CS_PIN = 9;
const int CAN_INT_PIN = 2;

/* Connessioni (Come CAN-BUS Shield V2):
 *  
 *  MCP_2515    |     Arduino
 *  
 *  Vcc         ->    5V
 *  GND         ->    GND
 *  CS          ->    9
 *  (MO)SI      ->    11
 *  (MI)SO      ->    12
 *  SCK         ->    13
 *  INT         ->    2
 *  
 */

mcp2515_can CAN(SPI_CS_PIN); // Set CS pin

void setup() {
    SERIAL_PORT_MONITOR.begin(115200);

    while (CAN_OK != CAN.begin(CAN_100KBPS, MCP_8MHz)) {             // init can bus : baudrate = 100k
        SERIAL_PORT_MONITOR.println("CAN init fail, retry...");
        delay(100);
    }

    SERIAL_PORT_MONITOR.println("CAN init ok!");
}


void loop() {
    unsigned char len = 0;
    unsigned char buf[8];

    if (CAN_MSGAVAIL == CAN.checkReceive()) {         // check if data coming
        CAN.readMsgBuf(&len, buf);    // read data,  len: data length, buf: data buf

        unsigned long canId = CAN.getCanId();

        SERIAL_PORT_MONITOR.println("-----------------------------");
        SERIAL_PORT_MONITOR.print("get data from ID: 0x");
        SERIAL_PORT_MONITOR.println(canId, HEX);

        for (int i = 0; i < len; i++) { // print the data
            SERIAL_PORT_MONITOR.print(buf[i],HEX);
            //SERIAL_PORT_MONITOR.print(" ");
        }
        SERIAL_PORT_MONITOR.println();
    }
}

//END FILE
