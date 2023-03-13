SmartDisplay CANbus Arduino Examples
==============================================================
## Development Environment

* Arduino IDE v2.0.4

## Development Environment

* Arduino IDE v2.0.4

## Information

This Examples provide the following help:

* Description example of control gauge
* ***To be added***


This example is demonstration Arduino mega 2560 through [Canbus shield](https://www.seeedstudio.com/CAN-BUS-Shield-V2.html) control [Smart Display Modules CANbus 3.5"](https://www.winstar.com.tw/products/smart-display/can-display/wl0f00035000xgaaasc00.html).

This Examples will work with all [Smart Display CANbus Modules](https://www.winstar.com.tw/products/smart-display/can-display.html).

The demo code included with ***[CanFestival](https://canfestival.org/)*** library was made for the Smart Display CANbus, 

however is easily be adapted to other size displays.

https://user-images.githubusercontent.com/7386787/143402804-648cdb11-c7e7-4275-b308-8f758b9d63c6.mp4


## Installation

The repository should be placed in the C:\Users\(User name)\My Documents\Arduino\Libraries\, or equivalent. (restart the IDE if already open)

![image](https://user-images.githubusercontent.com/7386787/143409293-ceda724e-e889-4116-b6ae-c6e412eae84d.png)


For more information on the installation, please refer to [Installing Additional Arduino Libraries] (http://arduino.cc/en/Guide/Libraries)

## General Examples Discussion

These objects (widget) are inserted into through ***writeNetworkDictCallBack*** and update data from the ***ObjectFuncallback*** function.

```cpp

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

  readLocalDict( &SmartDisplay_Data, Idx01_id, obj_getValue, &gaugeValue, &expectSize, &dataType, 0 );

  return 0;
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
```

## Available Object Types

This section lists all of the objects/widgets that are available to be used by this Smart Display Module (CANbus),

which represent each of the widgets available to be placed using the [Smart Display GUI Builder](https://www.winstar.com.tw/technology/smart_display.html).

Please refer to the [Smart Display GUI Builder](https://www.winstar.com.tw/technology/smart_display.html) simulator feature for more information about to use each widget. 

It can display the interactive message on how to send data through CAN bus to Smart Display.

```cpp
typedef enum
{
  TYPE_EMPTY            = 0,
  TYPE_IMAGE            = 1,
  TYPE_GAUGE            = 2,
  TYPE_BDI              = 3,
  TYPE_BUTTON           = 4,
  TYPE_TOGGLE_BUTTON    = 5,
  TYPE_VerticalSlider   = 6,
  TYPE_HorizontalSlider = 7,
  TYPE_CheckButton      = 8,
  TYPE_Temperature      = 9,
  TYPE_Battery          = 10,
  TYPE_Graph            = 11,
  TYPE_Indicator        = 12,
  TYPE_CircleProgress   = 13,
  TYPE_ImageProgress    = 14,
  TYPE_GroupButton      = 15,
  TYPE_AnimatedImage    = 16,
  TYPE_NumberStr        = 17,
  TYPE_TextStr          = 18,
  TYPE_CustomWidget     = 19,
  TYPE_DigitalClock     = 20,
  TYPE_Max,
} OBJECT_TYPE;
```

