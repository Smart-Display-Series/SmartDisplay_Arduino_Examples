/*
    MIT License

    Copyright (c) 2020, Alexey Dynda

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

/*
 * @file lcd_hal/esp/esp8266_i2c.h This is ESP8266 I2C implementation
 */

#ifndef _SSD1306V2_ESP_ESP8266_I2C_H_
#define _SSD1306V2_ESP_ESP8266_I2C_H_

#if defined(CONFIG_ESP8266_I2C_AVAILABLE) && defined(CONFIG_ESP8266_I2C_ENABLE)

#include "driver/i2c.h"

/**
 * Class implements I2c interface for ESP8266 controllers
 */
class EspI2c
{
public:
    /**
     * Creates instance of I2C interface for ESP8266 controllers.
     *
     * @param busId bus number to use, -1 defaults to I2C_NUM_0
     * @param sa address to use for oled device (7-bits, highest bit should be 0)
     * @param scl pin number to use as SCL pin, or -1 if default is used
     * @param sda pin number to use as SDA pin, or -1 if default is used
     * @param frequency frequency in HZ to run i2c bus on, defaults to 400kHz
     */
    EspI2c(int8_t busId = -1, uint8_t sa = 0x00, int8_t scl = -1, int8_t sda = -1, uint32_t frequency = 400000);
    ~EspI2c();

    /**
     * Initializes i2c interface
     */
    void begin();

    /**
     * Closes i2c interface
     */
    void end();

    /**
     * Starts communication with SSD1306 display.
     */
    void start();

    /**
     * Ends communication with SSD1306 display.
     */
    void stop();

    /**
     * Sends byte to SSD1306 device
     * @param data - byte to send
     */
    void send(uint8_t data);

    /**
     * @brief Sends bytes to SSD1306 device
     *
     * Sends bytes to SSD1306 device. This functions gives
     * ~ 30% performance increase than ssd1306_intf.send.
     *
     * @param buffer - bytes to send
     * @param size - number of bytes to send
     */
    void sendBuffer(const uint8_t *buffer, uint16_t size);

    /**
     * Sets i2c address for communication
     * This API is required for some led displays having multiple
     * i2c addresses for different types of data.
     *
     * @param addr i2c address to set (7 bits)
     */
    void setAddr(uint8_t addr)
    {
        m_sa = addr;
    }

private:
    int8_t m_busId;
    uint8_t m_sa;
    int8_t m_scl;
    int8_t m_sda;
    uint32_t m_frequency;
    i2c_cmd_handle_t m_cmd_handle;
};

#endif

#endif
