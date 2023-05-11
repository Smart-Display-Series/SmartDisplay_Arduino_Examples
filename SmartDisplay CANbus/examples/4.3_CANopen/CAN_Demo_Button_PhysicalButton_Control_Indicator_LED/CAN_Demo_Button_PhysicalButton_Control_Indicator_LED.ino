
#include <SPI.h>
#include <cppQueue.h>
#include <arduino-timer.h>
#include <SmartDisplay.h>
#include <canfestival.h>
#include <canInit.h>
#include <mcp2515_can.h>
#include "ModelSetting.h"

#define BUTTON_PIN A0

#define NODE_ID    0x01
#define CLIENT_ID  0x7B

#define	IMPLEMENTATION	FIFO
#define MAX_QUEUE_SIZE 5

typedef enum
{
  IsWaitConnect,
  IsConnected,

} ConnectState_t;

ConnectState_t connectState = IsWaitConnect;

ConfigCmd configCmdArray[MAX_QUEUE_SIZE];

cppQueue  configCmdQueue(sizeof(ConfigCmd), MAX_QUEUE_SIZE, IMPLEMENTATION); // Instantiate queue

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
uint16_t IndicatorValue = 0;
uint16_t buttonValue = 0;
byte physicalButtonState;
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
  /*
  sprintf( prbuf, "CheckSDO Finalise %04x", nodeId );
  Serial.println(prbuf);
  */

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

  readLocalDict( &SmartDisplay_Data, Idx01_id, obj_getValue, &obj01_getValue, &expectSize, &dataType, 0 );
  readLocalDict( &SmartDisplay_Data, Idx02_id, obj_getValue, &obj02_getValue, &expectSize, &dataType, 0 );
  
  buttonValue = obj01_getValue;

  sprintf( prbuf, "obj01_getValue %03d", obj01_getValue );
  Serial.println(prbuf);
  sprintf( prbuf, "obj02_getValue %03d", obj02_getValue );
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
  
  ConfigCmd cmd;

  cmd.index    = Idx02_id;
  cmd.subIndex = obj_setValue;
  cmd.count    = 2;
  cmd.dataType = uint16;
  cmd.data = buttonValue | physicalButtonState;
  res = configCmdQueue.push( &cmd );    
  
  /*
  sprintf( prbuf, "Push Result %d, index 0x%4x subindex %02d data %03d", res, cmd.index, cmd.subIndex, cmd.data );
  Serial.println(prbuf);
  */

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
  
    /*
    sprintf( prbuf, "ConfigCmdFun nodeId :%4.4x, index %04x,subIndex %03x", CLIENT_ID, cmd.index, cmd.subIndex );
    Serial.println(prbuf);
    */
    
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

bool WaitConnect(void)
{
  if(connectState==IsConnected)
    return true;

  if( getNodeState(&SmartDisplay_Data, CLIENT_ID) == Operational ) {

    // call the scanSensor function every 100 millis
    timer.every( 100, scanSensor );
    
    connectState = IsConnected;

    Serial.println(F("State Is Operational!"));
  }
  
  return true;
}

void setup() {

  Serial.begin(115200);          // init Serial communication baud rate 
  pinMode(BUTTON_PIN, INPUT);    // init physical button as input
  pinMode(LED_BUILTIN, OUTPUT);  // init LED as output

  // call the tickHook function every 1 millis
  timer.every( 1, tickHook );
  
  while (CAN_OK != CAN.begin(CAN_250KBPS))              // init can bus : baudrate = 250k
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
  RegisterSetODentryCallBack(&SmartDisplay_Data, Idx01_id, obj_getValue, (ODCallback_t) ObjectFuncallback);
  RegisterSetODentryCallBack(&SmartDisplay_Data, Idx02_id, obj_getValue, (ODCallback_t) ObjectFuncallback);

  connectState = IsWaitConnect;
}

void loop() {

  physicalButtonState = digitalRead(BUTTON_PIN);

  if ( (physicalButtonState | buttonValue) == HIGH) {
    
      // Serial.println("Button is pressed");
      // Serial.println("LED High");
      digitalWrite(LED_BUILTIN, HIGH);  // turn the LED on by making the voltage HIGH

  }
  else {
      // Serial.println("Button is not pressed");
      // Serial.println("LED Low");
      digitalWrite(LED_BUILTIN, LOW);   // turn the LED off by making the voltage LOW
  }
  
  timer.tick(); // tick the timer

  WaitConnect();
  
  // check if data coming
  if (CAN_MSGAVAIL != CAN.checkReceive()) {
    return;
  } 
  
  Message msg;
  
  // read data, len: data length, buf: data buf
  CAN.readMsgBuf(&msg.len, msg.data);

  msg.cob_id = CAN.getCanId();
  msg.rtr    = CAN.isRemoteRequest();
  
  /*
  int i, n;
  unsigned long t = millis();
  
  n = sprintf(prbuf, "%04lu.%03d RX: [%08lX] ", t / 1000, int(t % 1000), (unsigned long)msg.cob_id );

  for (i = 0; i < msg.len; i++) {
    n += sprintf(prbuf + n, "%02X ", msg.data[i]);
  }
  Serial.println(prbuf);
  */
  
  canDispatch(&SmartDisplay_Data, &msg );
}

extern "C" void pushCan( Message *msg )
{
  /*
  unsigned long t = millis();
  int i, n;
  
  n = sprintf( prbuf, "%04lu.%03d TX: [%08lX] ", t / 1000, int(t % 1000), (unsigned long)msg->cob_id );
  
  for (i = 0; i < msg->len; i++) {
    n += sprintf(prbuf + n, "%02X ", msg->data[i]);
  }
  
  Serial.println(prbuf);
  */

  CAN.sendMsgBuf( msg->cob_id, 0, msg->len, msg->data );
}

// END FILE
