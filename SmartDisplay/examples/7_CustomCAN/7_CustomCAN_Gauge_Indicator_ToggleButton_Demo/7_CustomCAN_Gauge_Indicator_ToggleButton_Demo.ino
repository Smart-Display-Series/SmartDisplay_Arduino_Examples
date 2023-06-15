
#include <SPI.h>
#include "mcp2515_can.h"

#define VR_Sensor A0
#define Button_Sensor A1

/*SAMD core*/
#ifdef ARDUINO_SAMD_VARIANT_COMPLIANCE
    #define SERIAL SerialUSB
#else
    #define SERIAL Serial
#endif

#define ADC_REF 5      //reference voltage of ADC is 5v.If the Vcc switch on the seeeduino
                       //board switches to 3V3, the ADC_REF should be 3.3
#define GROVE_VCC 5    //VCC of the grove interface is normally 5v
#define FULL_ANGLE 300 //full value of the rotary angle is 300 degrees

const int SPI_CS_PIN = 9;
mcp2515_can CAN(SPI_CS_PIN); // Set CS pin

uint16_t IndicatorValue = 0;       //Indicator value store
uint16_t ToggleButtonValue = 0;    //Toggle Button Sensor value store
uint16_t Button_SensorValue = 0;   //Physical Button Sensor value store
uint16_t VR_SensorValue = 0;       //Variable Resistor Sensor value store
uint16_t GaugeValue = 0;           //Gauge value store
float    Voltage = 0;              //Voltage value store

void setup() {
SERIAL.begin(115200);
while(!Serial){};

pinMode(13, OUTPUT);                                   // init LED at pin 13 as output
pinMode(12, OUTPUT);                                   // init LED at pin 12 as output
pinMode(VR_Sensor, INPUT);                             // init Variable Resistor Sensor as input
pinMode(Button_Sensor, INPUT);                         // init Physical Button Sensoras as input

while (CAN_OK != CAN.begin(CAN_250KBPS)) {             // init can bus : baudrate = 250k
SERIAL.println("CAN BUS Shield init fail");
SERIAL.println(" Init CAN BUS Shield again");
delay(100);
}
SERIAL.println("CAN BUS Shield init ok!");
}

unsigned char stmp[8]  = {0, 0, 0, 0, 100, 0, 0, 0};  //Gauge (0x702)array data Bytes
unsigned char stmp1[8] = {1, 0, 0, 0, 0, 0, 0, 0};    //Indicator (0x70C)array data Bytes
unsigned char stmp2[8] = {0, 2, 0, 0, 0, 0, 0, 0};    //ToggleButton (0x705)array data Bytes

void loop() {
VR_SensorValue = analogRead(VR_Sensor);
Voltage        = (float)VR_SensorValue*ADC_REF/1023;
float degrees  = (Voltage*FULL_ANGLE)/GROVE_VCC;
GaugeValue     = map( degrees, 0, 300, 0, 255);      /*Here gauge rotation angle is set. It could be controlled according to the type of gauge.*/

if(GaugeValue > 190) { 
digitalWrite(13, HIGH);  /*pin 13 turn the LED on by making the voltage HIGH*/
}
else {
digitalWrite(13, LOW);   /*pin 13 turn the LED off by making the voltage LOW*/
}

Button_SensorValue = digitalRead(Button_Sensor);
if ( (Button_SensorValue | ToggleButtonValue) == HIGH ) {
digitalWrite(12, HIGH);  /*pin 12 turn the LED on by making the voltage HIGH*/
IndicatorValue = 1;      /*turn the indicator turn on by making the value 1*/
}
else {
digitalWrite(12, LOW);   /*pin 12 turn the LED off by making the voltage LOW*/
IndicatorValue = 0;      /*turn the indicator turn off by making the value 0*/
}

  stmp[4]  = GaugeValue;
  stmp1[0] = IndicatorValue;

  CAN.sendMsgBuf(0x702, 0, 8, stmp);     // send data of the Gauge: id = 0x702, byte ext, data len = 8, stmp: data buf
  CAN.sendMsgBuf(0x70C, 0, 8, stmp1);    // send data of the Indicator: id = 0x70C, byte ext, data len = 8, stmp1: data buf

  unsigned char len = 2;     // Set the Length of the message to read for ToggleButton
  unsigned char buf[8];      // Declare a buffer array to store the message data
  unsigned long canId;       // Set the ID of the message to read
  byte rtr = 0;

// Check for incoming messages
if (CAN.checkReceive()) {
    CAN.readMsgBuf(&len, buf);                 // read data of the id: data len, data buf
    unsigned long canId = CAN.getCanId();

    if(canId==0x705)                          // read data of the Toggle Button: id = 0x705, data len = 8, stmp2: data buf
    {
      Serial.print("123 Get data from ID: ");
      Serial.println(canId, HEX);

      if(buf[0]==0x01)
        ToggleButtonValue = 1;
      else
        ToggleButtonValue = 0;

      for(int i = 0; i<len; i++)
      { // print the data
        Serial.print(buf[i], HEX);
        Serial.print("\t");
        Serial.println();
      }
      delay(100); 
    }
  }
}




