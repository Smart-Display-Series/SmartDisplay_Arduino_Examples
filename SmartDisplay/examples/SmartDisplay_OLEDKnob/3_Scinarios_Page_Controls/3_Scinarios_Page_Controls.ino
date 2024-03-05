/* Private includes ----------------------------------------------------------*/
#include <Adafruit_SSD1327.h>
#include <Adafruit_FT6206.h>
#include <Adafruit_GFX.h>
#include "mcp2515_can.h"
#include "lcdgfx_gui.h"
#include "lcdgfx.h"
#include <SPI.h>

#include "Image.h"

/* Private define ------------------------------------------------------------*/
// Rotary Encoder Pins define
#define pinA A0
#define pinB A1
#define pinC A2
#define EN_Pin 8

#define OLED_WIDTH 128
#define OLED_HEIGHT 128

#define CAN_PAGE_ID 0xEF7B00
#define CAN_REGISTER_MODE 0x1
#define CONFIG_MODE 0
#define DISPLAY_MODE 1

// #define MAX_WIDGETS_PER_PAGE 5

#define PRESS_CNT_LONG   20

#define KNOB_START  0
#define KNOB_ROTATE 1
#define KNOB_IDLE   2
#define KNOB_UNKONW 99

/*SAMD core*/
#ifdef ARDUINO_SAMD_VARIANT_COMPLIANCE
#define SERIAL SerialUSB
#else
#define SERIAL Serial
#endif

/* Private variables ---------------------------------------------------------*/

const int SPI_CS_PIN = 9;
const int CAN_INT_PIN = 2;

mcp2515_can CAN(SPI_CS_PIN);  // Set CS pin

Adafruit_FT6206 ts = Adafruit_FT6206();  // Touchscreen

DisplaySSD1327_128x128_SPI display(5, { -1, 4, 7, 0, -1, -1 });  // Display SPI pin configuration

// Define global variables
int counter = 0;

int oldState;
int newState;

bool isRotatingLeft = false;
bool isRotatingRight = false;

unsigned long idleTick = 0;  // Timestamp when the idle

int16_t IndicatorValue = 0;   //Indicator value store
int16_t LastIndicatorValue = 0;   //Indicator value store

int16_t ToggleButtonValue = 0;  //Toggle Button value store
int16_t LastToggleButtonValue = 0;

uint16_t MultiStateValue = 0;  //MultiState value store
uint16_t LastMultiStateValue = 0;

uint16_t CircleProgressValue = 0;
uint16_t LastCircleProgressValue = 0;

uint16_t ImageProgressValue = 0;
uint16_t LastImageProgressValue = 0;

uint16_t GaugeValue = 0;
uint16_t LastGaugeValue = 0;

uint16_t TemperatureValue = 0;
uint16_t LastTemperatureValue = 0;

uint16_t BatteryValue = 0; 
uint16_t LastBatteryValue = 0;

uint16_t VerticalSliderValue = 0;
uint16_t LastVerticalSliderValue  = 0;

uint16_t HorizontalSliderValue = 0;
uint16_t LastHorizontalSliderValue  = 0;

uint16_t FixedWidgetValue = 0;
uint16_t LastFixedWidgetValue = 0;

uint16_t GraphValue = 0;
uint16_t LastGraphValue = 0;

uint16_t AnimationValue = 0;
uint16_t LastAnimationValue = 0;

uint8_t nowPage = 0;
char prbuf[64];

uint8_t knob_state = KNOB_START;
unsigned char flagRecv = 0;

unsigned char configMsg[8] = { 0x53, 0x06, 0x20, 0x00, 0xE8, 0xA1 };
unsigned char PageIndex_0[8] = { 0x53, 0x06, 0x08, 0x00, 0xF6, 0xA1 };
unsigned char PageIndex_1[8] = { 0x53, 0x06, 0x08, 0x01, 0x37, 0x61 };
unsigned char PageIndex_2[8] = { 0x53, 0x06, 0x08, 0x02, 0x77, 0x60 };
unsigned char PageIndex_3[8] = { 0x53, 0x06, 0x08, 0x03, 0xB6, 0xA0 };
// unsigned char PageIndex_4[8] = { 0x53, 0x06, 0x08, 0x04, 0xF7, 0x62 };
unsigned char displayMode[8] = { 0x53, 0x06, 0x20, 0x01, 0x29, 0x61 };

