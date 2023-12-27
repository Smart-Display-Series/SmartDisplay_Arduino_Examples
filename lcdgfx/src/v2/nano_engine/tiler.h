/*
    MIT License

    Copyright (c) 2018-2021, Alexey Dynda

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
 * @file tiler.h Tiler helper for graphics processing
 */

#ifndef _NANO_ENGINE_TILER_H_
#define _NANO_ENGINE_TILER_H_

#include "canvas/rect.h"
#include "canvas/canvas.h"
#include "v2/lcd/base/display.h"

/**
 * @ingroup NANO_ENGINE_API_V2
 * @{
 */

#ifndef NE_MAX_TILE_ROWS
#define NE_MAX_TILE_ROWS 20 ///< Maximum tile rows supported. Can be defined outside the library
#endif

/**
 * Structure, holding currently set font.
 * @warning Only for internal use.
 */

/* The table below defines arguments for NanoEngineTiler.          *
 *                            canvas                    bits   */
// Tiles for monochrome displays
#define TILE_128x64_MONO NanoCanvas<128, 64, 1> ///< Full-screen 1-bit tile for SSD1306
#define TILE_8x8_MONO NanoCanvas<8, 8, 1>       ///< Standard 1-bit tile 8x8 for monochrome mode
#define TILE_16x16_MONO NanoCanvas<16, 16, 1>   ///< Standard 1-bit tile 16x16 for monochrome mode
#define TILE_32x32_MONO NanoCanvas<32, 32, 1>   ///< Standard 1-bit tile 32x32 for monochrome mode
// Tiles for 4-bit displays
#define TILE_16x16_GRAY4 NanoCanvas<16, 16, 4> ///< Standard 4-bit tile 16x16 for gray-color displays
// Tiles for 8-bit displays
#define TILE_8x8_RGB8 NanoCanvas<8, 8, 8>       ///< Standard 8-bit RGB tile 8x8
#define TILE_16x16_RGB8 NanoCanvas<16, 16, 8>   ///< Standard 8-bit RGB tile 16x16
#define TILE_32x32_RGB8 NanoCanvas<32, 32, 8>   ///< Standard 8-bit RGB tile 32x32
#define TILE_8x8_MONO_8 NanoCanvas<8, 8, 1>     ///< Standard 1-bit tile 8x8 for RGB mode
#define TILE_16x16_MONO_8 NanoCanvas<16, 16, 1> ///< Standard 1-bit tile 16x16 for RGB mode
// Tiles for 16-bit displays
#define TILE_8x8_RGB16 NanoCanvas<8, 8, 16>     ///< Standard 16-bit RGB tile 8x8
#define TILE_16x16_RGB16 NanoCanvas<16, 16, 16> ///< Standard 16-bit RGB tile 16x16
// Adafruit tiles
#define ADATILE_8x8_MONO AdafruitCanvas1, 8, 8, 3   ///< Use Adafruit GFX implementation as NanoEngine canvas
#define ADATILE_8x8_RGB8 AdafruitCanvas8, 8, 8, 3   ///< Use Adafruit GFX implementation as NanoEngine canvas
#define ADATILE_8x8_RGB16 AdafruitCanvas16, 8, 8, 3 ///< Use Adafruit GFX implementation as NanoEngine canvas

/**
 * Type of user-specified draw callback.
 */
typedef bool (*TNanoEngineOnDraw)(void);

template <class C, class D> class NanoEngine;

template <class C, class D> class NanoEngineTiler;

/**
 * Template class for all NanoEngine objects
 */
