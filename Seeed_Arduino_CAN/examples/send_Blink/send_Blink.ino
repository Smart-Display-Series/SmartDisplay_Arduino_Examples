// demo: CAN-BUS Shield, send data
#include <SPI.h>
#include <cppQueue.h>
#include <arduino-timer.h>
#include <SmartDisplay.h>
#include <canfestival.h>
#include <canInit.h>
#include <mcp2515_can.h>


#define CAN_2515
// #define CAN_2518FD

// Set SPI CS Pin according to your hardware

#if defined(SEEED_WIO_TERMINAL) && defined(CAN_2518FD)
// For Wio Terminal w/ MCP2518FD RPi Hat：
// Channel 0 SPI_CS Pin: BCM 8
// Channel 1 SPI_CS Pin: BCM 7
// Interupt Pin: BCM25
const int SPI_CS_PIN  = BCM8;
const int CAN_INT_PIN = BCM25;
#else

// For Arduino MCP2515 Hat:
// the cs pin of the version after v1.1 is default to D9
// v0.9b and v1.0 is default D10
const int SPI_CS_PIN = 9;
const int CAN_INT_PIN = 2;


#endif


#ifdef CAN_2518FD
#include "mcp2518fd_can.h"
mcp2518fd CAN(SPI_CS_PIN); // Set CS pin
#endif

#ifdef CAN_2515
#include "mcp2515_can.h"
mcp2515_can CAN(SPI_CS_PIN); // Set CS pin
#endif

#define NODE_ID    0x01
#define CLIENT_ID  0x7B

#define BUTTON_VOLTAGE A0
#define ADC_REF 5
#define GROVE_VCC 5    //VCC of the grove interface is normally 5v
#define FULL_ANGLE 300 //full value of the rotary angle is 300 degrees


const int ledHIGH    = 1;
const int ledLOW     = 0;
char prbuf[64];

uint16_t buttonValue = 0;

      bool scanSensor(void *)
{
  uint16_t data;
  uint8_t res;
  
  float voltage;
  int sensor_value = analogRead(BUTTON_VOLTAGE);
  voltage = (float)sensor_value*ADC_REF/1023;
float degrees = (voltage*FULL_ANGLE)/GROVE_VCC;  
mcp2515_can CAN(SPI_CS_PIN); // Set CS pin

}
void setup() {
    SERIAL_PORT_MONITOR.begin(115200);

pinMode(BUTTON_VOLTAGE, INPUT);
  pinMode(LED_BUILTIN, OUTPUT);

    while (CAN_OK != CAN.begin(CAN_500KBPS)) {             // init can bus : baudrate = 500k
        SERIAL_PORT_MONITOR.println("CAN init fail, retry...");
        delay(100);
    }
    SERIAL_PORT_MONITOR.println("CAN init ok!");
}

unsigned char stmp[8] = {ledHIGH, 1, 2, 3, ledLOW, 5, 6, 7};

void loop() {
    SERIAL_PORT_MONITOR.println("In loop");
    // send data:  id = 0x70, standard frame, data len = 8, stmp: data buf
    CAN.sendMsgBuf(0x70, 0, 8, stmp);

      if(buttonValue==1) {
    digitalWrite(LED_BUILTIN, HIGH);  // turn the LED on by making the voltage HIGH
    // digitalWrite(ROTARY_ANGLE_SENSOR, HIGH);
  }
  else if(buttonValue==0){

    digitalWrite(LED_BUILTIN, LOW);   // turn the LED off by making the voltage LOW
  }
    delay(1000);                       // send data once per second
}

/*********************************************************************************************************
    END FILE
*********************************************************************************************************/
