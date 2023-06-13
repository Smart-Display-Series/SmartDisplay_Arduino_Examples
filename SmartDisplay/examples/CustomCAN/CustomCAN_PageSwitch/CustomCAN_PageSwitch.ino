#include <SPI.h>
#include "mcp2515_can.h"

const int SPI_CS_PIN = 9;
mcp2515_can CAN(SPI_CS_PIN); // Define the MCP2515 CAN controller object

#define CAN_PAGE_ID       0xEF7B00     
#define CAN_REGISTER_MODE 0x1
#define CONFIG_MODE     0
#define DISPLAY_MODE    1
#define BUTTON_PIN      A1           // Change this to the pin number of your physical button
#define THRESHOLD       500          // Change this to adjust the sensitivity of the physical button unit: (mV)

uint16_t buttonValue = 0;
bool buttonState = HIGH;
bool lastButtonState = LOW;
unsigned long lastDebounce = 0;
unsigned long debounceDelay = 60;
uint8_t nowPage = 0;
char prbuf[64];

unsigned char  ConfigMode[8] = { 0x53, 0x06, 0x20, 0x00, 0xE8, 0xA1, 0, 0 };
unsigned char PageIndex_1[8] = { 0x53, 0x06, 0x08, 0x01, 0x37, 0x61, 0, 0 };
unsigned char PageIndex_0[8] = { 0x53, 0x06, 0x08, 0x00, 0xF6, 0xA1, 0, 0 };
unsigned char DisplayMode[8] = { 0x53, 0x06, 0x20, 0x01, 0x29, 0x61, 0, 0 };

void setup() {
  Serial.begin(115200);
  while(!Serial){};             // wait for serial port to connect          
  pinMode(LED_BUILTIN, OUTPUT); // init LED at pin 13 as output
  pinMode(BUTTON_PIN, INPUT);   

  while (CAN_OK != CAN.begin(CAN_250KBPS)) {             // init can bus : baudrate = 250k
    Serial.println("CAN BUS Shield init fail");
    Serial.println(" Init CAN BUS Shield again");
    delay(100);
  }
  Serial.println("CAN BUS Shield init ok!");
}
 
void changePageFunction(uint8_t pageValue) 
{  
  // Step 1. Send the device mode command - switch to config mode
  CAN.sendMsgBuf(CAN_PAGE_ID, 1, 6, ConfigMode);
 // Step 2. Send the change page command
  if(pageValue==0) {
    CAN.sendMsgBuf(CAN_PAGE_ID, 1, 6, PageIndex_0);
  }
  else if(pageValue==1){
    CAN.sendMsgBuf(CAN_PAGE_ID, 1, 6, PageIndex_1);
  }
  // Step 3. Send the device mode command - switch to display mode
  CAN.sendMsgBuf(CAN_PAGE_ID, 1, 6, DisplayMode);  
}

void loop() {  
  // read physical button voltage
  int buttonValue = analogRead(BUTTON_PIN);
  if (buttonValue > THRESHOLD) {
    nowPage++;
    if (nowPage > 1) {
      nowPage = 0;

    }

    sprintf( prbuf, "nowPage %d", nowPage );
    Serial.println(prbuf);
    changePageFunction(nowPage);

    if (nowPage != 0) {
      digitalWrite(LED_BUILTIN, HIGH);  // turn the LED on by making the voltage High
    } else {
      digitalWrite(LED_BUILTIN, LOW);  // turn the LED on by making the voltage High
    }
 
    lastDebounce = millis();
  }

  if (millis() - lastDebounce > debounceDelay) {
    lastButtonState = buttonState;
  }

  // wait button release 
  while (buttonValue > THRESHOLD) {
    buttonValue = analogRead(BUTTON_PIN);
    delay(100);
  }

  delay(100);     // send data per 100ms
}