template <class T> class NanoEngineObject
{
public:
    template <class C, class D> friend class NanoEngineTiler;

    NanoEngineObject() = default;

    /**
     * draw() method is called by NanoEngine, when it needs to refresh
     * object graphics.
     */
    virtual void draw() = 0;

    /**
     * update() method is called by NanoEngine, when it needs all objects
     * to update their logic state.
     */
    virtual void update() = 0;

    /**
     * update() method is called by NanoEngine, when it needs object to
     * mark occupied place for refreshing.
     */
    virtual void refresh() = 0;

    /**
     * Sets logic focus on NanoEngineObject
     */
    void focus()
    {
        m_focused = true;
        refresh();
    }

    /**
     * Clears logic focus from NanoEngineObject
     */
    void defocus()
    {
        m_focused = false;
        refresh();
    }

    /**
     * Returns true if logic focus points to the object.
     * At present NanoEngine allows many objects to be in focus at once
     */
    bool isFocused()
    {
        return m_focused;
    }

    /**
     * Returns true if NanoEngineObject is bound to NanoEngine instance
     */
    bool hasTiler()
    {
        return m_tiler != nullptr;
    }

    /**
     * Returns reference to NanoEngine. Before call to this function,
     * please check that NanoEngine is valid via hasTiler() function.
     */
    T &getTiler()
    {
        return *(static_cast<T *>(m_tiler));
    }

protected:
    T *m_tiler = nullptr;                  ///< Active tiler, assigned to the NanoEngineObject
    NanoEngineObject<T> *m_next = nullptr; ///< Next NanoEngineObject in the list

    /**
     * Bind NanoEngineObject to specific NanoEngine
     *
     * @param tiler pointer to NanoEngine object
     */
    void setTiler(T *tiler)
    {
        m_tiler = tiler;
    }

private:
    bool m_focused = false;
};

/**
 * This class template is responsible for holding and updating data about areas to be refreshed
 * on LCD display. It accepts canvas class, tile width in pixels, tile height in pixels and
 * number of bits in tile width as arguments for the template.
 * For example, for 8x8 8-bit RGB tiles the reference should be NanoEngineTiler<NanoCanvas8,8,8,3>,
 * and 3 bits means 3^2 = 8.
 * If you need to have single big buffer, holding the whole content for monochrome display,
 * you can specify something like this NanoEngineTiler<NanoCanvas1,128,64,7>.
 */
