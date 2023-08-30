#include <SPI.h>
#include <cppQueue.h>
#include <arduino-timer.h>
#include <SmartDisplay.h>
#include <canfestival.h>
#include <canInit.h>
#include <mcp2515_can.h>
#include "ModelSetting.h"
#include <DFRobot_DHT11.h>

DFRobot_DHT11 DHT;
#define DHT11_PIN A2

#define BUTTON_PIN A0

#define NODE_ID    0x01
#define CLIENT_ID  0x7B

#define	IMPLEMENTATION	FIFO
#define MAX_QUEUE_SIZE 5

#define WAIT_STABLE 5

#define SUB_IDX_PAGE 2
#define SUB_IDX_MODE 1

typedef enum
{
  IsWaitConnect,
  IsConnected,

} ConnectState_t;

ConnectState_t connectState = IsWaitConnect;

cppQueue configCmdQueue(sizeof(ConfigCmd), MAX_QUEUE_SIZE, IMPLEMENTATION); // Instantiate queue

auto timer = timer_create_default(); // create a timer with default settings

// Set SPI CS Pin according to your hardware
// For Arduino MCP2515 Hat:
const int SPI_CS_PIN = 9;
const int CAN_INT_PIN = 2;

mcp2515_can CAN(SPI_CS_PIN); // Set CS pin

/* Define the timer registers */
uint32_t TimerAlarm_CAN;
uint32_t TimerCounter_CAN;
uint32_t last_time_set = TIMEVAL_MAX;

char prbuf[64];
// uint16_t GaugeValue = 0;
uint16_t IndicatorValue = 0;
uint16_t buttonValue = 0;
uint16_t SwitchValue = 0;
byte physicalButtonState;
bool doConfig = 0;
uint8_t stableCnt = WAIT_STABLE;
static uint8_t page = 0;
uint8_t BrightnessValue = 0;
int temperatureC;

// bool isChangeValue = 0;

float convertToFahrenheit(float celsius) 
{
  return (celsius * 9/5) + 32;
}

/**
 * Initializes the timer, turn on the interrupt and put the interrupt time to zero
 * 
 */
void initTimer(void)
{
  /* initial time count 0 */
  TimerCounter_CAN = 0;
  TimerAlarm_CAN = 0;
  last_time_set = TIMEVAL_MAX;
}

/**
 * @brief  setTimer
 * @param  value:Set time value 0x0000-0xffff
 * @retval NONE
 */
void setTimer(TIMEVAL value)
{
  /* Add the desired time to timer interrupt time */
  TimerAlarm_CAN = (TimerCounter_CAN + value) % TIMEVAL_MAX;
}

/**
 * @brief  getElapsedTime
 * @param  NONE
 * @retval TIMEVAL:Return current timer value
 */
TIMEVAL getElapsedTime(void)
{
  uint32_t timer = TimerCounter_CAN;	// Copy the value of the running timer

  uint32_t res = (timer >= last_time_set) ? (timer - last_time_set) : (TIMEVAL_MAX - last_time_set + timer);

  return res;
}

void dummyFunction(CO_Data *d)
{

}

/**/
static void CheckSDO(CO_Data* d, UNS8 nodeId)
{
	UNS32 abortCode;

	if(getWriteResultNetworkDict (d, nodeId, &abortCode) != SDO_FINISHED) {
    
    sprintf( prbuf, "Master : Failed in slave %2.2x, AbortCode :%4.4x \n", nodeId, abortCode );
    Serial.println(prbuf);
  }

	/* Finalise last SDO transfer with this node */
	closeSDOtransfer(&SmartDisplay_Data, nodeId, SDO_CLIENT);

  doConfig = 0;
  ConfigCmdFun();
}

/**
 * @brief  upddate the value from the local object dictionary
 * @param  OD Pointer to a CAN object data structure
 * @param  unsused_indextable
 * @param  unsused_bSubindex
 * @retval 
 */

// UNS32 ObjectFuncallback ( CO_Data* d, const indextable *unsused_indextable, UNS8 unsused_bSubindex )