unsigned char stmp[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };   //Indicator (0x70C)array data Bytes
unsigned char stmp1[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };  //Multistate (0x716)array data Bytes
unsigned char stmp2[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };  //Circle Progress (0x70B)array data Bytes
unsigned char stmp3[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };  //Image Progress (0x70E)array data Bytes
unsigned char stmp4[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };  //Gauge (0x702)array data Bytes
unsigned char stmp5[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };  //Temperature (0x708)array data Bytes
unsigned char stmp6[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };  //Battery (0x70A)array data Bytes
unsigned char stmp7[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };  //VerticalSlider (0x706)array data Bytes
unsigned char stmp8[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };  //HorizontalSlider (0x707)array data Bytes
unsigned char stmp9[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };  //FixedWidget (0x713)array data Bytes
unsigned char stmp10[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };  //Graph (0x709)array data Bytes
unsigned char stmp11[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };  //ToggleButton (0x705)array data Bytes
unsigned char stmp12[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };  //Animation (0x70F)array data Bytes

/* Private function prototypes -----------------------------------------------*/


/* Private user code ---------------------------------------------------------*/

// Draw the image to display
void bitmapDemo( int imageIndex ) 
{
  display.clear();

  if (imageIndex == 0) {
    display.drawBitmap1(66, 5, 57, 55, Right_Image_01);
  } else if (imageIndex == 1) {
    display.drawBitmap1(67, 65, 55, 57, Right_Image_02);
  } else if (imageIndex == 2) {
    display.drawBitmap1(5, 67, 57, 55, Right_Image_03);
  } else if (imageIndex == 3) {
    display.drawBitmap1(5, 5, 55, 56, Right_Image_04);
  }
}

void waiteResponse()
{
  uint8_t len;
  uint8_t buf[8];
  unsigned long id;
  
  char str[40];

  uint32_t tick = millis();

  do {
    
    delay(200);

    SERIAL.println( "wait canbus response msg" );

  } while( flagRecv == 0 );

  flagRecv = 0;  
  // --------------------------------------

  sprintf( str, "wait response time  : %d mSec", millis() - tick );

  SERIAL.println( str );

  CAN.readMsgBufID( &id, &len, buf );
  
  SERIAL.println("receiver");

  SERIAL.print( "id   : " );
  
  SERIAL.println( CAN.getCanId(), HEX );
  
  sprintf( str, "len  : %d ", len );
  
  SERIAL.println( str );

  SERIAL.print("byte : ");

  for( uint8_t i = 0; i < len; i++ ) {
    
    sprintf( str, "0x%02x ", buf[i] );

    SERIAL.print( str );
  }

  SERIAL.println( " " );
}

void changePageFunction(uint8_t pageValue) 
{
  CAN.sendMsgBuf(CAN_PAGE_ID, 1, 6, configMsg);

  waiteResponse();
  
  if (pageValue == 0) {
    
    CAN.sendMsgBuf(CAN_PAGE_ID, 1, 6, PageIndex_0);
  } 
  else if (pageValue == 1) {
    
    CAN.sendMsgBuf(CAN_PAGE_ID, 1, 6, PageIndex_1);
  } 
  else if (pageValue == 2) {
    
    CAN.sendMsgBuf(CAN_PAGE_ID, 1, 6, PageIndex_2);
  } 
  else if (pageValue == 3) {
    
    CAN.sendMsgBuf(CAN_PAGE_ID, 1, 6, PageIndex_3);
  } 
  
  waiteResponse();
  
  CAN.sendMsgBuf(CAN_PAGE_ID, 1, 6, displayMode);
  
  waiteResponse();

  SERIAL.print("changePageFunction ");

  SERIAL.println( pageValue );
}