template <class C, class D> class NanoEngineTiler
{
protected:
    /** Only child classes can initialize the engine */
    explicit NanoEngineTiler(D &display)
        : m_display(display)
        , m_onDraw(nullptr)
        , offset{0, 0}
        , m_first(nullptr)
    {
        refresh();
    };

public:
    /**
     * This type is template argument for all Nano Objects.
     */
    typedef NanoEngineTiler<C, D> TilerT;
    /**
     * Marks all tiles for update. Actual update will take place in display() method.
     */
    void refresh()
    {
        memset(m_refreshFlags, 0xFF, sizeof(m_refreshFlags));
    }

    /**
     * Mark specified area in pixels for redrawing by NanoEngine.
     * Actual update will take place in display() method.
     * @note assumes that rect is in local screen coordinates
     */
    void refresh(const NanoRect &rect)
    {
        refresh(rect.p1.x, rect.p1.y, rect.p2.x, rect.p2.y);
    }

    /**
     * Mark specified area in pixels for redrawing by NanoEngine.
     * Actual update will take place in display() method.
     */
    void refresh(const NanoPoint &point) __attribute__((noinline))
    {
        if ( (point.x < 0) || (point.y < 0) || ((point.y / canvas.height()) >= NE_MAX_TILE_ROWS) )
            return;
        m_refreshFlags[(point.y / canvas.height())] |= (1 << (point.x / canvas.width()));
    }

    /**
     * Mark specified area in pixels for redrawing by NanoEngine.
     * Actual update will take place in display() method.
     */
    void refresh(lcdint_t x1, lcdint_t y1, lcdint_t x2, lcdint_t y2) __attribute__((noinline))
    {
        if ( y2 < 0 || x2 < 0 )
            return;
        if ( y1 < 0 )
            y1 = 0;
        if ( x1 < 0 )
            x1 = 0;
        y1 = y1 / canvas.height();
        y2 = min((y2 / canvas.height()), NE_MAX_TILE_ROWS - 1);
        for ( uint8_t x = x1 / canvas.width(); x <= (x2 / canvas.width()); x++ )
        {
            for ( uint8_t y = y1; y <= y2; y++ )
            {
                m_refreshFlags[y] |= (1 << x);
            }
        }
    }

    /**
     * Marks for refresh lcd area, which corresponds to specified rectangle in
     * global (World) coordinates. If engine offset is (0,0), then this function
     * refreshes the same area as refresh().
     */
    void refreshWorld(const NanoRect &rect)
    {
        refreshWorld(rect.p1.x, rect.p1.y, rect.p2.x, rect.p2.y);
    }

    /**
     * Marks for refresh lcd area, which corresponds to specified rectangle in
     * global (World) coordinates. If engine offset is (0,0), then this function
     * refreshes the same area as refresh().
     */
    void refreshWorld(lcdint_t x1, lcdint_t y1, lcdint_t x2, lcdint_t y2) __attribute__((noinline))
    {
        refresh(x1 - offset.x, y1 - offset.y, x2 - offset.x, y2 - offset.y);
    }

    /**
     * Marks specified pixel area for redrawing by NanoEngine.
     * @param point point in global (World) coordinates
     */
    void refreshWorld(const NanoPoint &point)
    {
        refresh(point - offset);
    }

    /**
     * Switches engine canvas to local coordinates system. This method can be useful
     * to ease up drawing of some static elements on lcd display.
     * @warning do not call twice subsequentally.
     */
    void localCoordinates()
    {
        canvas.offset -= offset;
    }

    /**
     * Switches engine canvas to global (World) coordinates system. This method can be useful
     * to create screen moving animation.
     * @warning do not call twice subsequentally.
     */
    void worldCoordinates()
    {
        canvas.offset += offset;
    }

    /**
     * Moves engine coordinate to new position (this sets World coordinates offset).
     */
    void moveTo(const NanoPoint &position)
    {
        offset = position;
    }

    /**
     * Moves engine coordinate to new position and mark whole display for refresh
     * (this sets World coordinates offset).
     */
    void moveToAndRefresh(const NanoPoint &position)
    {
        moveTo(position);
        refresh();
    }

    /**
     * Returns current World offset
     */
    const NanoPoint &getPosition() const
    {
        return offset;
    }

    /**
     * Sets user-defined draw callback. This callback will be called everytime, engine needs
     * to update display content. If user callback returns false, engine will not update that area.
     * You always have a way to find out, which area is being updated by engine via
     * NanoEngine<>::canvas::getOffset() and NanoEngine<>::NE_TILE_SIZE.
     * @warning   By default canvas in the engine is initialized with world coordinates. So
     *            graphics object with [0,0] coordinates will be placed at topleft position fo the window
     *            pointed by offset property of NanoEngine. But engine supports also local coordinates,
     *            if you need to draw some dialog, for example. In this case actual object
     *            position depends on current engine offset. Refer to worldCoordinates() and
     *            localCoordinates().
     * @param callback - user-defined draw callback.
     * @note you can change draw callback anytime you need.
     * @see worldCoordinates()
     * @see localCoordinates()
     */
    void drawCallback(TNanoEngineOnDraw callback)
    {
        m_onDraw = callback;
    }

    /**
     * @brief Returns true if point is inside the rectangle area.
     * Returns true if point is inside the rectangle area.
     * @param p - point to check
     * @param rect - rectangle, describing the region to check with the point
     * @returns true if point is inside the rectangle area.
     */
    bool collision(const NanoPoint &p, const NanoRect &rect)
    {
        return rect.collision(p);
    }

    /**
     * Inserts new NanoEngineObject. This object will be displayed during next call
     * to update()/draw() methods.
     *
     * @param object reference to object to place to NanoEngine
     */
    void insert(NanoEngineObject<TilerT> &object) __attribute__((noinline))
    {
        object.m_next = this->m_first;
        object.setTiler(this);
        m_first = &object;
        object.refresh();
    }

    /**
     * Removes NanoEngineObject from the list, but doesn't destroy the object.
     * The place occupied by this object will be refreshed during next call
     * to update()/draw() methods.
     *
     * @param object object to remove from NanoEngine
     */
    void remove(NanoEngineObject<TilerT> &object) __attribute__((noinline))
    {
        if ( this->m_first == nullptr )
        {
        }
        else if ( &object == m_first )
        {
            object.refresh();
            this->m_first = object.m_next;
            object.m_next = nullptr;
            object.m_tiler = nullptr;
        }
        else
        {
            NanoEngineObject<TilerT> *p = this->m_first;
            while ( p->m_next )
            {
                if ( p->m_next == &object )
                {
                    object.refresh();
                    p->m_next = object.m_next;
                    object.m_next = nullptr;
                    object.m_tiler = nullptr;
                    break;
                }
                p = p->m_next;
            }
        }
    }

    /**
     * Updates all objects. This method doesn't refresh screen content
     */
    void update() __attribute__((noinline))
    {
        NanoEngineObject<TilerT> *p = m_first;
        while ( p )
        {
            p->update();
            p = p->m_next;
        }
    }

    /**
     * Returns canvas, used by the NanoEngine.
     */
    C &getCanvas()
    {
        return canvas;
    }

    /**
     * Returns reference to display object.
     */
    D &getDisplay()
    {
        return m_display;
    }

protected:
    /**
     * Reference to display object, used by NanoEngine
     */
    D &m_display;

    /** Callback to call if specific tile needs to be updated */
    TNanoEngineOnDraw m_onDraw;

    /**
     * Contains information on tiles to be updated.
     * Elements of array are rows and bits are columns.
     */
    uint16_t m_refreshFlags[NE_MAX_TILE_ROWS];

    /**
     * @brief refreshes content on oled display.
     * Refreshes content on oled display. Call it, if you want to update the screen.
     * Engine will update only those areas, which are marked by refresh()
     * methods.
     */
    void displayBuffer() __attribute__((noinline));

    /**
     * @brief prints popup message over display content
     * prints popup message over display content
     * @param msg - message to display
     */
    void displayPopup(const char *msg);

private:
    /** object, representing canvas. Use it in your draw handler */
    C canvas;

    NanoPoint offset;

    NanoEngineObject<TilerT> *m_first = nullptr;

    void draw() __attribute__((noinline))
    {
        NanoEngineObject<TilerT> *p = m_first;
        while ( p )
        {
            p->draw();
            p = p->m_next;
        }
    }
};

