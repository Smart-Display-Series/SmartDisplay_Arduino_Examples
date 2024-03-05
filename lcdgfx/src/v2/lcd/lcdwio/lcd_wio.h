/*
    MIT License

    Copyright (c) 2021, Alexey Dynda

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
 * @file lcd_wio.h support for LCD ILI9341 display installed in wio terminal console
 */

#pragma once

#ifdef SEEED_WIO_TERMINAL

#include "lcd_hal/io.h"
#include "v2/lcd/ili9341/lcd_ili9341.h"

/**
 * @ingroup LCD_INTERFACE_API_V2
 * @{
 */

/**
 * Class implements Wio Terminal 320x240x16 lcd display in 16 bit mode over SPI
 */
class DisplayWioTerminal_320x240x16: public DisplayILI9341_240x320x16_CustomSPI<ArduinoSpi>
{
public:
    DisplayWioTerminal_320x240x16();

    /**
     * Initializes Wio Terminal display in 16-bit mode
     */
    void begin() override;

    /**
     * Closes connection to display
     */
    void end() override;

private:
};

#endif

/**
 * @}
 */
