#include <ModbusMaster.h>

#define SLAVE_ADDRESS 0x7B
#define BRIGHTNESS_REGISTER_ADDRESS 2111
#define MODBUS_REGISTER_PAGE 2112        // Modbus register to control the display page
#define MODBUS_REGISTER_MODE 2502

#define CONFIG_MODE     0
#define DISPLAY_MODE    1
#define BUTTON_PIN      A0           // Change this to the pin number of your physical button
#define THRESHOLD       500          // Change this to adjust the sensitivity of the physical button unit: (mV)
// #define DEBOUNCE_TIME   0         // Change this to adjust the debounce time (in ms)

#define MIN_BRIGHTNESS 0
#define MAX_BRIGHTNESS 255

// #define PAGE1_COLOR "red"
// #define PAGE2_COLOR "blue"
const int led = 13;
uint16_t buttonValue = 0;

ModbusMaster node;

bool buttonState = HIGH;
bool lastButtonState = LOW;

unsigned long lastDebounce = 0;
unsigned long debounceDelay = 60;

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  // pinMode(BUTTON_PIN, INPUT_PULLUP);    // Set the physical button pin as input with pull-up resistor
  pinMode(BUTTON_PIN, INPUT);
  Serial.begin(115200);
  node.begin(SLAVE_ADDRESS, Serial); // using the Serial object for communication
}

void chagePageFunction( uint8_t pageValue ) 
{  
  // Step 1. Send the device mode command - switch to config mode
  node.writeSingleRegister(MODBUS_REGISTER_MODE, CONFIG_MODE);

 // Step 5. Send the change page command
  node.writeSingleRegister(MODBUS_REGISTER_PAGE, pageValue);

  // Step 6. Send the device mode command - switch to display mode
  node.writeSingleRegister(MODBUS_REGISTER_MODE, DISPLAY_MODE);
}

void loop() {
  
  // read physical button voltage
  int buttonValue = analogRead(BUTTON_PIN);

  if( buttonValue > THRESHOLD ) {
    
    buttonState = HIGH;
  }
  else {
    
    buttonState = LOW;
  }

  // if ( buttonState != lastButtonState) {

  //   lastDebounce = millis();        // Starting lastDebounce timing
  // }
 
  if ( (millis() - lastDebounce) > debounceDelay) {        // Check if delay has passed

    if (lastButtonState != buttonState) {

      lastButtonState = buttonState;
      
      if( buttonState == HIGH ) {

        chagePageFunction(1);
        digitalWrite(LED_BUILTIN, HIGH);  // turn the LED on by making the voltage High
      }
      else {
        
        chagePageFunction(0);
        digitalWrite(LED_BUILTIN, LOW);   // turn the LED off by making the voltage Low
      }
    }
  } 
}

