/*
    MIT License

    Copyright 2019-2022 (C) Alexey Dynda

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
/* !!! THIS FILE IS AUTO GENERATED !!! */
/**
 * @file lcd_ssd1331.h support for LCD SSD1331 display
 */

#pragma once

#include "lcd_hal/io.h"
#include "v2/lcd/lcd_common.h"
#include "v2/lcd/base/display.h"

/**
 * @ingroup LCD_INTERFACE_API_V2
 * @{
 */

/**
 * Class implements interface functions to SSD1331 displays
 */
template <class I> class InterfaceSSD1331: public I
{
public:
    /**
     * Creates instance of interface to LCD display.
     *
     * @param bits display bit mode: 8 or 16
     * @param base Reference to base class, which represents Display
     * @param dc Data/command control pin number, for i2c communication should be -1
     * @param data variable argument list, accepted by platform interface (PlatformI2c, PlatformSpi)
     */
    template <typename... Args>
    InterfaceSSD1331(uint8_t bits, NanoDisplayBase<InterfaceSSD1331<I>> &base, int8_t dc, Args &&... data)
        : I(data...)
        , m_bits(bits)
        , m_dc(dc)
        , m_base(base)
    {
    }

    /**
     * @brief Sets block in RAM of lcd display controller to write data to.
     *
     * Sets block in RAM of lcd display controller to write data to.
     * For SSD1331 it uses horizontal addressing mode, while for
     * sh1106 the function uses page addressing mode.
     * Width can be specified as 0, thus the library will set the right boundary to
     * region of RAM block to the right column of the display.
     * @param x - column (left region)
     * @param y - row (top region)
     * @param w - width of the block in pixels to control
     *
     * @warning - this function initiates session (i2c or spi) and does not close it.
     *            To close session, please, call endBlock().
     */
    void startBlock(lcduint_t x, lcduint_t y, lcduint_t w);

    /**
     * Switches to the start of next RAM page for the block, specified by
     * startBlock().
     * For SSD1331 it does nothing, while for sh1106 the function moves cursor to
     * next page.
     */
    void nextBlock();

    /**
     * Closes data send operation to lcd display.
     */
    void endBlock();

    /**
     * Enables either data or command mode on SPI bus
     * @param mode 1 to enable data mode, or 0 to enable command mode
     */
    void setDataMode(uint8_t mode);

    /**
     * Starts communication with LCD display in command mode.
     * To stop communication use m_intf.end().
     */
    void commandStart();

    /**
     * @brief Sets screen orientation (rotation)
     *
     * Sets screen orientation (rotation): 0 - normal, 1 - 90 CW, 2 - 180 CW, 3 - 270 CW
     * @param rotation - screen rotation 0 - normal, 1 - 90 CW, 2 - 180 CW, 3 - 270 CW
     */
    void setRotation(uint8_t rotation);

    /**
     * Draws line using hardware accelerator capabilities
     *
     * @param x1 x position of first point
     * @param y1 y position of first point
     * @param x2 x position of second point
     * @param y2 y position of second point
     * @param color color to draw line with (refere RGB_COLOR16 macro)
     */
    void drawLine(lcdint_t x1, lcdint_t y1, lcdint_t x2, lcdint_t y2, uint16_t color);

    /**
     * Copies block in GDRAM to new position using hardware accelerator
     * features.
     * @param left left position of block to copy
     * @param top top position of block to copy
     * @param right right position of block to copy
     * @param bottom bottom position of block to copy
     * @param newLeft new left position for block being copied
     * @param newTop new top position for block being copied
     *
     * @note This API can be used only with ssd1331 RGB oled displays
     * @note after copy command is sent, it takes some time from oled
     *       controller to complete operation. So, it is highly recommended
     *       to wait for reasonable time before send other graphics operations
     *       (for example, use 250us delay). This time is required for
     *       oled display to become ready to accept new commands.
     */
    void copyBlock(uint8_t left, uint8_t top, uint8_t right, uint8_t bottom, uint8_t newLeft, uint8_t newTop);

private:
    const uint8_t m_bits;
    const int8_t m_dc = -1;                       ///< data/command pin for SPI, -1 for i2c
    NanoDisplayBase<InterfaceSSD1331<I>> &m_base; ///< basic lcd display support interface
    uint8_t m_rotation = 0x00;                    ///< Indicates display orientation: 0, 1, 2, 3. refer to setRotation
};
/**
 * Class implements basic functions for 8-bit mode of SSD1331-based displays
 */
