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

#include "../io.h"

#if defined(CONFIG_AVR_SPI_AVAILABLE) && defined(CONFIG_AVR_SPI_ENABLE)

#include <stdlib.h>

#define PORT_SPI PORTB
#define DDR_SPI DDRB

#if defined(__AVR_ATmega2560__) || defined(__AVR_ATmega1280__)
#define DD_MISO DDB3
#define DD_MOSI DDB2
#define DD_SCK DDB1
#define DD_SS DDB0
#else
#define DD_MISO DDB4
#define DD_MOSI DDB3
#define DD_SCK DDB5
#define DD_SS DDB2
#endif

#define DD_MISO DDB4
#define DD_MOSI DDB3
#define DD_SS DDB2
#define DD_SCK DDB5

#define SPI_CLOCK_MASK 0x03
#define SPI_2XCLOCK_MASK 0x01

static void ssd1306_spiConfigure_avr(uint32_t frequency)
{
    uint8_t clockDiv;
    uint32_t clockSetting = F_CPU / 2;
    clockDiv = 0;
    while ( (clockDiv < 6) && (frequency < clockSetting) )
    {
        clockSetting /= 2;
        clockDiv++;
    }
    if ( clockDiv == 6 )
    {
        clockDiv = 7;
    }
    // Invert the SPI2X bit
    clockDiv ^= 0x1;

    // SS pin must be HIGH, when enabling MASTER SPI mode
    // Otherwise, SPI will drop automatically to SLAVE mode
    DDR_SPI &= ~((1 << DD_SCK) | (1 << DD_SS) | (1 << DD_MOSI));
    PORT_SPI |= (1 << DD_SS);
    /* Define the following pins as output */
    DDR_SPI |= ((1 << DD_MOSI) | (1 << DD_SCK) | (1 << DD_SS));
    PORT_SPI |= (1 << DD_SS);

    SPCR = (1 << SPE) | (1 << MSTR) | (0 << CPOL) | (0 << CPHA) | ((clockDiv >> 1) & SPI_CLOCK_MASK);
    SPSR = clockDiv & SPI_2XCLOCK_MASK; // Double Clock Rate
    // Wait for some time to give SPI HW module to initialize
    lcd_delay(10);
}

AvrSpi::AvrSpi(int8_t csPin, int8_t dcPin, uint32_t frequency)
    : m_cs(csPin)
    , m_dc(dcPin)
    , m_frequency(frequency)
{
}

AvrSpi::~AvrSpi()
{
}

void AvrSpi::begin()
{
    if ( m_cs >= 0 )
        lcd_gpioMode(m_cs, LCD_GPIO_OUTPUT);
    if ( m_dc >= 0 )
        lcd_gpioMode(m_dc, LCD_GPIO_OUTPUT);
    ssd1306_spiConfigure_avr(m_frequency);
}

void AvrSpi::end()
{
}

void AvrSpi::start()
{
    if ( m_cs >= 0 )
    {
        lcd_gpioWrite(m_cs, LCD_LOW);
    }
}

void AvrSpi::stop()
{
    if ( m_cs >= 0 )
    {
        lcd_gpioWrite(m_cs, LCD_HIGH);
    }
}

void AvrSpi::send(uint8_t data)
{
    SPDR = data;
    asm volatile("nop");
    while ( (SPSR & (1 << SPIF)) == 0 )
        ;
}

void AvrSpi::sendBuffer(const uint8_t *buffer, uint16_t size)
{
    const uint8_t *p = buffer;
    while ( size-- )
    {
        SPDR = *p;
        asm volatile("nop"); // to improve speed
        while ( (SPSR & (1 << SPIF)) == 0 )
            ;
        SPDR; // read SPI input
        p++;
    }
}

#endif
