/*
    MIT License

    Copyright (c) 2017-2021, Alexey Dynda

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
*/
/**
 *   Attiny85 PINS (i2c)
 *             ____
 *   RESET   -|_|  |- 3V
 *   SCL (3) -|    |- (2)
 *   SDA (4) -|    |- (1)
 *   GND     -|____|- (0)
 *
 *   Attiny SPI PINS:     connect LCD to D4 (D/C), GND (CS), D3 (RES), D1(DIN), D2(CLK)
 *
 *   Nano/Atmega328 PINS: connect LCD to A4/A5 (i2c)
 *   ESP8266: GPIO4(SDA) / GPIO5( SCL )
 *   STM32: B7(SDA), B6(SCL)
 */

#include "lcdgfx.h"
#include "lcdgfx_gui.h"
#include "owl.h"

DisplaySSD1306_128x64_I2C display(-1); // or (-1,{busId, addr, scl, sda, frequency}). This line is suitable for most platforms by default
// The parameters are  RST pin, BUS number, CS pin, DC pin, FREQ (0 means default), CLK pin, MOSI pin
//DisplaySSD1306_128x64_SPI display(-1,{-1, 0, 1, 0, -1, -1); // Use this line for nano pi (RST not used, 0=CE, gpio1=D/C)
//DisplaySSD1306_128x64_SPI display(3,{-1, 4, 5, 0,-1,-1});   // Use this line for Atmega328p (3=RST, 4=CE, 5=D/C)
//DisplaySSD1306_128x64_SPI display(24,{-1, 0, 23, 0,-1,-1}); // Use this line for Raspberry  (gpio24=RST, 0=CE, gpio23=D/C)
//DisplaySSD1306_128x64_SPI display(22,{-1, 5, 21, 0,-1,-1}); // Use this line for ESP32 (VSPI)  (gpio22=RST, gpio5=CE for VSPI, gpio21=D/C)
//DisplaySSD1306_128x64_SPI display(4,{-1, -1, 5, 0,-1,-1});  // Use this line for ESP8266 Arduino style rst=4, CS=-1, DC=5
                                                            // And ESP8266 RTOS IDF. GPIO4 is D2, GPIO5 is D1 on NodeMCU boards

/*
 * Heart image below is defined directly in flash memory.
 * This reduces SRAM consumption.
 * The image is defined from bottom to top (bits), from left to
 * right (bytes).
 */
const PROGMEM uint8_t heartImage[8] =
{
    0B00001110,
    0B00011111,
    0B00111111,
    0B01111110,
    0B01111110,
    0B00111101,
    0B00011001,
    0B00001110
};

/*
 * Define sprite width. The width can be of any size.
 * But sprite height is always assumed to be 8 pixels
 * (number of bits in single byte).
 */
const int spriteWidth = sizeof(heartImage);

const char *menuItems[] =
{
    "draw bitmap",
    "sprites",
    "fonts",
    "canvas gfx",
    "draw lines",
};

LcdGfxMenu menu( menuItems, sizeof(menuItems) / sizeof(char *) );

static void bitmapDemo()
{
    display.drawBitmap1(0, 0, 128, 64, Owl);
    lcd_delay(1000);
    display.getInterface().invertMode();
    lcd_delay(2000);
    display.getInterface().normalMode();
}

static void spriteDemo()
{
    display.clear();
    /* Declare variable that represents our sprite */
    NanoPoint sprite = {0, 0};
    for (int i=0; i<250; i++)
    {
        lcd_delay(15);
        /* Erase sprite on old place. The library knows old position of the sprite. */
        display.setColor( 0 );
        display.drawBitmap1( sprite.x, sprite.y, spriteWidth, 8, heartImage );
        sprite.x++;
        if (sprite.x >= display.width())
        {
            sprite.x = 0;
        }
        sprite.y++;
        if (sprite.y >= display.height())
        {
            sprite.y = 0;
        }
        /* Draw sprite on new place */
        display.setColor( 0xFFFF );
        display.drawBitmap1( sprite.x, sprite.y, spriteWidth, 8, heartImage );
    }
}

static void textDemo()
{
    display.setFixedFont( ssd1306xled_font6x8 );
    display.clear();
    display.printFixed(0,  8, "Normal text", STYLE_NORMAL);
    display.printFixed(0, 16, "Bold text", STYLE_BOLD);
    display.printFixed(0, 24, "Italic text", STYLE_ITALIC);
    display.invertColors();
    display.printFixed(0, 32, "Inverted bold", STYLE_BOLD);
    display.invertColors();
    lcd_delay(3000);
}

#if !defined(__AVR_ATtiny85__) && !defined(__AVR_ATtiny84__)
static void canvasDemo()
{
    NanoCanvas<64,16,1> canvas;
    display.setFixedFont( ssd1306xled_font6x8 );
    display.clear();
    canvas.clear();
    canvas.setColor( 0xFF );
    canvas.fillRect(10, 3, 80, 5);
    display.drawCanvas((display.width()-64)/2, 1, canvas);
    lcd_delay(500);
    canvas.setColor( 0xFF );
    canvas.fillRect(50, 1, 60, 15);
    display.drawCanvas((display.width()-64)/2, 1, canvas);
    lcd_delay(1500);
    canvas.setFixedFont( ssd1306xled_font6x8 );
    canvas.printFixed(20, 1, " DEMO ", STYLE_BOLD );
    display.drawCanvas((display.width()-64)/2, 1, canvas);
    lcd_delay(3000);
}
#endif

static void drawLinesDemo()
{
    display.clear();
    for (uint8_t y = 0; y < display.height(); y += 8)
    {
        display.drawLine(0,0, display.width() -1, y);
    }
    for (uint8_t x = display.width() - 1; x > 7; x -= 8)
    {
        display.drawLine(0,0, x, display.height() - 1);
    }
    lcd_delay(3000);
}

void setup()
{
    /* Select the font to use with menu and all font functions */
    display.setFixedFont( ssd1306xled_font6x8 );

    display.begin();

    /* Uncomment 2 lines below to rotate your ssd1306 display by 180 degrees. */
    // display.getInterface().flipVertical();
    // display.getInterface().flipHorizontal();

    display.clear();
    menu.show( display );
}

void loop()
{
    lcd_delay(1000);
    switch (menu.selection())
    {
        case 0:
            bitmapDemo();
            break;

        case 1:
            spriteDemo();
            break;

        case 2:
            textDemo();
            break;

#if !defined(__AVR_ATtiny85__) && !defined(__AVR_ATtiny84__)
        case 3:
            canvasDemo();
            break;
#endif

        case 4:
            drawLinesDemo();
            break;

        default:
            break;
    }
    display.clear();
    menu.show( display );
    lcd_delay(500);
    menu.down();
    menu.show( display );
}