UNS32 ObjectFuncallback ( CO_Data* d, const indextable *ptrTable, UNS8 bSubindex )
{
  Serial.println(F("ObjectFuncallback get value"));

  uint32_t expectSize;
  uint8_t dataType;
  
  uint16_t obj00_getValue;
  uint16_t obj01_getValue;
  uint16_t obj02_getValue;
  uint16_t obj03_getValue;

  sprintf( prbuf, "now page %02d", page );
  Serial.println(prbuf);
  
  sprintf( prbuf, "update object index 0x %x", ptrTable->index );
  Serial.println(prbuf);
  
  sprintf( prbuf, "update object sub-index 0x%x", bSubindex );
  Serial.println(prbuf);

  uint16_t* pObject  = (uint16_t*) ( ptrTable->pSubindex[bSubindex].pObject );
  
  sprintf( prbuf, "update object value %d", *pObject );
  Serial.println(prbuf);

  readLocalDict( &SmartDisplay_Data, Idx00_id, obj_getValue, &obj00_getValue, &expectSize, &dataType, 0 );
  readLocalDict( &SmartDisplay_Data, Idx01_id, obj_getValue, &obj01_getValue, &expectSize, &dataType, 0 );
  readLocalDict( &SmartDisplay_Data, Idx02_id, obj_getValue, &obj02_getValue, &expectSize, &dataType, 0 );
  readLocalDict( &SmartDisplay_Data, Idx03_id, obj_getValue, &obj03_getValue, &expectSize, &dataType, 0 );

  // sprintf( prbuf, "obj00_getValue %03d", obj00_getValue );
  // Serial.println(prbuf);
  // sprintf( prbuf, "obj01_getValue %03d", obj01_getValue );
  // Serial.println(prbuf);
  // sprintf( prbuf, "obj02_getValue %03d", obj02_getValue );
  // Serial.println(prbuf);
  // sprintf( prbuf, "obj03_getValue %03d", obj03_getValue );
  // Serial.println(prbuf);

  if(page == 0) {

    if( ptrTable->index == 0x2002 ) {

      SwitchValue = *pObject;
      Serial.println("Page 0 Switch Button Press");
      page = 1;
      switchPageFun(page);

      sprintf( prbuf, "page %02d", page );
      Serial.println(prbuf); 
    }
    else if( ptrTable->index == 0x2000 ) {

      BrightnessValue = *pObject;

      if (BrightnessValue >= 100) {

        BrightnessValue =100; 
      }
      else if (BrightnessValue < 1){
        BrightnessValue = 1;
      }

      sprintf( prbuf, "BrightnessValue %02d", BrightnessValue );
      Serial.println(prbuf);

      BrightnessChangeFcn(BrightnessValue);
    }
  }

  else if(page == 1)
  {
    if( ptrTable->index == 0x2001 ) {

      buttonValue = *pObject;
      
      Serial.println("Page 1 Button Press");
      sprintf( prbuf, "buttonValue %02d", buttonValue );
      Serial.println(prbuf); 

      IndicatorValue = *pObject;
        
      if (buttonValue == 1) {
          IndicatorValue = HIGH;
      }
      else if (buttonValue == 0) {
        IndicatorValue = LOW;
      }

      ChangeIndicatorFcn( IndicatorValue );

      sprintf( prbuf, "IndicatorValue %02d", IndicatorValue );
      Serial.println(prbuf);

    }
    else if( ptrTable->index == 0x2002 ) { 

    }

    else if( ptrTable->index == 0x2003 ) {

      SwitchValue = *pObject;
      Serial.println("Page 1 Switch Button Press");
      sprintf( prbuf, "SwitchValue %02d", SwitchValue );
      Serial.println(prbuf);

      page = 2;
      switchPageFun(page);

      sprintf( prbuf, "page %02d", page );
      Serial.println(prbuf);
    }  

  }

  else if(page == 2) 
  {
    if( ptrTable->index == 0x2003 ) {
      SwitchValue = *pObject;
      Serial.println("Page 2 Switch Button Press");
      sprintf( prbuf, "SwitchValue %02d", SwitchValue );
      Serial.println(prbuf);

      page = 0;
      switchPageFun(page);

      sprintf( prbuf, "page %02d", page );
      Serial.println(prbuf);
    }

    else if( ptrTable->index == 0x2002 ){
      temperatureC = *pObject;

    }

  }
 
  return 0;

}

bool tickHook(void *) {

  TimerCounter_CAN++;

  if (TimerCounter_CAN >= TIMEVAL_MAX) {

    TimerCounter_CAN = 0;
  }

  /* ALARM */
  if (TimerCounter_CAN == TimerAlarm_CAN) {

    last_time_set = TimerCounter_CAN;

    /* Call the time handler of the stack to adapt the elapsed time  */
    TimeDispatch();
  }

  return true;
}

/**
 * @brief  update value of object (widget) to destination node via CANopen API 
 *         if SDO thread is empty
 * @param  None
 * @retval Return true.
 */