// Function to handle touch events
void handleTouchEvents() {
  
  static uint32_t scan_tick = 0;
  static uint8_t isPressedCnt = PRESS_CNT_LONG;
  
  if( millis() <= scan_tick )
    return;
  
  scan_tick = millis() + 20;
  
  if ( ts.touched() ) {
    
    TS_Point p = ts.getPoint();  // Retrieve a point

    int16_t x = map(p.x, 0, 128, 0, OLED_WIDTH - 1);
    int16_t y = map(p.y, 0, 128, 0, OLED_HEIGHT - 1);

    // Print debug information
    // Serial.print("X = ");
    // Serial.print(x);
    // Serial.print("\tY = ");
    // Serial.print(y);
    // Serial.print("\tPressure = ");
    // Serial.println(p.z);
    // Serial.println("Display Touched");
    
    if ( p.z >= 1 && p.z <= 1000 ) {
            
      setKnobState( KNOB_START );
      
      if( isPressedCnt ) {

        isPressedCnt --;

        // Serial.print("isPressedCnt : ");
        // Serial.println(isPressedCnt);
      }

      if( isPressedCnt == 1 ) // long time press
      {
        Serial.println("long pressed");
        nowPage = 0;        
        changePageFunction( nowPage );
        SERIAL.println("switchToHomePage");
      }
    }
  } 
  else {
    
    if( ( isPressedCnt > 1 ) && ( isPressedCnt < PRESS_CNT_LONG ) ) {
      
      Serial.println("short pressed");
      
      if(nowPage == 0 && MultiStateValue == 0){
        nowPage = 0;        
      }
      else if(nowPage == 0 && MultiStateValue == 1){
        nowPage = 1;        
      }
      else if (nowPage == 0 && MultiStateValue == 2){
        nowPage = 2;        
      }
      else if (nowPage == 0 && MultiStateValue == 3){
        nowPage = 3;        
      }      

      // Call the function to change the page
      changePageFunction(nowPage);
      
    }
    
    isPressedCnt = PRESS_CNT_LONG;
  }
}

void setup() { 
  pinMode(pinA, INPUT_PULLUP);
  pinMode(pinB, INPUT_PULLUP);
  pinMode(pinC, INPUT_PULLUP);
  pinMode(EN_Pin, OUTPUT);
  digitalWrite(EN_Pin, HIGH);

  Serial.begin(115200);
  while (!Serial) {};
  display.begin();
  display.clear();
  display.setFixedFont(ssd1306xled_font6x8);  // Display set text font

  display.setColor(GRAY_COLOR4(255));  // Display set color
  display.drawBitmap1(27, 37, 79, 46, SmartDisplay_Image);
  delay(2000);

 #if 1
  attachInterrupt( digitalPinToInterrupt(CAN_INT_PIN), MCP2515_ISR, FALLING ); // start interrupt

  while (CAN_OK != CAN.begin(CAN_250KBPS)) {  // init can bus : baudrate = 250k
    CAN.setMode(MODE_NORMAL); // Set to normal mode and start receiving messages
    SERIAL.println("CAN BUS Shield init fail");
    SERIAL.println(" Init CAN BUS Shield again");
    delay(100);
  }
  SERIAL.println("CAN BUS Shield init ok!");
  
  // Set filter masks to accept all frames, including extended ones
  CAN.init_Mask(0, CAN_STDID, 0); // Mask no Standard-ID bits to be filtered (0x00)
  CAN.init_Mask(1, CAN_EXTID, 0); // Mask no Extended-ID bits to be filtered (0x00)
  CAN.init_Filt(0, CAN_STDID, 0); // Init filter #0.0 in mcp2515 for no filtering
  CAN.init_Filt(1, CAN_STDID, 0); // Init filter #0.1 in mcp2515 for no filtering
  CAN.init_Filt(2, CAN_EXTID, 0); // Init filter #1.2 in mcp2515 for no filtering
  CAN.init_Filt(3, CAN_EXTID, 0); // Init filter #1.3 in mcp2515 for no filtering
  CAN.init_Filt(4, CAN_EXTID, 0); // Init filter #1.4 in mcp2515 for no filtering
  CAN.init_Filt(5, CAN_EXTID, 0); // Init filter #1.5 in mcp2515 for no filtering
 #endif

  if ( !ts.begin(0) ) {
    Serial.println("Couldn't start FT6206 touchscreen controller");
    while (1);
  }
  Serial.println("Touchscreen started");
}

void MCP2515_ISR() 
{
  uint8_t len;
  uint8_t buf[8];
  
  flagRecv = 1;
  SERIAL.println( "MCP2515_ISR!" );
}

void setKnobState( uint8_t state )
{
  knob_state = state;
  
  idleTick = millis() + 10000;
}

int isKnobIdle( void )
{    
  if( millis() < idleTick )
    return 0;
  
  return 1;
}

