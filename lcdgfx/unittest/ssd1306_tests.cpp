/*
    MIT License

    Copyright (c) 2019, Alexey Dynda

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

#include <CppUTest/TestHarness.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <vector>
#include "lcdgfx.h"
#include "sdl_core.h"
#include "utils/utils.h"
#include "ssd1306_data.h"


TEST_GROUP(SSD1306)
{
    void setup()
    {
        // ...
    }

    void teardown()
    {
        // ...
    }
};

//DisplaySSD1306_128x64_SPI display(-1,{-1, 0, 1, 0, -1, -1); // Use this line for nano pi (RST not used, 0=CE, gpio1=D/C)
//DisplaySSD1306_128x64_SPI display(3,{-1, 4, 5, 0,-1,-1});   // Use this line for Atmega328p (3=RST, 4=CE, 5=D/C)
//DisplaySSD1306_128x64_SPI display(24,{-1, 0, 23, 0,-1,-1}); // Use this line for Raspberry  (gpio24=RST, 0=CE, gpio23=D/C)
//DisplaySSD1306_128x64_SPI display(22,{-1, 5, 21, 0,-1,-1}); // Use this line for ESP32 (VSPI)  (gpio22=RST, gpio5=CE for VSPI, gpio21=D/C)


TEST(SSD1306, monochrome_test)
{
    DisplaySSD1306_128x64_I2C display(-1);
    display.begin();
    display.clear();
    display.setFixedFont(ssd1306xled_font6x8);
    display.printFixed (0,  8, "Line 1. Normal text", STYLE_NORMAL);
    display.printFixed (0, 16, "Line 2. Bold text", STYLE_BOLD);
    display.printFixed (0, 24, "Line 3. Italic text", STYLE_ITALIC);
    display.printFixedN (0, 32, "Line 4. Double size", STYLE_BOLD, FONT_SIZE_2X);

    std::vector<uint8_t> pixels( sdl_core_get_pixels_len( 1 ), 0 );
    sdl_core_get_pixels_data( pixels.data(), 1 );
//    print_buffer_data( pixels.data(), sdl_core_get_pixels_len( 1 ), 1, 128 );
//    print_screen_content( pixels.data(), sdl_core_get_pixels_len( 1 ), 1, 128 );

    CHECK_EQUAL( sizeof(monochrome_test_data), pixels.size() );
    MEMCMP_EQUAL( monochrome_test_data, pixels.data(), sizeof(monochrome_test_data));

    display.end();
}

