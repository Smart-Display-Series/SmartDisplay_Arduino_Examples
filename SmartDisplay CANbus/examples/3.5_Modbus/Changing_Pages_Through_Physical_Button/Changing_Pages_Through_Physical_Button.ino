#include <ModbusMaster.h>

#define SLAVE_ADDRESS 0x7B
#define BRIGHTNESS_REGISTER_ADDRESS 2111
// #define MODBUS_REGISTER_PAGE 2112  // Modbus register to control the display page

#define MIN_BRIGHTNESS 0
#define MAX_BRIGHTNESS 255

// #define PAGE1_COLOR "red"
// #define PAGE2_COLOR "blue"
// const int led = 13;

ModbusMaster node;

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  
  Serial.begin(115200);
  node.begin(SLAVE_ADDRESS, Serial); // using the Serial object for communication
  // node.writeSingleRegister(0x0002, 0x7B06); // Set backlight to 123
}

void loop() {
  // digitalWrite(led, 1 - digitalRead(led));
  digitalWrite(LED_BUILTIN, LOW);  // turn the LED on by making the voltage Low
  node.writeSingleRegister(BRIGHTNESS_REGISTER_ADDRESS, MIN_BRIGHTNESS  ); //Minimum brightness level
  // node.writeSingleRegister(0x2101, 150);
  delay(1000);

  digitalWrite(LED_BUILTIN, HIGH);   // turn the LED off by making the voltage High
  node.writeSingleRegister(BRIGHTNESS_REGISTER_ADDRESS, MAX_BRIGHTNESS); //Maximum brightness level
  // node.writeSingleRegister(0x2101, 255);
  delay(1000);
}