bool ConfigCmdFun(void)
{  
  ConfigCmd cmd;
  uint8_t res;
  uint16_t data;

  if(doConfig)
    return true;
      
  if( configCmdQueue.pop(&cmd) ){
      
    data = cmd.data;
    res = writeNetworkDictCallBack ( &SmartDisplay_Data, /*CO_Data* d*/
                   /**TestSlave_Data.bDeviceNodeId, UNS8 nodeId*/
    CLIENT_ID,     /*UNS8 nodeId*/
    cmd.index,     /*UNS16 index*/
    cmd.subIndex,  /*UNS8 subindex*/
    cmd.count,     /*UNS8 count*/
    cmd.dataType,  /*UNS8 dataType*/
    (void*)&data,  /*void *data*/
    CheckSDO,      /*SDOCallback_t Callback*/
    0);            /* use block mode */

    doConfig = 1;
  } 
  
  return true;
}

void measureTemperature()
{
  DHT.read(DHT11_PIN);
  temperatureC = DHT.temperature;
  float temperatureF = convertToFahrenheit(temperatureC);
  int humidity = DHT.humidity;

  // Check if the temperature and humidity readings are valid
  if (temperatureC != -999 && humidity != -999) {

    Serial.print("Temperature (C): ");
    Serial.print(temperatureC);
    Serial.print(" Temperature (F): ");
    Serial.print(temperatureF);
    Serial.print(" Humidity (H): ");
    Serial.println(humidity);

  } else {
    Serial.println("Failed to read data from DHT sensor!");
  }

}

bool WaitConnect(void)
{
  if(connectState==IsConnected)
    return true;

  if( getNodeState(&SmartDisplay_Data, CLIENT_ID) == Operational ) {

    // call the scanSensor function every 1000 millis
    timer.every( 2000, measureTemperature );
 
    connectState = IsConnected;

    Serial.println(F("State Is Operational!"));
  }
  
  return false;
}

bool BrightnessChangeFcn( uint8_t Brightness)
{
  ConfigCmd cmd;
  bool res;

  // sprintf( prbuf, "Brightness %02d", Brightness );
  // Serial.println(prbuf);

  cmd.index    = IdxBacklight;
  cmd.subIndex = obj_pSetting;
  cmd.count    = 1;
  cmd.dataType = uint8;
  cmd.data     = Brightness;
  res = configCmdQueue.push( &cmd );

  ConfigCmdFun();

}

bool ChangeIndicatorFcn(uint8_t Indicator)
{
  ConfigCmd cmd;
  bool res;
  
  sprintf( prbuf, "Indicator change %02d", Indicator );
  Serial.println(prbuf);
  
  cmd.index    = Idx02_id;
  cmd.subIndex = obj_setValue;
  cmd.count    = 2;
  cmd.dataType = uint16;
  cmd.data     = Indicator;
  res = configCmdQueue.push( &cmd ); 

  ConfigCmdFun();
}

bool switchPageFun(uint8_t page) 
{
  ConfigCmd cmd;
  bool res;

  sprintf( prbuf, "switch page %02d", page );
  Serial.println(prbuf);

  cmd.index    = IdxMode;
  cmd.subIndex = SUB_IDX_MODE;
  cmd.count    = 1;
  cmd.dataType = uint8;
  cmd.data = 0;
  res = configCmdQueue.push( &cmd );   

  cmd.index    = IdxPage;
  cmd.subIndex = SUB_IDX_PAGE;
  cmd.count    = 1;
  cmd.dataType = uint8;
  cmd.data = page;
  res = configCmdQueue.push( &cmd );   

  cmd.index    = IdxMode;
  cmd.subIndex = SUB_IDX_MODE;
  cmd.count    = 1;
  cmd.dataType = uint8;
  cmd.data = 1;
  res = configCmdQueue.push( &cmd );    
  
  ConfigCmdFun();
}

/**
 * @brief  push value of obj id 0x01 from analog pin A0
 * @param  void *
 * @retval Return true if the execute function is required for the next cycle.
 */