template <class C, class D> void NanoEngineTiler<C, D>::displayBuffer()
{
    //    printf("--------------\n");
    for ( lcduint_t y = 0; y < m_display.height(); y = y + canvas.height() )
    {
        uint16_t flag = m_refreshFlags[y / canvas.height()];
        m_refreshFlags[y / canvas.height()] = 0;
        //        printf("|%d%d%d%d%d%d%d%d|\n", flag & 1, (flag >> 1) & 1, (flag >> 2) & 1, (flag >> 3) & 1, (flag >>
        //        4) & 1, (flag >> 5) & 1,(flag >> 6) & 1,(flag >> 7) & 1 );
        for ( lcduint_t x = 0; x < m_display.width(); x = x + canvas.width() )
        {
            if ( flag & 0x01 )
            {
                canvas.setOffset(x + offset.x, y + offset.y);
                if ( m_onDraw == nullptr )
                {
                    canvas.clear();
                    draw();
                    this->m_display.drawCanvas(x, y, canvas);
                }
                else if ( m_onDraw() )
                {
                    draw();
                    this->m_display.drawCanvas(x, y, canvas);
                }
            }
            flag >>= 1;
        }
    }
}

template <class C, class D> void NanoEngineTiler<C, D>::displayPopup(const char *msg)
{
    lcduint_t height = 0;
    lcduint_t width = m_display.getFont().getTextSize(msg, &height);
    NanoRect rect = {
        {((m_display.width() - (lcdint_t)width) >> 1) - 8, ((m_display.height() - (lcdint_t)height) >> 1) - 4},
        {((m_display.width() + (lcdint_t)width) >> 1) + 8, ((m_display.height() + (lcdint_t)height) >> 1) + 4}};
    // TODO: It would be nice to calculate message height
    NanoPoint textPos = {(m_display.width() - (lcdint_t)width) >> 1, (m_display.height() - height) >> 1};
    refresh(rect);
    for ( lcduint_t y = 0; y < m_display.height(); y = y + canvas.height() )
    {
        uint16_t flag = m_refreshFlags[y / canvas.height()];
        m_refreshFlags[y / canvas.height()] = 0;
        for ( lcduint_t x = 0; x < m_display.width(); x = x + canvas.width() )
        {
            if ( flag & 0x01 )
            {
                canvas.setOffset(x + offset.x, y + offset.y);
                if ( !m_onDraw )
                {
                    canvas.clear();
                    draw();
                }
                else if ( m_onDraw() )
                {
                    draw();
                }
                canvas.setOffset(x, y);
                canvas.setColor(RGB_COLOR8(0, 0, 0));
                canvas.fillRect(rect);
                canvas.setColor(RGB_COLOR8(192, 192, 192));
                canvas.drawRect(rect);
                canvas.printFixed(textPos.x, textPos.y, msg);

                m_display.drawCanvas(x, y, canvas);
            }
            flag >>= 1;
        }
    }
}

/**
 * @}
 */

#endif
