
#include <ModbusMaster.h>

#define SLAVE_ADDRESS 0x7B
// #define BRIGHTNESS_REGISTER_ADDRESS 2111
#define MODBUS_REGISTER_PAGE 2112        /* Modbus register address to control the display page*/
#define MODBUS_REGISTER_MODE 2502 
#define MODBUS_REGISTER_INDICATOR 206    /*Modbus register address (in decimal) of the Indicator*/
#define MODBUS_REGISTER_GAUGE 106        /*Modbus register address (in decimal) of the Gauge*/
#define MODBUS_REGISTER_TOGGLEBUTTON 7   /*Modbus register address (in decimal) of the Toggle Button*/

#define CONFIG_MODE   0
#define DISPLAY_MODE  1
#define ButtonSensor  A0
#define VR_Sensor     A1

// #define THRESHOLD 500    // Change this to adjust the sensitivity of the physical button unit: (mV)
#define ADC_REF 5        //reference voltage of ADC is 5v.If the Vcc switch on the seeeduino
                         //board switches to 3V3, the ADC_REF should be 3.3
#define GROVE_VCC 5      //VCC of the grove interface is normally 5v
#define FULL_ANGLE 300   //full value of the rotary angle is 300 degrees

uint16_t ButtonSensorValue = 0;    //Physical Button Sensor value store
uint16_t IndicatorValue = 0;       //Indicator value store
uint16_t ToggleButtonValue = 0;    //Toggle Button value store
uint16_t VR_SensorValue = 0;       //Variable Resistor Sensor value store
uint16_t GaugeValue = 0;           //Gauge value store
float    Voltage = 0;              //Voltage value store

ModbusMaster node;

uint8_t  nowPage = 0;
char prbuf[64];

void setup() {
  pinMode(13, OUTPUT);                // init LED at pin 13 as output
  pinMode(12, OUTPUT);                // init LED at pin 12 as output
  pinMode(VR_Sensor, INPUT);          // init Variable Resistor Sensor as input
  pinMode(ButtonSensor, INPUT);       // init Physical Button Sensoras as input
  Serial.begin(115200);               // initialize the Serial port
  node.begin(SLAVE_ADDRESS, Serial);  // using the Serial object for communication
}

void changePageFunction( uint8_t pageValue ) 
{  
  // Step 1. Send the device mode command - switch to config mode
  node.writeSingleRegister(MODBUS_REGISTER_MODE, CONFIG_MODE);

 // Step 5. Send the change page command
  node.writeSingleRegister(MODBUS_REGISTER_PAGE, pageValue);

  // Step 6. Send the device mode command - switch to display mode
  node.writeSingleRegister(MODBUS_REGISTER_MODE, DISPLAY_MODE);
}

void loop() {

  VR_SensorValue = analogRead(VR_Sensor);
  Voltage        = (float)VR_SensorValue*ADC_REF/1023;
  float degrees  = (Voltage*FULL_ANGLE)/GROVE_VCC;
  GaugeValue     = map( degrees, 0, 300, 0, 160);     /*Here gauge rotation angle is set. It could be controlled according to the type of gauge.*/
  
  if( GaugeValue >= 120 ) {
    digitalWrite(13, HIGH);  /*pin 12 turn the LED on by making the voltage HIGH*/
    IndicatorValue = 0;      /*turn the indicator turn red by making the value 0*/
  }
  else {
    digitalWrite(13, LOW);   /*pin 12 turn the LED off by making the voltage LOW*/
    IndicatorValue = 1;      /*turn the indicator turn green by making the value 1*/
    }
  
  int ButtonSensorValue = digitalRead(ButtonSensor); /*read physical button voltage*/

  if ( ButtonSensorValue == HIGH ) {
    nowPage ++;
    if(nowPage>1)
      nowPage=0;

    changePageFunction(nowPage);
    // sprintf( prbuf, "nowPage %d", nowPage );
    // Serial.println(prbuf);
  }

  if(ToggleButtonValue == 1 ) { 
  // if(nowPage!=0)
    digitalWrite(12, HIGH);  // turn the LED on by making the voltage High
  }
  else{
    digitalWrite(12, LOW);  // turn the LED on by making the voltage Low  
  } 
 
  // wait button release 
  while( ButtonSensorValue == HIGH) {
    ButtonSensorValue = digitalRead(ButtonSensor);
    delay(100);
  }

  // Write the indicator value to the Modbus register
  node.writeSingleRegister(MODBUS_REGISTER_INDICATOR, IndicatorValue);

  // Write the Gauge value to the Modbus register
  node.writeSingleRegister(MODBUS_REGISTER_GAUGE, GaugeValue);
  Serial.print(" Gauge Value: ");
  Serial.print(GaugeValue);
  Serial.println();
  delay(100);

  // Read the Toggle Button value to the Modbus register
  node.readHoldingRegisters(MODBUS_REGISTER_TOGGLEBUTTON, 1);
  // Get the Toggle Button value from the Modbus response buffer
  ToggleButtonValue = node.getResponseBuffer(0);
}


  