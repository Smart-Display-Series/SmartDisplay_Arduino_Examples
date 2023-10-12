/*
*   SmartDisplayModbus.h
*
*     SmartDisplay Modbus Specification
*/
#ifndef __SMARTDISPLAYMODBUS_H__
#define __SMARTDISPLAYMODBUS_H__

#define SMARTDISPLAY_SLAVE_ADDRESS 0x7B
#define SMARTDISPLAY_BAUDRATE 115200

#define SMARTDISPLAY_BUZZER_CYCLE 2100
#define SMARTDISPLAY_BUZZER_HIGH 2101
#define SMARTDISPLAY_BUZZER_LOW 2102
#define SMARTDISPLAY_BUZZER_ACTIVE 2103

#define SMARTDISPLAY_BRIGHTNESS 2111
#define SMARTDISPLAY_SET_PAGE 2112
#define SMARTDISPLAY_PAGE_COUNT 2113

#define SMARTDISPLAY_MODE 2502  // CONFIG_MODE(0), DISPLAY_MODE(1)

#define CONFIG_MODE 0
#define DISPLAY_MODE 1

#define MAX_WIDGET 64
#define WIDGET_EXTENDED 10
#define WIDGET_EXTENDED_BASE 10000

// From widget id to Modbus register index
#define SMARTDISPLAY_WIDGET_TYPE(id) (100 * id + 0 + ((id >= WIDGET_EXTENDED) ? WIDGET_EXTENDED_BASE : 0))
#define SMARTDISPLAY_WIDGET_POS_X(id) (100 * id + 2 + ((id >= WIDGET_EXTENDED) ? WIDGET_EXTENDED_BASE : 0))
#define SMARTDISPLAY_WIDGET_POS_Y(id) (100 * id + 3 + ((id >= WIDGET_EXTENDED) ? WIDGET_EXTENDED_BASE : 0))
#define SMARTDISPLAY_WIDGET_STYLE(id) (100 * id + 4 + ((id >= WIDGET_EXTENDED) ? WIDGET_EXTENDED_BASE : 0)
#define SMARTDISPLAY_WIDGET_SETVALUE(id) (100 * id + 6 + ((id >= WIDGET_EXTENDED) ? WIDGET_EXTENDED_BASE : 0))
#define SMARTDISPLAY_WIDGET_GETVALUE(id) (100 * id + 7 + ((id >= WIDGET_EXTENDED) ? WIDGET_EXTENDED_BASE : 0))
#define SMARTDISPLAY_WIDGET_TEXT(id) (100 * id + 8 + +((id >= WIDGET_EXTENDED) ? WIDGET_EXTENDED_BASE : 0))

typedef enum _WidgetType {
    Empty = 0,
    Image = 1,
    Gauge = 2,
    //BDI = 3,
    Button = 4,
    ToggleButton = 5,
    VerticalSlider = 6,
    HorizontalSlider = 7,
    CheckButton = 8,
    Temperature = 9,
    Battery = 10,
    Graph = 11,
    Indicator = 12,
    CircleProgress = 13,
    ImageProgress = 14,
    GroupButton = 15,
    AnimatedImage = 16,
    NumberStr = 17,
    Text = 18,
    CustomWidget = 19,
    DigitalClock = 20,
    PhysicalButton = 21,
    MultiState = 22,
} WidgetType;

#endif