template <class I> class DisplaySSD1331x8: public NanoDisplayOps<NanoDisplayOps8<I>, I>
{
public:
    /**
     * Creates instance of SSD1331 controller class for 8-bit mode
     *
     * @param intf interface to use
     * @param rstPin pin to use as HW reset pin for LCD display
     */
    DisplaySSD1331x8(I &intf, int8_t rstPin)
        : NanoDisplayOps<NanoDisplayOps8<I>, I>(intf)
        , m_rstPin(rstPin)
    {
    }

protected:
    int8_t m_rstPin; ///< indicates hardware reset pin used, -1 if it is not required

    /**
     * Basic SSD1331 initialization
     */
    void begin() override;

    /**
     * Basic SSD1331 deinitialization
     */
    void end() override;
};

/**
 * Class implements basic functions for 8-bit mode of SSD1331-based displays
 */
template <class I> class DisplaySSD1331_96x64x8: public DisplaySSD1331x8<I>
{
public:
    /**
     * Creates instance of SSD1331 96x64x8 controller class for 8-bit mode
     *
     * @param intf interface to use
     * @param rstPin pin to use as HW reset pin for LCD display
     */
    DisplaySSD1331_96x64x8(I &intf, int8_t rstPin)
        : DisplaySSD1331x8<I>(intf, rstPin)
    {
    }

protected:
    /**
     * Basic SSD1331 96x64x8 initialization
     */
    void begin() override;

    /**
     * Basic SSD1331 deinitialization
     */
    void end() override;
};
/**
 * Class implements SSD1331 96x64x8 lcd display in 8 bit mode over SPI
 */
class DisplaySSD1331_96x64x8_SPI: public DisplaySSD1331_96x64x8<InterfaceSSD1331<PlatformSpi>>
{
public:
    /**
     * @brief Inits 96x64x8 lcd display over spi (based on SSD1331 controller): 8-bit mode.
     *
     * Inits 96x64x8 lcd display over spi (based on SSD1331 controller): 8-bit mode
     * @param rstPin pin controlling LCD reset (-1 if not used)
     * @param config platform spi configuration. Please refer to SPlatformSpiConfig.
     */
    explicit DisplaySSD1331_96x64x8_SPI(int8_t rstPin, const SPlatformSpiConfig &config = {-1, {-1}, -1, 0, -1, -1})
        : DisplaySSD1331_96x64x8(m_spi, rstPin)
        , m_spi(8, *this, config.dc,
                SPlatformSpiConfig{
                    config.busId, {config.cs}, config.dc, config.frequency ?: 6000000, config.scl, config.sda})
    {
    }

    /**
     * Initializes SSD1331 lcd in 8-bit mode
     */
    void begin() override;

    /**
     * Closes connection to display
     */
    void end() override;

private:
    InterfaceSSD1331<PlatformSpi> m_spi;
};

/**
 * Template class implements SSD1331 96x64x8 lcd display in 8 bit mode over custom SPI implementation
 * (user-defined spi implementation). I - user custom spi class
 */
template <class I> class DisplaySSD1331_96x64x8_CustomSPI: public DisplaySSD1331_96x64x8<InterfaceSSD1331<I>>
{
public:
    /**
     * @brief Inits 96x64x8 lcd display over spi (based on SSD1331 controller): 8-bit mode.
     *
     * Inits 96x64x8 lcd display over spi (based on SSD1331 controller): 8-bit mode
     * @param rstPin pin controlling LCD reset (-1 if not used)
     * @param dcPin pin to use as data/command control pin
     * @param data variable argument list for custom user spi interface.
     */
    template <typename... Args>
    DisplaySSD1331_96x64x8_CustomSPI(int8_t rstPin, int8_t dcPin, Args &&... data)
        : DisplaySSD1331_96x64x8<InterfaceSSD1331<I>>(m_spi, rstPin)
        , m_spi(8, *this, dcPin, data...)
    {
    }

    /**
     * Initializes SSD1331 lcd in 8-bit mode
     */
    void begin() override
    {
        m_spi.begin();
        DisplaySSD1331_96x64x8<InterfaceSSD1331<I>>::begin();
    }

    /**
     * Closes connection to display
     */
    void end() override
    {
        DisplaySSD1331_96x64x8<InterfaceSSD1331<I>>::end();
        m_spi.end();
    }

private:
    InterfaceSSD1331<I> m_spi;
};
/**
 * Class implements basic functions for 16-bit mode of SSD1331-based displays
 */
