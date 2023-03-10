#include <cppQueue.h>

// demo: CAN-BUS Shield, send data
// loovee@seeed.cc
#include <SPI.h>
#include <can-serial.h>
#include <mcp2515_can.h>
#include <mcp2515_can_dfs.h>
#include <mcp2518fd_can.h>
#include <mcp2518fd_can_dfs.h>
#include <mcp_can.h>
#include <math.h>

#define ROTARY_ANGLE_SENSOR A0
#define ADC_REF 5      //reference voltage of ADC is 5v.If the Vcc switch on the seeeduino
                       //board switches to 3V3, the ADC_REF should be 3.3
#define GROVE_VCC 5    //VCC of the grove interface is normally 5v
#define FULL_ANGLE 100 //full value of the rotary angle is 300 degrees
#define CAN_2515
#define ON  1
#define OFF 0
#define	MAX_LENGTH		100

// Set SPI CS Pin according to your hardware

#if defined(SEEED_WIO_TERMINAL) && defined(CAN_2518FD)
// For Wio Terminal w/ MCP2518FD RPi Hat
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

const int B = 4275;               // B value of the thermistor
const int R0 = 100000;            // R0 = 100k
const int pinTempSensor = SCL;     // Grove - Temperature Sensor connect to SCL
//const int vibrationSensor = SCL;
const int Buzzer = 6;
unsigned long startMillis;  //some global variables available anywhere in the program
unsigned long currentMillis;
const unsigned long period = 100;
float lastDegrees;
float lastTemp;
int present_condition = 0;
int previous_condition = 0;
unsigned char flagRecv = 0;
unsigned char guage[8] = {0, 0, 0, 0, 0, 0, 0, 0};
unsigned char temp[8] = {0, 0, 0, 0, 0, 0, 0, 0};

unsigned char button[8];

unsigned char len = 0;

typedef struct {
  
  unsigned long id;
  unsigned char toggle[8];
  
}CanBusPacket_t;

CanBusPacket_t canBusPacket;
CanBusPacket_t canBusPacketTest;
unsigned char cnt = 0;
cppQueue	q(sizeof(CanBusPacket_t), MAX_LENGTH, FIFO, true);


void setup() {
  
  SERIAL_PORT_MONITOR.begin(115200);
  pinMode(ROTARY_ANGLE_SENSOR, INPUT);
  //pinMode(vibrationSensor, INPUT);
  pinMode(pinTempSensor, INPUT);
  pinMode(Buzzer, OUTPUT);

  while(!Serial){};
  attachInterrupt(digitalPinToInterrupt(CAN_INT_PIN), MCP2515_ISR, FALLING); // start interrupt

  while (CAN_OK != CAN.begin(CAN_250KBPS)) {             // init can bus : baudrate = 500k
      SERIAL_PORT_MONITOR.println("CAN init fail, retry...");
      delay(100);
  }
  SERIAL_PORT_MONITOR.println("CAN init ok!");
  startMillis = millis();  //initial start time
  CAN.init_Mask(0, 0, 0x7ff);                         // there are 2 mask in mcp2515, you need to set both of them
  CAN.init_Mask(1, 0, 0x7ff);
  CAN.init_Filt(0, 0, 0x601);                          // there are 6 filter in mcp2515
}

void MCP2515_ISR() {

  if (CAN_MSGAVAIL == CAN.checkReceive()) { 

    unsigned char toggle[8];
    
    CAN.readMsgBuf(&len, toggle);

    canBusPacket.id = CAN.getCanId();

    canBusPacket.toggle[0] = toggle[0];
    canBusPacket.toggle[1] = toggle[1];
    canBusPacket.toggle[2] = toggle[2];
    canBusPacket.toggle[3] = toggle[3];
    canBusPacket.toggle[4] = toggle[4];
    canBusPacket.toggle[5] = toggle[5];
    canBusPacket.toggle[6] = toggle[6];
    canBusPacket.toggle[7] = toggle[7];
            
    q.push(&canBusPacket);          // Push char to cppQueue
  }
}

void loop() {
  
  // send data:  id = 0x00, standrad frame, data len = 8, stmp: data buf
  float voltage;
  float temperature;
  float degrees;
  int sensor_value;
  int a;
  float R;
  int present_graph;
  currentMillis = millis();
  previous_condition = present_condition;
  

  if ((currentMillis - startMillis) >= period) {
    
    sensor_value = analogRead(ROTARY_ANGLE_SENSOR);
    voltage = (float)sensor_value*ADC_REF/1023;
    degrees = 100 - ((voltage*FULL_ANGLE)/GROVE_VCC);
    
    a = analogRead(pinTempSensor);
    R = 1023.0/a-1.0;
    R = R0*R;
    temperature = 1.0/(log(R/R0)/B+1/298.15)-273.15; // convert to temperature via datasheet
    
    // present_condition = 1- (digitalRead(vibrationSensor));
    // present_graph = 100 * present_condition;

    startMillis = currentMillis;
    unsigned char guage[8] = {0x2B, 0x02, 0x20, 0x07, degrees, 0x00, 0x00, 0x00};
    unsigned char temp[8] = {0x2B, 0x00, 0x20, 0x07, temperature, 0x00, 0x00, 0x00};
    //unsigned char indicator[8] = {0x2B, 0x05, 0x20, 0x07, present_graph, 0x00, 0x00, 0x00};
    
    if(lastDegrees != degrees){
      CAN.sendMsgBuf(0x67B, 0, 8, guage);
      lastDegrees = degrees;
    }
    // if (previous_condition != present_condition) {
    //   CAN.sendMsgBuf(0x67B, 0, 8, indicator);
    //   previous_condition = present_condition;
    // }
    if(lastTemp != temperature){
      CAN.sendMsgBuf(0x67B, 0, 8, temp);
      lastTemp = temperature;
    }
  }

  if (!q.isEmpty()) {               // Only if q is not empty
  
    q.pop(&canBusPacketTest);    
    
    if ( canBusPacketTest.toggle[4] == 0x01 && 
         canBusPacketTest.toggle[0] == 0x2B && 
         canBusPacketTest.toggle[1]==0x04 && 
         canBusPacketTest.toggle[2]==0x20 && 
         canBusPacketTest.toggle[3]==0x08) {
              
      digitalWrite(6, HIGH);
      delay(5); 
      digitalWrite(6, LOW);
      delay(5);
    }
      
    //   for(int i = 0; i<len; i++) {   // print the data
    //     Serial.print(canBusPacketTest[i], HEX);
    //     Serial.print("\t");
    //   }
   
    // if ( button[4] == 0x01 && button[0] == 0x2B && button[1]==0x03 && button[2]==0x20 && button[3]==0x07) {
    //   digitalWrite(13, ON);
    //   delay(1000); 
    //   digitalWrite(13, OFF);
    //   delay(1000);
    //   for(int i = 0; i<len; i++) {   // print the data
    //     Serial.print(button[i], HEX);
    //     Serial.print("\t");
    //   }
    // }
    // Serial.print("\n");
  }
          
}

// END FILE