bool scanSensor(void *)
{ 
  ConfigCmd cmd;
  uint8_t res;
  static uint8_t page_previous = 0; 

  if (digitalRead(BUTTON_PIN) == HIGH ) {
    
    if( stableCnt )
      stableCnt --;

    if (stableCnt == 1) {
      
      physicalButtonState == 1;
      digitalWrite(LED_BUILTIN, HIGH);  // turn the LED on by making the voltage HIGH
      Serial.println("physicalButton Press");

      page++;

      if(page>2)
        page = 0; 

      switchPageFun(page);
    }
  }

  else {
    if(stableCnt == 0) {

      Serial.println("physicalButton Release");
      digitalWrite(LED_BUILTIN, LOW);   // turn the LED off by making the voltage LOW
    }
    stableCnt = WAIT_STABLE;
  }

  /*
   if now page is same previous value 
  return
  */ 

  if( page_previous == page )
    return true;

  page_previous  = page;

  if (page == 2) {  
    cmd.index    = Idx00_id;
    cmd.subIndex = obj_setValue;
    cmd.count    = 2;
    cmd.dataType = uint16;
    cmd.data     = temperatureC;
    res = configCmdQueue.push( &cmd );

    cmd.index    = Idx02_id;
    cmd.subIndex = obj_setValue;
    cmd.count    = 2;
    cmd.dataType = uint16;
    cmd.data     = temperatureC;
    res = configCmdQueue.push( &cmd );

    Serial.println("Page 2 Temperature widgets !");
  }

  ConfigCmdFun();      
  return true;

}

void setup() {

  Serial.begin(115200);          // init Serial communication baud rate 
  pinMode(BUTTON_PIN, INPUT);    // init physical button as input
  pinMode(LED_BUILTIN, OUTPUT);  // init LED as output

  // init can bus : baudrate = 250k
  while (CAN_OK != CAN.begin(CAN_250KBPS))              
  {
    Serial.println("CAN BUS Shield init fail");
    Serial.println(" Init CAN BUS Shield again");
    delay(100);
  }
  
  Serial.println("CAN BUS Shield init ok!");

  setNodeId( &SmartDisplay_Data, NODE_ID );

  SmartDisplay_Data.heartbeatError = (heartbeatError_t) dummyFunction;
  SmartDisplay_Data.initialisation = (initialisation_t)dummyFunction;
  SmartDisplay_Data.preOperational = (preOperational_t) dummyFunction;
  SmartDisplay_Data.operational = (operational_t) dummyFunction;
  SmartDisplay_Data.stopped = (stopped_t) dummyFunction;
  SmartDisplay_Data.post_sync = dummyFunction;
  SmartDisplay_Data.post_TPDO = dummyFunction;
  SmartDisplay_Data.storeODSubIndex = (storeODSubIndex_t) dummyFunction;
  SmartDisplay_Data.post_emcy = (post_emcy_t) dummyFunction;

  setState(&SmartDisplay_Data, Operational);

  Serial.println("RegisterSetODentryCallBack");
  // set value change callback function 
  RegisterSetODentryCallBack(&SmartDisplay_Data, Idx00_id, obj_getValue, (ODCallback_t) ObjectFuncallback);
  RegisterSetODentryCallBack(&SmartDisplay_Data, Idx01_id, obj_getValue, (ODCallback_t) ObjectFuncallback);
  RegisterSetODentryCallBack(&SmartDisplay_Data, Idx02_id, obj_getValue, (ODCallback_t) ObjectFuncallback);
  RegisterSetODentryCallBack(&SmartDisplay_Data, Idx03_id, obj_getValue, (ODCallback_t) ObjectFuncallback);

  connectState = IsWaitConnect;
  
  while( connectState == IsWaitConnect ) {
    
    delay(100);
    
    Message msg = {0};
  
    // read data, len: data length, buf: data buf
    if( CAN.readMsgBuf(&msg.len, msg.data) == CAN_OK ) {

      msg.cob_id = CAN.getCanId();
      msg.rtr    = CAN.isRemoteRequest();

      // Update the CANopen status of itself when receiving a CAN message.
      canDispatch(&SmartDisplay_Data, &msg );
    }
    WaitConnect();
  }

  /* 
  Register a function to be executed with a time interval parameter 
  for executing this function.
  Execute the "scanSensor" function every 100 milliseconds.
  */
  timer.every( 20, scanSensor );

  // call the tickHook function every 1 millis
  timer.every( 1, tickHook );
}

// float convertToFahrenheit(float celsius) 
// {
//   return (celsius * 9/5) + 32;
// }

void loop() {

  timer.tick(); // tick the timer
  
  Message msg = {0};
  
  // read data, len: data length, buf: data buf
  if( CAN.readMsgBuf(&msg.len, msg.data) == CAN_OK ) {

    msg.cob_id = CAN.getCanId();
    msg.rtr    = CAN.isRemoteRequest();
    
     // Update the CANopen status of itself when receiving a CAN message.
    canDispatch(&SmartDisplay_Data, &msg );
  }

}

extern "C" void pushCan( Message *msg )
{
  CAN.sendMsgBuf( msg->cob_id, 0, msg->len, msg->data );
}

// END FILE
