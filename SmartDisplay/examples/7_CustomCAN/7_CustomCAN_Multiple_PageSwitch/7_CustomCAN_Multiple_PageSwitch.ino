#include <SPI.h>
#include "mcp2515_can.h"

const int SPI_CS_PIN = 9;
mcp2515_can CAN(SPI_CS_PIN);

#define CAN_PAGE_ID       0xEF7B00     
#define CAN_REGISTER_MODE 0x1

#define CONFIG_MODE     0
#define DISPLAY_MODE    1
#define BUTTON_PIN      A1
#define THRESHOLD       500

const int led = 13;
bool buttonState = HIGH;
bool lastButtonState = LOW;
unsigned long lastDebounce = 0;
unsigned long debounceDelay = 60;
uint8_t nowPage = 0;
char prbuf[64];

unsigned char   configMsg[8] = { 0x53, 0x06, 0x20, 0x00, 0xE8, 0xA1 };
unsigned char PageIndex_0[8] = { 0x53, 0x06, 0x08, 0x00, 0xF6, 0xA1 };
unsigned char PageIndex_1[8] = { 0x53, 0x06, 0x08, 0x01, 0x37, 0x61 };
unsigned char PageIndex_2[8] = { 0x53, 0x06, 0x08, 0x02, 0x77, 0x60 };
unsigned char PageIndex_3[8] = { 0x53, 0x06, 0x08, 0x03, 0xB6, 0xA0 };
unsigned char PageIndex_4[8] = { 0x53, 0x06, 0x08, 0x04, 0xF7, 0x62 };
unsigned char PageIndex_5[8] = { 0x53, 0x06, 0x08, 0x05, 0x36, 0xA2 };
unsigned char PageIndex_6[8] = { 0x53, 0x06, 0x08, 0x06, 0x76, 0xA3 };
unsigned char PageIndex_7[8] = { 0x53, 0x06, 0x08, 0x07, 0xB7, 0x63 };
unsigned char PageIndex_8[8] = { 0x53, 0x06, 0x08, 0x08, 0xF7, 0x67 };
unsigned char PageIndex_9[8] = { 0x53, 0x06, 0x08, 0x09, 0x36, 0xA7 };
unsigned char displayMode[8] = { 0x53, 0x06, 0x20, 0x01, 0x29, 0x61 };

void setup() {
  Serial.begin(115200);
  while(!Serial){}; // wait for serial port to connect
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT);   

  while (CAN_OK != CAN.begin(CAN_250KBPS)) {
    Serial.println("CAN BUS Shield init fail");
    Serial.println(" Init CAN BUS Shield again");
    delay(100);
  }
  Serial.println("CAN BUS Shield init ok!");
}

void changePageFunction(uint8_t pageValue) 
{  
  CAN.sendMsgBuf(CAN_PAGE_ID, 1, 6, configMsg);

  if(pageValue==0) {
    CAN.sendMsgBuf(CAN_PAGE_ID, 1, 6, PageIndex_0);
  }
  else if(pageValue==1){
    CAN.sendMsgBuf(CAN_PAGE_ID, 1, 6, PageIndex_1);
  }
  else if(pageValue==2){
    CAN.sendMsgBuf(CAN_PAGE_ID, 1, 6, PageIndex_2);
  }
  else if(pageValue==3){
    CAN.sendMsgBuf(CAN_PAGE_ID, 1, 6, PageIndex_3);
  }
  else if(pageValue==4){
    CAN.sendMsgBuf(CAN_PAGE_ID, 1, 6, PageIndex_4);
  }
  else if(pageValue==5){
    CAN.sendMsgBuf(CAN_PAGE_ID, 1, 6, PageIndex_5);
  }
  else if(pageValue==6){
    CAN.sendMsgBuf(CAN_PAGE_ID, 1, 6, PageIndex_6);
  }
  else if(pageValue==7){
    CAN.sendMsgBuf(CAN_PAGE_ID, 1, 6, PageIndex_7);
  }
  else if(pageValue==8){
    CAN.sendMsgBuf(CAN_PAGE_ID, 1, 6, PageIndex_8);
  }
  else if(pageValue==9){
    CAN.sendMsgBuf(CAN_PAGE_ID, 1, 6, PageIndex_9);
  }
  
  CAN.sendMsgBuf(CAN_PAGE_ID, 1, 6, displayMode);
}

void loop() {  
  int buttonValue = analogRead(BUTTON_PIN);
  if (buttonValue > THRESHOLD && lastButtonState == LOW && millis() - lastDebounce > debounceDelay) {
    nowPage++;
    if (nowPage > 9) {
      nowPage = 0;
    }

    sprintf( prbuf, "nowPage %d", nowPage );
    Serial.println(prbuf);
    changePageFunction(nowPage);

    if (nowPage % 2 != 0) {
      digitalWrite(LED_BUILTIN, HIGH);
    } else {
      digitalWrite(LED_BUILTIN, LOW);
    }

    lastDebounce = millis();
  }

  lastButtonState = buttonValue > THRESHOLD;

  delay(100);
}

