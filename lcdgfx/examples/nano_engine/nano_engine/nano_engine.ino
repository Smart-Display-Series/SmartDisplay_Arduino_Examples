/*
    MIT License

    Copyright (c) 2018-2019, Alexey Dynda

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
/////////////////////////////////////////////////////////////////////////
// THIS SKETCH IS FOR TEST PURPOSES ONLY
/////////////////////////////////////////////////////////////////////////
/**
 *   Nano/Atmega328 PINS: connect LCD to D5 (D/C), D4 (CS), D3 (RES), D11(DIN), D13(CLK)
 *   Attiny SPI PINS:     connect LCD to D4 (D/C), GND (CS), D3 (RES), D1(DIN), D2(CLK)
 *   ESP8266: connect LCD to D1(D/C), D2(CS), RX(RES), D7(DIN), D5(CLK)
 */

#include "lcdgfx.h"
#include "nano_bitmaps.h"

#define FRAMES_CAPTURE  128

DisplaySSD1331_96x64x8_SPI display(3, {-1, 4, 5});
NanoEngine8<DisplaySSD1331_96x64x8_SPI> engine(display);


int x = 72;
int b_x = -128;
int b_y = -128;
int textx = 10;
char bufStr[16] = {0};
uint32_t totalDuration = 0;
uint32_t frames = 0;

void displayStats()
{
    if (frames >= FRAMES_CAPTURE)
    {
        engine.getCanvas().setMode(0);
        utoa(totalDuration/frames,bufStr,10);
        engine.getCanvas().setColor(RGB_COLOR8(255,0,255));
        engine.getCanvas().setFixedFont(ssd1306xled_font6x8);
        engine.getCanvas().printFixed(0, 0, "MS: ");
        engine.getCanvas().printFixed(24, 0, bufStr);
        utoa(1000/(totalDuration/frames),bufStr,10);
        engine.getCanvas().printFixed(0, 8, "FPS: ");
        engine.getCanvas().printFixed(30, 8, bufStr);
    }
}

bool drawAll()
{
    engine.getCanvas().clear();
    engine.getCanvas().setMode(CANVAS_MODE_TRANSPARENT);
    engine.getCanvas().setColor(RGB_COLOR8(255,255,0));
    engine.getCanvas().drawRect(15,12,x,55);
    engine.getCanvas().setColor(RGB_COLOR8(64,64,64));
    engine.getCanvas().fillRect(16,13,x-1,54);
    engine.getCanvas().setColor(RGB_COLOR8(0,255,255));
    engine.getCanvas().drawBitmap1(b_x, b_y, 128, 64, Sova);
    engine.getCanvas().setColor(RGB_COLOR8(255,0,0));
    engine.getCanvas().setFixedFont(ssd1306xled_font6x8);
    engine.getCanvas().printFixed(textx, 30, "This is example of text output");
    displayStats();
    return true;
}

void setup()
{
    display.begin();
    display.setFixedFont(ssd1306xled_font6x8);

    /* Set draw callback, it will be called by engine every time, *
     * when it needs to refresh some area on the lcd display.     */
    engine.drawCallback( drawAll );

    engine.begin();
}


uint32_t lastTs;

void loop()
{
    uint32_t lastTs = lcd_millis();
    uint32_t updateDuration;
    engine.refresh();
    engine.display();
    if ( frames < FRAMES_CAPTURE ) 
    {
        totalDuration += lcd_millis() - lastTs;
        frames++;
    }
    lcd_delay(30);
    textx++; if (textx ==96) textx = -192;
    b_x++;
    b_y++;
}
