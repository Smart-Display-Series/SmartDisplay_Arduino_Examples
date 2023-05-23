#include <SPI.h>
#include <cppQueue.h>
#include <arduino-timer.h>
#include <SmartDisplay.h>
#include <canfestival.h>
#include <canInit.h>
#include <mcp2515_can.h>
#include "ModelSetting.h"

/*Need to make a layout and do memory mapping with the help of the GUI builder app.*/
/*A .wgb file has been added to the Arduino library to verify the icon index ID in the Arduino project. so that the correct icon index could be used in coding.*/
/*Added a video in librery to watch, how to compile, and download the sketch, and how the behavior of the Smart Display.*/

#define BUTTON_VOLTAGE A0
#define ADC_REF 5      //reference voltage of ADC is 5v.If the Vcc switch on the seeeduino
                       //board switches to 3V3, the ADC_REF should be 3.3
#define GROVE_VCC 5    //VCC of the grove interface is normally 5v
#define FULL_ANGLE 300 //full value of the rotary angle is 300 degrees

#define NODE_ID    0x01
#define CLIENT_ID  0x7B
#define CONFIG_MODE   0
#define DISPLAY_MODE  1
#define MAX_DATA_SIZE 8

#define	IMPLEMENTATION	FIFO
#define ArrayLength(x)  (sizeof(x) / sizeof(x[0]))

typedef enum
{
  IsWaitConnect,
  IsConnected,

} ConnectState_t;

ConnectState_t connectState = IsWaitConnect;

ConfigCmd configCmdArray[] = {

  {.index = INDEX_MODEVIEW, .subIndex = SUBINDEX_MODEVIEW, .count = 1, .dataType = uint8, .data = CONFIG_MODE           },  // entry config mode

  {.index = Idx01_id, .subIndex = obj_type,     .count = 1, .dataType = uint8,  .data = TYPE_GAUGE  },  // set obj type
  {.index = Idx01_id, .subIndex = obj_posX,     .count = 2, .dataType = int16,  .data = 160          },  // set obj x
  {.index = Idx01_id, .subIndex = obj_posY,     .count = 2, .dataType = int16,  .data = 40          },  // set obj y
  {.index = Idx01_id, .subIndex = obj_style,    .count = 2, .dataType = uint16, .data = 0           },  // set obj style
  {.index = Idx01_id, .subIndex = obj_setValue, .count = 2, .dataType = uint16, .data = 50          },  // set obj value

  {.index = Idx02_id, .subIndex = obj_type,     .count = 1, .dataType = uint8,  .data = TYPE_BUTTON  },  // set obj type
  {.index = Idx02_id, .subIndex = obj_posX,     .count = 2, .dataType = int16,  .data = 20          },  // set obj x
  {.index = Idx02_id, .subIndex = obj_posY,     .count = 2, .dataType = int16,  .data = 110          },  // set obj y
  {.index = Idx02_id, .subIndex = obj_style,    .count = 2, .dataType = uint16, .data = 0           },  // set obj style
  {.index = Idx02_id, .subIndex = obj_setValue, .count = 2, .dataType = uint16, .data = 50          },  // set obj value
  
  {.index = Idx03_id, .subIndex = obj_type,     .count = 1, .dataType = uint8,  .data = TYPE_Indicator  },  // set obj type empty
  {.index = Idx03_id, .subIndex = obj_posX,     .count = 2, .dataType = int16,  .data = 380          },  // set obj x
  {.index = Idx03_id, .subIndex = obj_posY,     .count = 2, .dataType = int16,  .data = 110          },  // set obj y
  {.index = Idx03_id, .subIndex = obj_style,    .count = 2, .dataType = uint16, .data = 0           },  // set obj style
  {.index = Idx03_id, .subIndex = obj_setValue, .count = 2, .dataType = uint16, .data = 50          },  // set obj value 
  
  {.index = Idx04_id, .subIndex = obj_type, .count = 1, .dataType = uint8, .data = 0  },  // set obj type empty
  {.index = Idx05_id, .subIndex = obj_type, .count = 1, .dataType = uint8, .data = 0  },  // set obj type empty
  {.index = Idx06_id, .subIndex = obj_type, .count = 1, .dataType = uint8, .data = 0  },  // set obj type empty
  {.index = Idx07_id, .subIndex = obj_type, .count = 1, .dataType = uint8, .data = 0  },  // set obj type empty
  {.index = Idx08_id, .subIndex = obj_type, .count = 1, .dataType = uint8, .data = 0  },  // set obj type empty
  {.index = Idx09_id, .subIndex = obj_type, .count = 1, .dataType = uint8, .data = 0  },  // set obj type empty
  {.index = Idx10_id, .subIndex = obj_type, .count = 1, .dataType = uint8, .data = 0  },  // set obj type empty

  {.index = INDEX_BACKGROUND, .subIndex = SUBINDEX_BACKGROUND, .count = 1, .dataType = uint8, .data = 1  },  // set background img
  
  {.index = INDEX_MODEVIEW, .subIndex = SUBINDEX_MODEVIEW, .count = 1, .dataType = uint8, .data = DISPLAY_MODE  }   // entry display mode
};

