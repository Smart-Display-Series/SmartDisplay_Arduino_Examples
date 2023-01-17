
#define INDEX_BACKGROUND 0x2100
#define SUBINDEX_BACKGROUND 0x0

#define INDEX_MODEVIEW 0x2104
#define SUBINDEX_MODEVIEW 0x1

typedef enum
{
  Idx01_id = 0x2000,
  Idx02_id = 0x2001,
  Idx03_id = 0x2002,
  Idx04_id = 0x2003,
  Idx05_id = 0x2004,
  Idx06_id = 0x2005,
  Idx07_id = 0x2006,
  Idx08_id = 0x2007,
  Idx09_id = 0x2008,
  Idx10_id = 0x2009,
  IdxChangeScreen_id = 0x2099,
  IdxBackground = 0x2100,
  IdxBacklight = 0x2101,
  IdxBuzzer_Active = 0x2102,
  IdxMax,

} Idx_t;

typedef enum
{
  TYPE_EMPTY      = 0,
  TYPE_IMAGE      = 1,
  TYPE_GAUGE      = 2,
  TYPE_BDI        = 3,
  TYPE_BUTTON     = 4,
  TYPE_TOGGLE_BUTTON    = 5,
  TYPE_VerticalSlider   = 6,
  TYPE_HorizontalSlider = 7,
  TYPE_CheckButton    = 8,
  TYPE_Temperature    = 9,
  TYPE_Battery          = 10,
  TYPE_Graph            = 11,
  TYPE_Indicator    = 12,
  TYPE_CircleProgress = 13,
  TYPE_ImageProgress  = 14,
  TYPE_GroupButton    = 15,
  TYPE_AnimatedImage  = 16,
  TYPE_NumberStr    = 17,
  TYPE_TextStr      = 18,
  TYPE_CustomWidget   = 19,
  TYPE_DigitalClock   = 20,
  TYPE_Max,
} OBJECT_TYPE;

typedef struct {

  uint16_t index;
  uint8_t subIndex;
  uint8_t count;
  uint8_t dataType;
  uint16_t data;
  
}ConfigCmd;

typedef enum
{
  obj_pSetting = 0,
  obj_type,
  obj_reserve1,
  obj_posX,
  obj_posY,
  obj_style,
  obj_reserve2,
  obj_setValue,
  obj_getValue,
  obj_buffer,

} _obj_value_item;