template <class I> class DisplaySSD1331x16: public NanoDisplayOps<NanoDisplayOps16<I>, I>
{
public:
    /**
     * Creates instance of SSD1331 controller class for 16-bit mode
     *
     * @param intf interface to use
     * @param rstPin pin to use as HW reset pin for LCD display
     */
    DisplaySSD1331x16(I &intf, int8_t rstPin)
        : NanoDisplayOps<NanoDisplayOps16<I>, I>(intf)
        , m_rstPin(rstPin)
    {
    }

protected:
    int8_t m_rstPin; ///< indicates hardware reset pin used, -1 if it is not required

    /**
     * Basic SSD1331 initialization
     */
    void begin() override;

    /**
     * Basic SSD1331 deinitialization
     */
    void end() override;
};

/**
 * Class implements basic functions for 16-bit mode of SSD1331-based displays
 */
template <class I> class DisplaySSD1331_96x64x16: public DisplaySSD1331x16<I>
{
public:
    /**
     * Creates instance of SSD1331 96x64x16 controller class for 16-bit mode
     *
     * @param intf interface to use
     * @param rstPin pin to use as HW reset pin for LCD display
     */
    DisplaySSD1331_96x64x16(I &intf, int8_t rstPin)
        : DisplaySSD1331x16<I>(intf, rstPin)
    {
    }

protected:
    /**
     * Basic SSD1331 96x64x16 initialization
     */
    void begin() override;

    /**
     * Basic SSD1331 deinitialization
     */
    void end() override;
};
/**
 * Class implements SSD1331 96x64x16 lcd display in 16 bit mode over SPI
 */
class DisplaySSD1331_96x64x16_SPI: public DisplaySSD1331_96x64x16<InterfaceSSD1331<PlatformSpi>>
{
public:
    /**
     * @brief Inits 96x64x16 lcd display over spi (based on SSD1331 controller): 16-bit mode.
     *
     * Inits 96x64x16 lcd display over spi (based on SSD1331 controller): 16-bit mode
     * @param rstPin pin controlling LCD reset (-1 if not used)
     * @param config platform spi configuration. Please refer to SPlatformSpiConfig.
     */
    explicit DisplaySSD1331_96x64x16_SPI(int8_t rstPin, const SPlatformSpiConfig &config = {-1, {-1}, -1, 0, -1, -1})
        : DisplaySSD1331_96x64x16(m_spi, rstPin)
        , m_spi(16, *this, config.dc,
                SPlatformSpiConfig{
                    config.busId, {config.cs}, config.dc, config.frequency ?: 6000000, config.scl, config.sda})
    {
    }

    /**
     * Initializes SSD1331 lcd in 16-bit mode
     */
    void begin() override;

    /**
     * Closes connection to display
     */
    void end() override;

private:
    InterfaceSSD1331<PlatformSpi> m_spi;
};

/**
 * Template class implements SSD1331 96x64x16 lcd display in 16 bit mode over custom SPI implementation
 * (user-defined spi implementation). I - user custom spi class
 */
template <class I> class DisplaySSD1331_96x64x16_CustomSPI: public DisplaySSD1331_96x64x16<InterfaceSSD1331<I>>
{
public:
    /**
     * @brief Inits 96x64x16 lcd display over spi (based on SSD1331 controller): 16-bit mode.
     *
     * Inits 96x64x16 lcd display over spi (based on SSD1331 controller): 16-bit mode
     * @param rstPin pin controlling LCD reset (-1 if not used)
     * @param dcPin pin to use as data/command control pin
     * @param data variable argument list for custom user spi interface.
     */
    template <typename... Args>
    DisplaySSD1331_96x64x16_CustomSPI(int8_t rstPin, int8_t dcPin, Args &&... data)
        : DisplaySSD1331_96x64x16<InterfaceSSD1331<I>>(m_spi, rstPin)
        , m_spi(16, *this, dcPin, data...)
    {
    }

    /**
     * Initializes SSD1331 lcd in 16-bit mode
     */
    void begin() override
    {
        m_spi.begin();
        DisplaySSD1331_96x64x16<InterfaceSSD1331<I>>::begin();
    }

    /**
     * Closes connection to display
     */
    void end() override
    {
        DisplaySSD1331_96x64x16<InterfaceSSD1331<I>>::end();
        m_spi.end();
    }

private:
    InterfaceSSD1331<I> m_spi;
};
#include "lcd_ssd1331.inl"

/**
 * @}
 */
