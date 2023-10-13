/*
*   MultiPagesOperation
*
*       This example shows how to switch pages with buttons, use a buzzer to make sounds,
*   and present analog signals on a graph widget.
*
*       Arduino uses Serial to communicate with Modbus Shield, so if you want to display debugging messages,
*   you must use other methods, such as SoftwareSerial.
*/
#include <ModbusMaster.h>
#include <SoftwareSerial.h>     // For debug information
#include "SmartDisplayModbus.h"

/******************************************************************************
*                             Constants Definition
*******************************************************************************/
// Brightness
#define MAX_BRIGHTNESS  100
#define MIN_BRIGHTNESS  0
#define BRIGHTNESS_STEP 1      

// Page ID
#define PAGE_0 0
#define PAGE_1 1
#define PAGE_2 2

// Widget ID (The value must be consistent with the object index in GUI Builder)
#define WIDGET_P0_TEXT_TITLE 0
#define WIDGET_P0_BUTTON_NEXT 1
#define WIDGET_P0_SLIDER_BRIGHTNESS 2

#define WIDGET_P1_TEXT_TITLE 0
#define WIDGET_P1_BUTTON_NEXT 1
#define WIDGET_P1_BUTTON_PREV 2
#define WIDGET_P1_BUTTON_BUZZER_1 3
#define WIDGET_P1_BUTTON_BUZZER_2 4
#define WIDGET_P1_BUTTON_BUZZER_3 5

#define WIDGET_P2_TEXT_TITLE 0
#define WIDGET_P2_BUTTON_PREV 1
#define WIDGET_P2_GRAPH 2

#define MAX_WIDGETS_PER_PAGE 6

// I/O
#define METER_PIN A0
#define METER_READ_PERIOD 100   // milliseconds

/******************************************************************************
*                             Data Type/Structure
*******************************************************************************/
typedef void (*fnPageHandler)(bool init, uint16_t *values);

typedef struct _PageInfo {
    fnPageHandler fnHandler;
    uint16_t values[MAX_WIDGETS_PER_PAGE];
} PageInfo;

/******************************************************************************
*                              Function Declaration
*******************************************************************************/
void Page0_Handler(bool init, uint16_t *values);
void Page1_Handler(bool init, uint16_t *values);
void Page2_Handler(bool init, uint16_t *values);

/******************************************************************************
*                             Global Varables
*******************************************************************************/

// Serial for debugging
SoftwareSerial mySerial(2, 3);  // Rx, Tx 

// Modbus object
ModbusMaster _node;

// Status
uint8_t _brightness = MAX_BRIGHTNESS;  // Default value
uint8_t _curPage = -1;
uint8_t _nextPage = 0;

PageInfo _pageInfo[] = {
    { Page0_Handler },
    { Page1_Handler },
    { Page2_Handler }
};

// I/O
long _nextReadTime = 0;

/******************************************************************************
*                                   Programs 
*******************************************************************************/

/*
*   Use this function to read widget value
*/
uint16_t readWidgetValue(uint16_t id) {
    uint16_t value = 0;
    uint8_t result = _node.readHoldingRegisters(SMARTDISPLAY_WIDGET_GETVALUE(id), 1);
    if (result == _node.ku8MBSuccess) {
        value = _node.getResponseBuffer(0);
    }
    return value;
}

/*
*   Use this function to write widget value
*/
bool writeWidgetValue(uint16_t id, uint16_t value) {
    uint8_t result = _node.writeSingleRegister(SMARTDISPLAY_WIDGET_SETVALUE(id), value);
    return (value == _node.ku8MBSuccess);
}

uint8_t getPageCount() {
    return (uint8_t)_node.readHoldingRegisters(SMARTDISPLAY_PAGE_COUNT, 1);
}

bool setPageIndex(uint8_t pageIdx) {

    bool ret =  ((_node.writeSingleRegister(SMARTDISPLAY_MODE, CONFIG_MODE) == _node.ku8MBSuccess) && 
                (_node.writeSingleRegister(SMARTDISPLAY_SET_PAGE, (uint16_t)pageIdx) == _node.ku8MBSuccess) &&
                (_node.writeSingleRegister(SMARTDISPLAY_MODE, DISPLAY_MODE) == _node.ku8MBSuccess));
    
    //------- Debug Message -----------
    mySerial.print("Change to page ");
    mySerial.print(pageIdx);
    if(ret) 
        mySerial.println(" OK");
    else
        mySerial.println(" failed");
    //---------------------------------

    return ret;
}

void setNextPage(uint8_t pageIdx) {
    _nextPage = pageIdx;
}

bool setBrightness(uint8_t brightness) {
    if(brightness > MAX_BRIGHTNESS)
        brightness = MAX_BRIGHTNESS;
    else if(brightness < MIN_BRIGHTNESS)
        brightness = MIN_BRIGHTNESS;
    uint8_t value = _node.writeSingleRegister(SMARTDISPLAY_BRIGHTNESS, brightness);          // Set Brightness  
    _brightness = brightness;

    //------- Debug Message -----------
    mySerial.print("SetBrightness to ");
    mySerial.print(_brightness);
    if(value == _node.ku8MBSuccess)
        mySerial.println(" OK");
    else
        mySerial.println(" failed");  
    //---------------------------------        

    return true;
}

