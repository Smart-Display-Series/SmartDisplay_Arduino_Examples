#include <ModbusMaster.h>

// Modbus parameters
#define MODBUS_SLAVE_ID 1     // Modbus slave ID of the smart display 
#define MODBUS_BAUD_RATE 115200 // Baud rate for Modbus communication
#define MODBUS_PARITY 'N'
#define MODBUS_DATA_FORMAT 8
#define MODBUS_SERIAL Serial1 // Modbus slave ID of the smart display
#define MODBUS_REGISTER_PAGE 0x01 // Modbus register to control the display page

// Button configuration
#define BUTTON_PIN 2  // Pin for the physical button

// Page colors
#define PAGE1_COLOR "red"
#define PAGE2_COLOR "blue"

// Modbus object
ModbusMaster modbus;

// Button state variables
bool buttonState = false;
bool lastButtonState = false; // Initialize lastButtonState with a default value

void setup() {
  // Initialize Serial for Modbus communication
  MODBUS_SERIAL.begin(static_cast<unsigned long>(MODBUS_BAUD_RATE), MODBUS_DATA_FORMAT);
 
  // Initialize button pin as input
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  
  // Connect to Modbus device
  modbus.begin(MODBUS_SLAVE_ID, MODBUS_SERIAL);
}


// void loop() {
//   // Read button status
//   int buttonState = digitalRead(BUTTON_PIN);
  
//   // Send Modbus command to switch pages based on button status
//   if (buttonState == LOW) {
//     // Button pressed, send Modbus command to switch to second page
//     modbus.writeSingleRegister(0, 1); // Write 1 to register 0 to switch to second page
//   } else {
//     // Button released, send Modbus command to switch to first page
//     modbus.writeSingleRegister(0, 0); // Write 0 to register 0 to switch to first page
//   }

//   // Wait for a short duration before checking button status again
//   delay(100);
// }


void loop() {
  // Read the button state
  buttonState = digitalRead(BUTTON_PIN);

  // Detect button press (falling edge)
  if (!buttonState && lastButtonState) {
    // Switch page on the smart display
    modbus.writeSingleRegister(static_cast<uint16_t>(MODBUS_SLAVE_ID), static_cast<uint16_t>(MODBUS_REGISTER_PAGE));

      Serial.println("Switching to Page 2...");
      delay(100); // Delay for Modbus communication
    } else {
      Serial.println("Failed to switch page!");
    }
  
  // Detect button release (rising edge)
  if (buttonState && !lastButtonState) {
    // Switch page on the smart display
    modbus.writeSingleRegister(static_cast<uint16_t>(MODBUS_SLAVE_ID), static_cast<uint16_t>(MODBUS_REGISTER_PAGE));

      Serial.println("Switching to Page 1...");
      delay(100); // Delay for Modbus communication
    } else {
      Serial.println("Failed to switch page!");
    }
  }

  // lastButtonState = buttonState; // Update last button state
// }


