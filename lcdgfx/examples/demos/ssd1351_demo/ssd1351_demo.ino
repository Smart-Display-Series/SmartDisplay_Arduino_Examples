/*
    MIT License

    Copyright (c) 2019-2020, Alexey Dynda

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
 *   Nano/Atmega328 PINS: connect LCD to D5 (D/C), D4 (CS), D3 (RES), D11(DIN), D13(CLK)
 *   Attiny SPI PINS:     connect LCD to D4 (D/C), GND (CS), D3 (RES), D1(DIN), D2(CLK)
 *   ESP8266: connect LCD to D1(D/C), D2(CS), RX(RES), D7(DIN), D5(CLK)
 */

/* !!! THIS DEMO RUNS in FULL COLOR MODE */

#include "lcdgfx.h"
#include "lcdgfx_gui.h"
#include "owl.h"

// The parameters are  RST pin, BUS number, CS pin, DC pin, FREQ (0 means default), CLK pin, MOSI pin
DisplaySSD1351_128x128x16_SPI display(3,{-1, 4, 5, 0,-1,-1}); // Use this line for Atmega328p
//DisplaySSD1351_128x128x16_SPI display(22,{-1, 5, 21, 0,-1,-1}); // Use this line for ESP32 (VSPI)  (gpio22=RST, gpio5=CE for VSPI, gpio21=D/C
//DisplaySSD1351_128x128x16_SPI display(24,{-1, 0, 23, 0,-1,-1}); // Use this line for Raspberry Pi
//DisplaySSD1351_128x128x16_SPI display(4,{-1, -1, 5, 0,-1,-1});  // Use this line for ESP8266 Arduino style rst=4, CS=-1, DC=5
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

const PROGMEM uint8_t heartImage8[ 8 * 8 ] =
{
    0x00, 0xE0, 0xE0, 0x00, 0x00, 0xE5, 0xE5, 0x00,
    0xE0, 0xC0, 0xE0, 0xE0, 0xE0, 0xEC, 0xEC, 0xE5,
    0xC0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE5, 0xEC, 0xE5,
    0x80, 0xC0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE5, 0xE0,
    0x00, 0x80, 0xC0, 0xE0, 0xE0, 0xE0, 0xE0, 0x00,
    0x00, 0x00, 0x80, 0xE0, 0xE0, 0xE0, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x80, 0xE0, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

const char *menuItems[] =
{
    "draw bitmap",
    "sprites",
    "fonts",
    "nano canvas",
    "draw lines",
};

LcdGfxMenu menu( menuItems, sizeof(menuItems) / sizeof(char *) );

static void bitmapDemo()
{
    display.setColor(RGB_COLOR16(64,64,255));
    display.drawBitmap1(0, 0, 128, 64, Owl);
    display.drawBitmap8(0, 0, 8, 8, heartImage8);
    display.setColor(RGB_COLOR16(255,64,64));
    display.drawBitmap1(0, 16, 8, 8, heartImage);
    lcd_delay(3000);
}

/* Sprites are not implemented for color modes.
 * But there is NanoEngine support
 * To make example clear, we use lambda as function pointer. Since lambda can be
 * passed to function only if it doesn't capture, all variables should be global.
 * Refer to C++ documentation.
 */
NanoPoint sprite;
NanoEngine16<DisplaySSD1351_128x128x16_SPI> engine( display );
static void spriteDemo()
{
    // We not need to clear screen, engine will do it for us
    engine.begin();
    // Force engine to refresh the screen
    engine.refresh();
    // Set function to draw our sprite
    engine.drawCallback( []()->bool {
        engine.getCanvas().clear();
        engine.getCanvas().setColor( RGB_COLOR16(255, 32, 32) );
        engine.getCanvas().drawBitmap1( sprite.x, sprite.y, 8, 8, heartImage );
        return true;
    } );
    sprite.x = 0;
    sprite.y = 0;
    for (int i=0; i<250; i++)
    {
        lcd_delay(15);
        // Tell the engine to refresh screen at old sprite position
        engine.refresh( sprite.x, sprite.y, sprite.x + 8 - 1, sprite.y + 8 - 1 );
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
        // Tell the engine to refresh screen at new sprite position
        engine.refresh( sprite.x, sprite.y, sprite.x + 8 - 1, sprite.y + 8 - 1 );
        // Do refresh required parts of screen
        engine.display();
    }
}

static void textDemo()
{
    display.setFixedFont(ssd1306xled_font6x8);
    display.clear();
    display.setColor(RGB_COLOR16(255,255,0));
    display.printFixed(0,  8, "Normal text", STYLE_NORMAL);
    display.setColor(RGB_COLOR16(0,255,0));
    display.printFixed(0, 16, "bold text?", STYLE_BOLD);
    display.setColor(RGB_COLOR16(0,255,255));
    display.printFixed(0, 24, "Italic text?", STYLE_ITALIC);
    display.setColor(RGB_COLOR16(255,255,255));
    display.invertColors();
    display.printFixed(0, 32, "Inverted bold?", STYLE_BOLD);
    display.invertColors();
    lcd_delay(3000);
}

static void canvasDemo()
{
    NanoCanvas<64,16,1> canvas;
    display.setColor(RGB_COLOR16(0,255,0));
    display.clear();
    canvas.clear();
    canvas.fillRect(10, 3, 80, 5);
    display.drawCanvas((display.width()-64)/2, 1, canvas);
    lcd_delay(500);
    canvas.fillRect(50, 1, 60, 15);
    display.drawCanvas((display.width()-64)/2, 1, canvas);
    lcd_delay(1500);
    canvas.setFixedFont(ssd1306xled_font6x8);
    canvas.printFixed(20, 1, " DEMO ", STYLE_BOLD );
    display.drawCanvas((display.width()-64)/2, 1, canvas);
    lcd_delay(3000);
}

static void drawLinesDemo()
{
    display.clear();
    display.setColor(RGB_COLOR16(255,0,0));
    for (uint8_t y = 0; y < display.height(); y += 8)
    {
        display.drawLine(0,0, display.width() -1, y);
    }
    display.setColor(RGB_COLOR16(0,255,0));
    for (uint8_t x = display.width() - 1; x > 7; x -= 8)
    {
        display.drawLine(0,0, x, display.height() - 1);
    }
    lcd_delay(3000);
}

uint8_t rotation = 0;

void setup()
{
    display.begin();
    display.setFixedFont(ssd1306xled_font6x8);

    display.fill( 0x0000 );
    menu.show( display );
}

void loop()
{
    lcd_delay(1000);
    switch ( menu.selection() )
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

        case 3:
            canvasDemo();
            break;

        case 4:
            drawLinesDemo();
            break;

        default:
            break;
    }
    if ((menu.size() - 1) == menu.selection())
    {
        display.getInterface().setRotation((++rotation) & 0x03);
    }
    display.fill( 0x00 );
    display.setColor(RGB_COLOR16(255,255,255));
    menu.show( display );
    lcd_delay(500);
    menu.down();
    menu.show( display );
}