void redrawScreen()
{
  static uint8_t lastState = KNOB_UNKONW;

  if( knob_state == lastState )
      return;
  
  lastState = knob_state;
  
  if( knob_state == KNOB_ROTATE )
    return;
  
  display.clear();

  if( knob_state == KNOB_START ) {
    
    display.drawBitmap1(27, 37, 79, 46, SmartDisplay_Image);
    
  }
    else if( knob_state == KNOB_IDLE ) {
    
    display.drawBitmap1(30, 32, 74, 59, Touch_or_Rotate_Image);

    MultiStateValue = 0;
    nowPage = 0;        
    changePageFunction( nowPage );
    SERIAL.println("switchToHomePage");
    
  }
  else if( knob_state == KNOB_ROTATE ) {
    
  }
}

#if 1
void loop()
 {
  int pinAState = digitalRead(pinA);
  int pinBState = digitalRead(pinB);
  int pinCState = digitalRead(pinC);
  
  #if 0
  GaugeValue            = map(counter, 0, 16, 0, 255);   /*Here gauge rotation angle is set. It could be controlled according to the type of gauge.*/
  ImageProgressValue    = map(counter, 0, 16, 0, 255);
  CircleProgressValue   = map(counter, 0, 16, 0, 100);
  BatteryValue          = map(counter, 0, 16, 0, 255);
  TemperatureValue      = map(counter, 0, 16, 0, 100);
  VerticalSliderValue   = map(counter, 0, 16, 0, 255);
  HorizontalSliderValue = map(counter, 0, 16, 0, 255);
  #endif
  
  if (pinAState == HIGH && pinBState == HIGH && pinCState == LOW)
    newState = 1;
  else if (pinAState == HIGH && pinBState == LOW && pinCState == HIGH)
    newState = 2;
  else if (pinAState == LOW && pinBState == HIGH && pinCState == HIGH)
    newState = 3;

  if ( oldState != newState ) 
  {
    if (oldState == 1 && newState == 2 || 
        oldState == 2 && newState == 3 || 
        oldState == 3 && newState == 1) {
          
      counter = ( counter + 1 ) % 4;
      MultiStateValue = ( MultiStateValue + 1 ) % 4;

      if( GaugeValue < 255)
        GaugeValue += 26;

      if( GaugeValue > 255 )
        GaugeValue = 255;

      if( CircleProgressValue < 100)
        CircleProgressValue += 10;
        
      if( CircleProgressValue > 100 )
        CircleProgressValue = 100; 

      if( ImageProgressValue < 255)
        ImageProgressValue += 26;
        
      if( ImageProgressValue > 255 )
        ImageProgressValue = 255;  

      if( HorizontalSliderValue < 255)
        HorizontalSliderValue += 26;
        
      if( HorizontalSliderValue > 255 )
        HorizontalSliderValue = 255;  
        
      if( VerticalSliderValue < 255)
        VerticalSliderValue += 26;
        
      if( VerticalSliderValue > 255 )
        VerticalSliderValue = 255; 

      if( TemperatureValue < 100)
        TemperatureValue += 10;
        
      if( TemperatureValue > 100 )
        TemperatureValue = 100;              

      if( BatteryValue < 255)
        BatteryValue += 26;
        
      if( BatteryValue > 255 )
        BatteryValue = 255;
        
      if( FixedWidgetValue < 4 )    
        FixedWidgetValue += 1;

      if( FixedWidgetValue > 4 )    
        FixedWidgetValue = 4;
              
      if( GraphValue < 255)
        GraphValue += 26;

      if( GraphValue > 255 )
        GraphValue = 255; 
        
      if( AnimationValue < 100)
        AnimationValue += 10;
        
      if( AnimationValue > 100 )
        AnimationValue = 100;
      
      if( IndicatorValue < 1)
        IndicatorValue += 1;
        
      if( IndicatorValue > 1 )
        IndicatorValue = 1;
                      
      if( ToggleButtonValue > 1)
        ToggleButtonValue += 1;
        
      if( ToggleButtonValue < 1 )
        ToggleButtonValue = 1;
      
      isRotatingLeft = false; 
      isRotatingRight = true;           
       
      Serial.println("Direction: Right Rotation");        
    } 
    else if ( oldState == 2 && newState == 1 || 
              oldState == 1 && newState == 3 || 
              oldState == 3 && newState == 2) {
                
      counter = ( counter + 3 ) % 4; 
      MultiStateValue = ( MultiStateValue + 3 ) % 4;
      

      if( GaugeValue > 26 )    
        GaugeValue -= 26;
      else
        GaugeValue = 0;

      if( CircleProgressValue > 10 )    
        CircleProgressValue -= 10;
      else
        CircleProgressValue = 0;

      if( ImageProgressValue > 26 )    
        ImageProgressValue -= 26;
      else
        ImageProgressValue = 0;
        
      if( HorizontalSliderValue > 26 )    
        HorizontalSliderValue -= 26;
      else
        HorizontalSliderValue = 0;
                 
      if( VerticalSliderValue > 26 )    
        VerticalSliderValue -= 26;
      else
        VerticalSliderValue = 0;

      if( TemperatureValue > 10 )    
        TemperatureValue -= 10;
      else
        TemperatureValue = 0;        

      if( BatteryValue > 26 )    
        BatteryValue -= 26;
      else
        BatteryValue = 0;

      if( FixedWidgetValue > 4 )    
        FixedWidgetValue -= 4;
      else
        FixedWidgetValue = 0;

      if( GraphValue > 26)
        GraphValue -= 26;
      else
        GraphValue = 0; 

      if( AnimationValue > 10 )    
        AnimationValue -= 10;
      else
        AnimationValue = 0;    
      
      if( IndicatorValue > 1 )    
        IndicatorValue -= 1;
      else
        IndicatorValue = 0; 
           
      if( ToggleButtonValue < 1 )    
        ToggleButtonValue -= 1;
      else
        ToggleButtonValue = 0;                  
       
      isRotatingLeft = true;
      isRotatingRight = false; 

      Serial.println("Direction: Left Rotation");  
    } 

    Serial.print("Counter: ");
    Serial.println(counter);
 
    setKnobState( KNOB_ROTATE );
    bitmapDemo( counter );

    FixedWidgetValue = counter; 
  }
  
  oldState = newState; 
  
  if (nowPage == 0){
    if( MultiStateValue != LastMultiStateValue )
    {
      LastMultiStateValue = MultiStateValue;
      stmp1[0] = MultiStateValue;
      CAN.sendMsgBuf(0x716, 0, 1, stmp1);   // send data of the MultiState: id = 0x716, byte ext, data len = 8, stmp1: data buf
      Serial.print( "Canbus Send: MultiStateValue " ); 
      Serial.println( MultiStateValue ); 
    }
  }

  if (nowPage == 1){
    if( CircleProgressValue  != LastCircleProgressValue ) 
    {
      LastCircleProgressValue = CircleProgressValue;
      stmp2[0] = CircleProgressValue;
      CAN.sendMsgBuf(0x70B, 0, 3, stmp2);   // send data of the CircleProgres: id = 0x70B, byte ext, data len = 8, stmp2: data buf
    }

    if( HorizontalSliderValue != LastHorizontalSliderValue ) 
    {
      LastHorizontalSliderValue = HorizontalSliderValue;
      stmp8[0] = HorizontalSliderValue;
      CAN.sendMsgBuf(0x707, 0, 2, stmp8);   // send data of the HorizontalSlider: id = 0x70E, byte ext, data len = 8, stmp8: data buf
    }
    
    if( VerticalSliderValue != LastVerticalSliderValue ) 
    {
      LastVerticalSliderValue = VerticalSliderValue;
      stmp7[0] = VerticalSliderValue;
      CAN.sendMsgBuf(0x706, 0, 3, stmp7);   // send data of the VerticalSlider: id = 0x706, byte ext, data len = 8, stmp7: data buf
    }

    if( TemperatureValue != LastTemperatureValue ) 
    {
      LastTemperatureValue = TemperatureValue;
      stmp5[0] = TemperatureValue;     
      CAN.sendMsgBuf(0x708, 0, 4, stmp5);   // send data of the Temperature: id = 0x708, byte ext, data len = 8, stmp5: data buf
    }

    if( GaugeValue != LastGaugeValue ) 
    {
      LastGaugeValue = GaugeValue;      // send data of the Gauge: id = 0x702, byte ext, data len = 8, stmp4: data buf
      stmp4[0] = GaugeValue;     
      CAN.sendMsgBuf(0x702, 0, 7, stmp4);
    }
    
    if( FixedWidgetValue != LastFixedWidgetValue ) 
    { 
      LastFixedWidgetValue = FixedWidgetValue;
      stmp9[0] = FixedWidgetValue; 
      CAN.sendMsgBuf(0x713, 0, 3, stmp9);   // send data of the FixedWidget: id = 0x713, byte ext, data len = 8, stmp9: data buf
    }

    if( IndicatorValue != LastIndicatorValue )
    {
      LastIndicatorValue = IndicatorValue;
      stmp[0]  = IndicatorValue;
      CAN.sendMsgBuf(0x70C, 0, 1, stmp);    // send data of the Indicator: id = 0x70C, byte ext, data len = 8, stmp: data buf 
    }        
  }

  if (nowPage == 2){    
    if( CircleProgressValue  != LastCircleProgressValue ) 
    {
      LastCircleProgressValue = CircleProgressValue;
      stmp2[0] = CircleProgressValue;
      CAN.sendMsgBuf(0x70B, 0, 3, stmp2);   // send data of the CircleProgres: id = 0x70B, byte ext, data len = 8, stmp2: data buf
    }

    if( HorizontalSliderValue != LastHorizontalSliderValue ) 
    {
      LastHorizontalSliderValue = HorizontalSliderValue;
      stmp8[0] = HorizontalSliderValue;
      CAN.sendMsgBuf(0x707, 0, 2, stmp8);   // send data of the HorizontalSlider: id = 0x70E, byte ext, data len = 8, stmp8: data buf
    }
    
    if( BatteryValue != LastBatteryValue ) 
    {
      LastBatteryValue = BatteryValue;
      stmp6[0] = BatteryValue;     
      CAN.sendMsgBuf(0x70A, 0, 2, stmp6);   // send data of the Battery: id = 0x70A, byte ext, data len = 8, stmp6: data buf
    }
    
    if( GraphValue != LastGraphValue ) 
    {
      LastGraphValue = GraphValue;
      stmp10[0] = GraphValue;     
      CAN.sendMsgBuf(0x709, 0, 2, stmp10);   // send data of the Battery: id = 0x709, byte ext, data len = 8, stmp10: data buf
    }

    if( AnimationValue != LastAnimationValue ) 
    {
      LastAnimationValue = AnimationValue;
      stmp12[0] = AnimationValue;
      CAN.sendMsgBuf(0x70F, 0, 2, stmp12);   // send data of the Animation: id = 0x70F, byte ext, data len = 8, stmp12: data buf
    }    
  }

  if (nowPage == 3){    
    if( HorizontalSliderValue != LastHorizontalSliderValue ) 
    {
      LastHorizontalSliderValue = HorizontalSliderValue;
      stmp8[0] = HorizontalSliderValue;
      CAN.sendMsgBuf(0x707, 0, 2, stmp8);   // send data of the HorizontalSlider: id = 0x70E, byte ext, data len = 8, stmp8: data buf
    } 
    
    if( GaugeValue != LastGaugeValue ) 
    {
      LastGaugeValue = GaugeValue;      // send data of the Gauge: id = 0x702, byte ext, data len = 8, stmp4: data buf
      stmp4[0] = GaugeValue;     
      CAN.sendMsgBuf(0x702, 0, 7, stmp4);     
    }
    
    if( FixedWidgetValue != LastFixedWidgetValue ) 
    { 
      LastFixedWidgetValue = FixedWidgetValue;
      stmp9[0] = FixedWidgetValue; 
      CAN.sendMsgBuf(0x713, 0, 3, stmp9);   // send data of the FixedWidget: id = 0x713, byte ext, data len = 8, stmp9: data buf
    } 

    if( AnimationValue != LastAnimationValue ) 
    {
      LastAnimationValue = AnimationValue;
      stmp12[0] = AnimationValue;
      CAN.sendMsgBuf(0x70F, 0, 2, stmp12);   // send data of the Animation: id = 0x70F, byte ext, data len = 8, stmp12: data buf
    } 
    
    if( IndicatorValue != LastIndicatorValue )
    {
      LastIndicatorValue = IndicatorValue;
      stmp[0]  = IndicatorValue;
      CAN.sendMsgBuf(0x70C, 0, 1, stmp);    // send data of the Indicator: id = 0x70C, byte ext, data len = 8, stmp: data buf 
    } 
    
    if( ToggleButtonValue != LastToggleButtonValue )
    { 
      LastToggleButtonValue = ToggleButtonValue;                
      stmp11[0]  = ToggleButtonValue;
      CAN.sendMsgBuf(0x705, 0, 2, stmp);    // send data of the ToggleButton: id = 0x705, byte ext, data len = 8, stmp: data buf     
    }
  }
  
  handleTouchEvents();
  
  if( isKnobIdle( ) == 1 )
    setKnobState( KNOB_IDLE );
    
  redrawScreen();
}
#endif