void Buzzer(uint8_t high) {
    static uint8_t cycle = 1;   // Repeat count
    //static uint8_t high = 10;   // Buzzer-ON counter
    static uint8_t low = 0;     // Buzzer-OFF counter
    static uint8_t active = 1;  // After power-on, the internal value of active is 1

    active = !active;           // Load parameters to BEEP if active != last active
    _node.writeSingleRegister(SMARTDISPLAY_BUZZER_CYCLE, cycle);
    _node.writeSingleRegister(SMARTDISPLAY_BUZZER_HIGH, high);
    _node.writeSingleRegister(SMARTDISPLAY_BUZZER_LOW, low);
    _node.writeSingleRegister(SMARTDISPLAY_BUZZER_ACTIVE, active);

    //----------- Debug Message -------------
    mySerial.print("Buzzer with active = ");
    mySerial.println(active);
    //--------------------------------------
}

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(METER_PIN, INPUT);

    Serial.begin(SMARTDISPLAY_BAUDRATE);
    _node.begin(SMARTDISPLAY_SLAVE_ADDRESS, Serial);
    
    mySerial.begin(38400);  // for Debug messages

    setBrightness(_brightness);
}

void loop() {
    bool newPage = false;
    if (_nextPage != _curPage) {
        newPage = setPageIndex(_nextPage);
        _curPage = _nextPage;
    }
    _pageInfo[_curPage].fnHandler(newPage, &_pageInfo[_curPage].values[0]);
    delay(10);
}

/*
*   WIDGET_P0_BUTTON_NEXT           goto PAGE_1
*   WIDGET_P0_SLIDER_BRIGHTNESS     Set brightness
*/
void Page0_Handler(bool init, uint16_t *values) {
    if (init) {
        values[WIDGET_P0_BUTTON_NEXT] = 0;                           // Default button state
        writeWidgetValue(WIDGET_P0_SLIDER_BRIGHTNESS, _brightness);  // Set slider position

        //-------------- Debug Message ---------------
        //mySerial.print("Page0 init: set slider to ");
        //mySerial.println(_brightness);
        //--------------------------------------------
    } else {
        uint16_t value = readWidgetValue(WIDGET_P0_BUTTON_NEXT);     // Read button status
        if (values[WIDGET_P0_BUTTON_NEXT] == 0 && value != 0) {      // Just pressed ?
            setNextPage(PAGE_1);                                     // goto PAGE_1
        }
        values[WIDGET_P0_BUTTON_NEXT] = value;  // Save button status

        value = readWidgetValue(WIDGET_P0_SLIDER_BRIGHTNESS);       // Read slider value
        if (value != _brightness) {                                 // Slider position changed ?
            setBrightness(value);
        }
    }
}

/*
*   WIDGET_P1_BUTTON_NEXT           goto PAGE_2
*   WIDGET_P1_BUTTON_PREV           goto PAGE_0
*   WIDGET_P2_BUTTON_BEEP
*/
void Page1_Handler(bool init, uint16_t *values) {
    if (init) {
        values[WIDGET_P1_BUTTON_NEXT] = 0;
        values[WIDGET_P1_BUTTON_PREV] = 0;
    } else {
        uint16_t value = readWidgetValue(WIDGET_P1_BUTTON_NEXT);
        if (values[WIDGET_P1_BUTTON_NEXT] == 0 && value != 0) {
            setNextPage(PAGE_2);
        }
        values[WIDGET_P1_BUTTON_NEXT] = value;

        value = readWidgetValue(WIDGET_P1_BUTTON_PREV);
        if (values[WIDGET_P1_BUTTON_PREV] == 0 && value != 0) {
            setNextPage(PAGE_0);
        }
        values[WIDGET_P1_BUTTON_PREV] = value;

        value = readWidgetValue(WIDGET_P1_BUTTON_BUZZER_1);
        if (values[WIDGET_P1_BUTTON_BUZZER_1] == 0 && value != 0) {
            Buzzer(10);
        }
        values[WIDGET_P1_BUTTON_BUZZER_1] = value;

        value = readWidgetValue(WIDGET_P1_BUTTON_BUZZER_2);
        if (values[WIDGET_P1_BUTTON_BUZZER_2] == 0 && value != 0) {
            Buzzer(30);
        }
        values[WIDGET_P1_BUTTON_BUZZER_2] = value;

        value = readWidgetValue(WIDGET_P1_BUTTON_BUZZER_3);
        if (values[WIDGET_P1_BUTTON_BUZZER_3] == 0 && value != 0) {
            Buzzer(60);
        }
        values[WIDGET_P1_BUTTON_BUZZER_3] = value;

    }
}

/*
*   WIDGET_P2_GRAPH                 render A0 input
*   WIDGET_P2_BUTTON_PREV           goto PAGE_1
*/
void Page2_Handler(bool init, uint16_t *values) {
    if (init) {
        values[WIDGET_P2_BUTTON_PREV] = 0;
    } else {
        uint16_t value = readWidgetValue(WIDGET_P2_BUTTON_PREV);
        if (values[WIDGET_P2_BUTTON_PREV] == 0 && value != 0) {
            setNextPage(PAGE_1);
        }
        values[WIDGET_P2_BUTTON_PREV] = value;

        long now = millis();
        if (now >= _nextReadTime) {
            int value = analogRead(METER_PIN);
            int vr = map(value, 0, 1023, 0, 100);

            /*
             * The Graph widget won't display the same value, so we'll give it some variation so the 
             * curve continues to advance.
             */
            static int lastVR = 0;
            if(vr == lastVR) {
                vr = vr > 1 ? vr - 1 : vr + 1;      // Make it different from last value to keep curve moving
            }
            lastVR = vr;

            writeWidgetValue(WIDGET_P2_GRAPH, vr);
            _nextReadTime = now + METER_READ_PERIOD;

            //------------------- Debug Message ----------------
            /*
            if(lastValue != value) {
                mySerial.print("Read analog value changed to ");
                mySerial.print(value);
                mySerial.print(" and mapped to ");
                mySerial.println(vr);
                lastValue = value;
            }
            */
            //---------------------------------------------------
        }
    }
}