#define MAX_QUEUE_SIZE ArrayLength(configCmdArray)

cppQueue  configCmdQueue(sizeof(ConfigCmd), MAX_QUEUE_SIZE, IMPLEMENTATION); // Instantiate queue

auto timer = timer_create_default(); // create a timer with default settings

// Set SPI CS Pin according to your hardware
// For Arduino MCP2515 Hat:
const int SPI_CS_PIN = 9;
const int CAN_INT_PIN = 2;

mcp2515_can CAN(SPI_CS_PIN); // Set CS pin

void ConfigSmartDisplay(void);

/* Define the timer registers */
uint32_t TimerAlarm_CAN;
uint32_t TimerCounter_CAN;
uint32_t last_time_set = TIMEVAL_MAX;

char prbuf[64];
uint16_t gaugeValue = 0;
uint16_t IndicatorValue = 0;
uint16_t buttonValue = 0;
bool doConfig = 0;



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
	//  123
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
  sprintf( prbuf, "CheckSDO Finalise %04x", nodeId );
  Serial.println(prbuf);

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
UNS32 ObjectFuncallback ( CO_Data* d, const indextable *unsused_indextable, UNS8 unsused_bSubindex )
{
  Serial.println(F("ObjectFuncallback get value"));

  uint32_t expectSize;
  uint8_t dataType;
  
  uint16_t obj01_getValue;
  uint16_t obj02_getValue;
  uint16_t obj03_getValue;
  uint16_t obj04_getValue;
  uint16_t obj05_getValue;
  uint16_t obj06_getValue;
  uint16_t obj07_getValue;
  uint16_t obj08_getValue;
  uint16_t obj09_getValue;
  uint16_t obj10_getValue;

  readLocalDict( &SmartDisplay_Data, Idx01_id, obj_getValue, &gaugeValue, &expectSize, &dataType, 0 );
  readLocalDict( &SmartDisplay_Data, Idx02_id, obj_getValue, &IndicatorValue, &expectSize, &dataType, 0 );
  readLocalDict( &SmartDisplay_Data, Idx03_id, obj_getValue, &buttonValue, &expectSize, &dataType, 0 );
  // -------------------------------------------------------------------------------------------------------
  
  readLocalDict( &SmartDisplay_Data, Idx01_id, obj_getValue, &obj01_getValue, &expectSize, &dataType, 0 );
  readLocalDict( &SmartDisplay_Data, Idx02_id, obj_getValue, &obj02_getValue, &expectSize, &dataType, 0 );
  readLocalDict( &SmartDisplay_Data, Idx03_id, obj_getValue, &obj03_getValue, &expectSize, &dataType, 0 );
  readLocalDict( &SmartDisplay_Data, Idx04_id, obj_getValue, &obj04_getValue, &expectSize, &dataType, 0 );
  readLocalDict( &SmartDisplay_Data, Idx05_id, obj_getValue, &obj05_getValue, &expectSize, &dataType, 0 );
  readLocalDict( &SmartDisplay_Data, Idx06_id, obj_getValue, &obj06_getValue, &expectSize, &dataType, 0 );
  readLocalDict( &SmartDisplay_Data, Idx07_id, obj_getValue, &obj07_getValue, &expectSize, &dataType, 0 );
  readLocalDict( &SmartDisplay_Data, Idx08_id, obj_getValue, &obj08_getValue, &expectSize, &dataType, 0 );
  readLocalDict( &SmartDisplay_Data, Idx09_id, obj_getValue, &obj09_getValue, &expectSize, &dataType, 0 );
  readLocalDict( &SmartDisplay_Data, Idx10_id, obj_getValue, &obj10_getValue, &expectSize, &dataType, 0 );
  
  buttonValue = obj02_getValue;
  // gaugeValue = obj01_getValue;

  sprintf( prbuf, "obj01_getValue %03d", obj01_getValue );
  Serial.println(prbuf);
  sprintf( prbuf, "obj02_getValue %03d", obj02_getValue );
  Serial.println(prbuf);
  sprintf( prbuf, "obj03_getValue %03d", obj03_getValue );
  Serial.println(prbuf);
  sprintf( prbuf, "obj04_getValue %03d", obj04_getValue );
  Serial.println(prbuf);
  sprintf( prbuf, "obj05_getValue %03d", obj05_getValue );
  Serial.println(prbuf);
  sprintf( prbuf, "obj06_getValue %03d", obj06_getValue );
  Serial.println(prbuf);
  sprintf( prbuf, "obj07_getValue %03d", obj07_getValue );
  Serial.println(prbuf);
  sprintf( prbuf, "obj08_getValue %03d", obj08_getValue );
  Serial.println(prbuf);
  sprintf( prbuf, "obj09_getValue %03d", obj09_getValue );
  Serial.println(prbuf);
  sprintf( prbuf, "obj10_getValue %03d", obj10_getValue );
  Serial.println(prbuf);
  
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
 * @brief  push value of obj id 0x01 from analog pin A0
 * @param  void *
 * @retval Return true if the execute function is required for the next cycle.
 */
bool scanSensor(void *)
{
  uint16_t data;
  uint8_t res;
  
  float voltage;
  int sensor_value = analogRead(BUTTON_VOLTAGE);
  voltage = (float)sensor_value*ADC_REF/1023;
  float degrees = (voltage*FULL_ANGLE)/GROVE_VCC;

  data = map( degrees, 0, 300, 0, 100);

  if( data == gaugeValue )
    return true;

  gaugeValue = data;
  
  ConfigCmd cmd;

  cmd.index    = Idx01_id; /*Idx01_id is associated with Gauge. It could be verify from .wgb file.*/
  cmd.subIndex = obj_setValue;
  cmd.count    = 2;
  cmd.dataType = uint16;
  cmd.data = data;
  res = configCmdQueue.push( &cmd ); /*it allows to add a command to a queue that will be processed later on.*/

  cmd.index    = Idx03_id; /*Idx03_id is associated with Indicator. It could be verify from .wgb file.*/
  cmd.subIndex = obj_setValue;
  cmd.count    = 2;
  cmd.dataType = uint16;
  cmd.data = data;
  res = configCmdQueue.push( &cmd );    

  sprintf( prbuf, "Push Result %d, index 0x%4x subindex %02d data %03d", res, cmd.index, cmd.subIndex, cmd.data );
  Serial.println(prbuf);

  ConfigCmdFun();
        
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
  
    sprintf( prbuf, "ConfigCmdFun nodeId :%4.4x, index %04x,subIndex %03x", CLIENT_ID, cmd.index, cmd.subIndex );
    Serial.println(prbuf);
    
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

void ConfigSmartDisplay( void )
{
  uint8_t res;
  ConfigCmd cmd;

  for( uint8_t i = 0; i < ArrayLength(configCmdArray); i++ ) {

    cmd.index    = configCmdArray[i].index;
    cmd.subIndex = configCmdArray[i].subIndex;
    cmd.count    = configCmdArray[i].count;
    cmd.dataType = configCmdArray[i].dataType;
    cmd.data     = configCmdArray[i].data;
    res          = configCmdQueue.push( &cmd );  
  }
  
  ConfigCmdFun();
}

bool WaitConnect(void)
{
  if(connectState==IsConnected)
    return true;

  if( getNodeState(&SmartDisplay_Data, CLIENT_ID) == Operational ) {

    ConfigSmartDisplay();

    // call the scanSensor function every 100 millis
    timer.every( 100, scanSensor );
    
    connectState = IsConnected;

    Serial.println(F("State Is Operational!"));
  }
  
  return true;
}

void setup() {

  pinMode(BUTTON_VOLTAGE, INPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  Serial.begin(115200);
  while (!SERIAL_PORT_MONITOR) {}

  // call the tickHook function every 1 millis
  timer.every( 1, tickHook );
  
  while (CAN_OK != CAN.begin(CAN_250KBPS)) {             // init can bus : baudrate = 500k
    Serial.println(F("CAN init fail, retry..."));
    delay(100);
  }
  Serial.println(F("CAN init ok!"));

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
  RegisterSetODentryCallBack(&SmartDisplay_Data, Idx01_id, obj_getValue, (ODCallback_t) ObjectFuncallback);
  RegisterSetODentryCallBack(&SmartDisplay_Data, Idx02_id, obj_getValue, (ODCallback_t) ObjectFuncallback);
  RegisterSetODentryCallBack(&SmartDisplay_Data, Idx03_id, obj_getValue, (ODCallback_t) ObjectFuncallback);
  RegisterSetODentryCallBack(&SmartDisplay_Data, Idx04_id, obj_getValue, (ODCallback_t) ObjectFuncallback);
  RegisterSetODentryCallBack(&SmartDisplay_Data, Idx05_id, obj_getValue, (ODCallback_t) ObjectFuncallback);
  RegisterSetODentryCallBack(&SmartDisplay_Data, Idx06_id, obj_getValue, (ODCallback_t) ObjectFuncallback);
  RegisterSetODentryCallBack(&SmartDisplay_Data, Idx07_id, obj_getValue, (ODCallback_t) ObjectFuncallback);
  RegisterSetODentryCallBack(&SmartDisplay_Data, Idx08_id, obj_getValue, (ODCallback_t) ObjectFuncallback);
  RegisterSetODentryCallBack(&SmartDisplay_Data, Idx09_id, obj_getValue, (ODCallback_t) ObjectFuncallback);
  RegisterSetODentryCallBack(&SmartDisplay_Data, Idx10_id, obj_getValue, (ODCallback_t) ObjectFuncallback);

  connectState = IsWaitConnect;
}

void loop() {

  if(buttonValue==1) {
    digitalWrite(LED_BUILTIN, HIGH);  // turn the LED on by making the voltage HIGH
    // digitalWrite(ROTARY_ANGLE_SENSOR, HIGH);
  }
  else if(buttonValue==0){

    digitalWrite(LED_BUILTIN, LOW);   // turn the LED off by making the voltage LOW
  }
    
  
  // buttonValue = digitalWrite(TYPE_BUTTON);
  // if (LED_BUILTIN == HIGH) {
  //   // turn LED on:
  // digitalWrite(LED_BUILTIN, HIGH);
  // } else {
  //   // turn LED off:
  // digitalWrite(LED_BUILTIN, LOW);
  // }


  Message msg;

  timer.tick(); // tick the timer

  WaitConnect();
  
  // check if data coming
  if (CAN_MSGAVAIL != CAN.checkReceive()) {
    return;
  } 
  
  int i, n;

  unsigned long t = millis();
  
  // read data, len: data length, buf: data buf
  CAN.readMsgBuf(&msg.len, msg.data);

  msg.cob_id = CAN.getCanId();
  msg.rtr    = CAN.isRemoteRequest();
  n = sprintf(prbuf, "%04lu.%03d RX: [%08lX] ", t / 1000, int(t % 1000), (unsigned long)msg.cob_id );

  for (i = 0; i < msg.len; i++) {
    n += sprintf(prbuf + n, "%02X ", msg.data[i]);
  }
  Serial.println(prbuf);
  
  canDispatch(&SmartDisplay_Data, &msg );
}

extern "C" void pushCan( Message *msg )
{
  unsigned long t = millis();
  int i, n;

  n = sprintf( prbuf, "%04lu.%03d TX: [%08lX] ", t / 1000, int(t % 1000), (unsigned long)msg->cob_id );
  
  for (i = 0; i < msg->len; i++) {
    n += sprintf(prbuf + n, "%02X ", msg->data[i]);
  }
  
  Serial.println(prbuf);
  CAN.sendMsgBuf( msg->cob_id, 0, msg->len, msg->data );
}

// END FILE
