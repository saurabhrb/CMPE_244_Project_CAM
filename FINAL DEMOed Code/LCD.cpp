/**************************************************************************/
/*!
 @file     Adafruit_RA8875.cpp
 @author   Limor Friend/Ladyada, K.Townsend/KTOWN for Adafruit Industries
 @license  BSD license, all text above and below must be included in
 any redistribution

 This is the library for the Adafruit RA8875 Driver board for TFT displays
 ---------------> http://www.adafruit.com/products/1590
 The RA8875 is a TFT driver for up to 800x480 dotclock'd displays
 It is tested to work with displays in the Adafruit shop. Other displays
 may need timing adjustments and are not guanteed to work.

 Adafruit invests time and resources providing this open
 source code, please support Adafruit and open-source hardware
 by purchasing products from Adafruit!

 Written by Limor Fried/Ladyada for Adafruit Industries.
 BSD license, check license.txt for more information.
 All text above must be included in any redistribution.

 @section  HISTORY

 v1.0 - First release
 */
/**************************************************************************/
#include "ssp1.h"
#include "LCD.hpp"

void LCD::set_ssp_freq(unsigned int max_clock_mhz)
{
    max_SPI_clock_mhz = max_clock_mhz;
    ssp1_set_max_clock(12);
}

/**************************************************************************/
/*!
 Constructor for a new RA8875 instance

 @args CS[in]  Location of the SPI chip select pin
 @args RST[in] Location of the reset pin
 */
/**************************************************************************/
LCD::LCD(LabGPIO_X CS, LabGPIO_X RST, unsigned int max_clock_mhz)
{
    _cs = CS;
    _rst = RST;
    max_SPI_clock_mhz = max_clock_mhz;
}

/**************************************************************************/
/*!
 Initialises the LCD driver and any HW required by the display

 @args s[in] The display size, which can be either:
 'RA8875_480x272' (4.3" displays) r
 'RA8875_800x480' (5" and 7" displays)
 */
/**************************************************************************/
bool LCD::begin(enum RA8875sizes s)
{
    _size = s;

    if (_size == RA8875_480x272) {
        _width = 480;
        _height = 272;
    }
    else if (_size == RA8875_800x480) {
        _width = 800;
        _height = 480;
    }
    else {
        return false;
    }

    _cs.setAsOutput();
    _cs.set(1);

    _rst.setAsOutput();

    _rst.set(0);
    vTaskDelayMs(100);
    vTaskDelayMs(100);
    _rst.set(1);
    vTaskDelayMs(100);

    //2MHZ connect to get version
    ssp1_init();
    ssp1_set_max_clock(2);

    uint8_t x = readReg(0);
//    Serial.print("x = 0x"); Serial.println(x,HEX);
    if (x != 0x75) {
        u0_dbg_printf("WRONG VERSION = %x\n", x);
        return false;
    }

    u0_dbg_printf("CORRECT VERSION = %x\n", x);

    initialize();

    //12Mhz connect to do LCD stuff
    ssp1_set_max_clock(max_SPI_clock_mhz);

    return true;
}

/************************* Initialization *********************************/

/**************************************************************************/
/*!
 Performs a SW-based reset of the RA8875
 */
/**************************************************************************/
void LCD::softReset(void)
{
    writeCommand(RA8875_PWRR);
    writeData(RA8875_PWRR_SOFTRESET);
    writeData(RA8875_PWRR_NORMAL);
    vTaskDelayMs(1);
}

/**************************************************************************/
/*!
 Initialise the PLL
 */
/**************************************************************************/
void LCD::PLLinit(void)
{
//    RA8875 Osc Freq = max_SPI_clock_mhz * (RA8875_PLLC1[4:0] + 1) / ((RA8875_PLLC1[7:7] + 1) * 2^(RA8875_PLLC2[2:0]))
    if (_size == RA8875_480x272) {
        writeReg(RA8875_PLLC1, RA8875_PLLC1_PLLDIV1 + 10);
        vTaskDelayMs(1);
        writeReg(RA8875_PLLC2, RA8875_PLLC2_DIV4);
        vTaskDelayMs(1);
    }
    else /* (_size == RA8875_800x480) */{
//        writeReg(RA8875_PLLC1, RA8875_PLLC1_PLLDIV1 + 10); //def
        writeReg(RA8875_PLLC1, RA8875_PLLC1_PLLDIV1 + 8);
        vTaskDelayMs(1);
        writeReg(RA8875_PLLC2, RA8875_PLLC2_DIV4); //def
//        writeReg(RA8875_PLLC2, RA8875_PLLC2_DIV4);
        vTaskDelayMs(1);

        //120MHz
    }
}

/**************************************************************************/
/*!
 Initialises the driver IC (clock setup, etc.)
 */
/**************************************************************************/
void LCD::initialize(void)
{
    PLLinit();
    writeReg(RA8875_SYSR, RA8875_SYSR_16BPP | RA8875_SYSR_MCU8);

    /* Timing values */
    uint8_t pixclk;
    uint8_t hsync_start;
    uint8_t hsync_pw;
    uint8_t hsync_finetune;
    uint8_t hsync_nondisp;
    uint8_t vsync_pw;
    uint16_t vsync_nondisp;
    uint16_t vsync_start;

    /* Set the correct values for the display being used */
    if (_size == RA8875_480x272) {
        pixclk = RA8875_PCSR_PDATL | RA8875_PCSR_4CLK;
        hsync_nondisp = 10;
        hsync_start = 8;
        hsync_pw = 48;
        hsync_finetune = 0;
        vsync_nondisp = 3;
        vsync_start = 8;
        vsync_pw = 10;
    }
    else // (_size == RA8875_800x480)
    {
        pixclk = RA8875_PCSR_PDATL | RA8875_PCSR_2CLK;
        hsync_nondisp = 26;
        hsync_start = 32;
        hsync_pw = 96;
        hsync_finetune = 0;
        vsync_nondisp = 32;
        vsync_start = 23;
        vsync_pw = 2;
    }

    writeReg(RA8875_PCSR, pixclk);
    vTaskDelayMs(1);

    /* Horizontal settings registers */
    writeReg(RA8875_HDWR, (_width / 8) - 1);                          // H width: (HDWR + 1) * 8 = 480
    writeReg(RA8875_HNDFTR, RA8875_HNDFTR_DE_HIGH + hsync_finetune);
    writeReg(RA8875_HNDR, (hsync_nondisp - hsync_finetune - 2) / 8);    // H non-display: HNDR * 8 + HNDFTR + 2 = 10
    writeReg(RA8875_HSTR, hsync_start / 8 - 1);                         // Hsync start: (HSTR + 1)*8
    writeReg(RA8875_HPWR, RA8875_HPWR_LOW + (hsync_pw / 8 - 1));        // HSync pulse width = (HPWR+1) * 8

    /* Vertical settings registers */
    writeReg(RA8875_VDHR0, (uint16_t) (_height - 1) & 0xFF);
    writeReg(RA8875_VDHR1, (uint16_t) (_height - 1) >> 8);
    writeReg(RA8875_VNDR0, vsync_nondisp - 1);                          // V non-display period = VNDR + 1
    writeReg(RA8875_VNDR1, vsync_nondisp >> 8);
    writeReg(RA8875_VSTR0, vsync_start - 1);                            // Vsync start position = VSTR + 1
    writeReg(RA8875_VSTR1, vsync_start >> 8);
    writeReg(RA8875_VPWR, RA8875_VPWR_LOW + vsync_pw - 1);            // Vsync pulse width = VPWR + 1

    /* Set active window X */
    writeReg(RA8875_HSAW0, 0);                                        // horizontal start point
    writeReg(RA8875_HSAW1, 0);
    writeReg(RA8875_HEAW0, (uint16_t) (_width - 1) & 0xFF);            // horizontal end point
    writeReg(RA8875_HEAW1, (uint16_t) (_width - 1) >> 8);

    /* Set active window Y */
    writeReg(RA8875_VSAW0, 0);                                        // vertical start point
    writeReg(RA8875_VSAW1, 0);
    writeReg(RA8875_VEAW0, (uint16_t) (_height - 1) & 0xFF);           // horizontal end point
    writeReg(RA8875_VEAW1, (uint16_t) (_height - 1) >> 8);

    /* Setup touch panel? */

    /* Clear the entire window */
    writeReg(RA8875_MCLR, RA8875_MCLR_START | RA8875_MCLR_FULL);
    vTaskDelayMs(500);
}

/**************************************************************************/
/*!
 Returns the display width in pixels

 @returns  The 1-based display width in pixels
 */
/**************************************************************************/
uint16_t LCD::width(void)
{
    return _width;
}

/**************************************************************************/
/*!
 Returns the display height in pixels

 @returns  The 1-based display height in pixels
 */
/**************************************************************************/
uint16_t LCD::height(void)
{
    return _height;
}

/************************* Text Mode ***********************************/

/**************************************************************************/
/*!
 Sets the display in text mode (as opposed to graphics mode)
 */
/**************************************************************************/
void LCD::textMode(void)
{
    /* Set text mode */
    writeCommand(RA8875_MWCR0);
    uint8_t temp = readData();
    temp |= RA8875_MWCR0_TXTMODE; // Set bit 7
    writeData(temp);

    /* Select the internal (ROM) font */
    writeCommand(0x21);
    temp = readData();
    temp &= ~((1 << 7) | (1 << 5)); // Clear bits 7 and 5
    writeData(temp);
}

/**************************************************************************/
/*!
 Sets the display in text mode (as opposed to graphics mode)

 @args x[in] The x position of the cursor (in pixels, 0..1023)
 @args y[in] The y position of the cursor (in pixels, 0..511)
 */
/**************************************************************************/
void LCD::textSetCursor(uint16_t x, uint16_t y)
{
    /* Set cursor location */
    curr = {x,y};
    writeCommand(0x2A);
    writeData(x & 0xFF);
    writeCommand(0x2B);
    writeData(x >> 8);
    writeCommand(0x2C);
    writeData(y & 0xFF);
    writeCommand(0x2D);
    writeData(y >> 8);
}

uint16_t LCD::getCurrX()
{
    return (uint16_t) curr.x;

}
uint16_t LCD::getCurrY()
{
    return (uint16_t) curr.y;

}
/**************************************************************************/
/*!
 Sets the fore and background color when rendering text

 @args foreColor[in] The RGB565 color to use when rendering the text
 @args bgColor[in]   The RGB565 colot to use for the background
 */
/**************************************************************************/
void LCD::textColor(uint16_t foreColor, uint16_t bgColor)
{
    /* Set Fore Color */
    writeCommand(0x63);
    writeData((foreColor & 0xf800) >> 11);
    writeCommand(0x64);
    writeData((foreColor & 0x07e0) >> 5);
    writeCommand(0x65);
    writeData((foreColor & 0x001f));

    /* Set Background Color */
    writeCommand(0x60);
    writeData((bgColor & 0xf800) >> 11);
    writeCommand(0x61);
    writeData((bgColor & 0x07e0) >> 5);
    writeCommand(0x62);
    writeData((bgColor & 0x001f));

    /* Clear transparency flag */
    writeCommand(0x22);
    uint8_t temp = readData();
    temp &= ~(1 << 6); // Clear bit 6
    writeData(temp);
}

/**************************************************************************/
/*!
 Sets the fore color when rendering text with a transparent bg

 @args foreColor[in] The RGB565 color to use when rendering the text
 */
/**************************************************************************/
void LCD::textTransparent(uint16_t foreColor)
{
    /* Set Fore Color */
    writeCommand(0x63);
    writeData((foreColor & 0xf800) >> 11);
    writeCommand(0x64);
    writeData((foreColor & 0x07e0) >> 5);
    writeCommand(0x65);
    writeData((foreColor & 0x001f));

    /* Set transparency flag */
    writeCommand(0x22);
    uint8_t temp = readData();
    temp |= (1 << 6); // Set bit 6
    writeData(temp);
}

/**************************************************************************/
/*!
 Sets the text enlarge settings, using one of the following values:

 0 = 1x zoom
 1 = 2x zoom
 2 = 3x zoom
 3 = 4x zoom

 @args scale[in]   The zoom factor (0..3 for 1-4x zoom)
 */
/**************************************************************************/
void LCD::textEnlarge(uint8_t scale)
{
    if (scale > 3) scale = 3;

    /* Set font size flags */
    writeCommand(0x22);
    uint8_t temp = readData();
    temp &= ~(0xF); // Clears bits 0..3
    temp |= scale << 2;
    temp |= scale;
    writeData(temp);

    _textScale = scale;
}

/**************************************************************************/
/*!
 Renders some text on the screen when in text mode

 @args buffer[in]    The buffer containing the characters to render
 @args len[in]       The size of the buffer in bytes
 */
/**************************************************************************/
void LCD::textWrite(const char* buffer, uint16_t len)
{
    if (len == 0) len = strlen(buffer);
    writeCommand(RA8875_MRWC);
    for (uint16_t i = 0; i < len; i++) {
        {
            if (buffer[i] == '\n') {
                textSetCursor(getCurrX(), getCurrY() + ((1+1)*(16)) + 10);
                writeCommand(RA8875_MRWC);
            }
            else
                writeData(buffer[i]);
        }
#if defined(__AVR__)
        if (_textScale > 1) vTaskDelayMs(1);
#elif defined(__arm__)
        // This delay is needed with textEnlarge(1) because
        // Teensy 3.X is much faster than Arduino Uno
        if (_textScale > 0) vTaskDelayMs(1);
#endif
    }
}
void LCD::textWriteOFF(const char* buffer, uint16_t len, uint16_t Xoffset, uint16_t Yoffset)
{
    uint8_t len_max = 1;
    if (len == 0) len = strlen(buffer);

    if (len > (200 / (10 * _textScale))) {
        len_max = 200 / (10 * _textScale);
    }
    for (uint16_t j = 0; j < len / len_max; j++) {
        writeCommand(RA8875_MRWC);
        for (uint16_t i = j * len_max; i < len; i++) {
            writeData(buffer[i]);
#if defined(__AVR__)
            if (_textScale > 1) vTaskDelayMs(1);
#elif defined(__arm__)
            // This delay is needed with textEnlarge(1) because
            // Teensy 3.X is much faster than Arduino Uno
            if (_textScale > 0) vTaskDelayMs(1);
#endif
        }
        textSetCursor(getCurrX(), getCurrY() + Yoffset);
    }
}

void LCD::textWriteChar(const char buffer)
{
    writeCommand(RA8875_MRWC);
    writeData(buffer);
// This delay is needed with textEnlarge(1) because
// Teensy 3.X is much faster than Arduino Uno
    if (_textScale > 0) vTaskDelayMs(1);

}

/************************* Graphics ***********************************/

/**************************************************************************/
/*!
 Sets the display in graphics mode (as opposed to text mode)
 */
/**************************************************************************/
void LCD::graphicsMode(void)
{
    writeCommand(RA8875_MWCR0);
    uint8_t temp = readData();
    temp &= ~RA8875_MWCR0_TXTMODE; // bit #7
    writeData(temp);
}

/**************************************************************************/
/*!
 Waits for screen to finish by polling the status!
 */
/**************************************************************************/
bool LCD::waitPoll(uint8_t regname, uint8_t waitflag)
{
    /* Wait for the command to finish */
    while (1) {
        uint8_t temp = readReg(regname);
        if (!(temp & waitflag)) return true;
    }
    return false; // MEMEFIX: yeah i know, unreached! - add timeout?
}

/**************************************************************************/
/*!
 Sets the current X/Y position on the display before drawing

 @args x[in] The 0-based x location
 @args y[in] The 0-base y location
 */
/**************************************************************************/
void LCD::setXY(uint16_t x, uint16_t y)
{
    curr = {x,y};
    writeReg(RA8875_CURH0, x);
    writeReg(RA8875_CURH1, x >> 8);
    writeReg(RA8875_CURV0, y);
    writeReg(RA8875_CURV1, y >> 8);
}

/**************************************************************************/
/*!
 HW accelerated function to push a chunk of raw pixel data

 @args num[in] The number of pixels to push
 @args p[in]   The pixel color to use
 */
/**************************************************************************/
void LCD::pushPixels(uint32_t num, uint16_t p)
{
    _cs.set(0);
    ssp1_exchange_byte(RA8875_DATAWRITE);
    while (num--) {
        ssp1_exchange_byte(p >> 8);
        ssp1_exchange_byte(p);
    }

    _cs.set(1);
}

/**************************************************************************/
/*!

 */
/**************************************************************************/
void LCD::fillRect(void)
{
    writeCommand(RA8875_DCR);
    writeData(RA8875_DCR_LINESQUTRI_STOP | RA8875_DCR_DRAWSQUARE);
    writeData(RA8875_DCR_LINESQUTRI_START | RA8875_DCR_FILL | RA8875_DCR_DRAWSQUARE);
}

/**************************************************************************/
/*!
 Draws a single pixel at the specified location

 @args x[in]     The 0-based x location
 @args y[in]     The 0-base y location
 @args color[in] The RGB565 color to use when drawing the pixel
 */
/**************************************************************************/
void LCD::drawPixel(int16_t x, int16_t y, uint16_t color)
{
    writeReg(RA8875_CURH0, x);
    writeReg(RA8875_CURH1, x >> 8);
    writeReg(RA8875_CURV0, y);
    writeReg(RA8875_CURV1, y >> 8);
    writeCommand(RA8875_MRWC);
    _cs.set(0);
    ssp1_exchange_byte(RA8875_DATAWRITE);
    ssp1_exchange_byte(color >> 8);
    ssp1_exchange_byte(color);
    _cs.set(1);
}

/**************************************************************************/
/*!
 Draws a HW accelerated line on the display

 @args x0[in]    The 0-based starting x location
 @args y0[in]    The 0-base starting y location
 @args x1[in]    The 0-based ending x location
 @args y1[in]    The 0-base ending y location
 @args color[in] The RGB565 color to use when drawing the pixel
 */
/**************************************************************************/
void LCD::drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color)
{
    /* Set X */
    writeCommand(0x91);
    writeData(x0);
    writeCommand(0x92);
    writeData(x0 >> 8);

    /* Set Y */
    writeCommand(0x93);
    writeData(y0);
    writeCommand(0x94);
    writeData(y0 >> 8);

    /* Set X1 */
    writeCommand(0x95);
    writeData(x1);
    writeCommand(0x96);
    writeData((x1) >> 8);

    /* Set Y1 */
    writeCommand(0x97);
    writeData(y1);
    writeCommand(0x98);
    writeData((y1) >> 8);

    /* Set Color */
    writeCommand(0x63);
    writeData((color & 0xf800) >> 11);
    writeCommand(0x64);
    writeData((color & 0x07e0) >> 5);
    writeCommand(0x65);
    writeData((color & 0x001f));

    /* Draw! */
    writeCommand(RA8875_DCR);
    writeData(0x80);

    /* Wait for the command to finish */
    waitPoll(RA8875_DCR, RA8875_DCR_LINESQUTRI_STATUS);
}

/**************************************************************************/
/*!

 */
/**************************************************************************/
void LCD::drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color)
{
    drawLine(x, y, x, y + h, color);
}

/**************************************************************************/
/*!

 */
/**************************************************************************/
void LCD::drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color)
{
    drawLine(x, y, x + w, y, color);
}

/**************************************************************************/
/*!
 Draws a HW accelerated rectangle on the display

 @args x[in]     The 0-based x location of the top-right corner
 @args y[in]     The 0-based y location of the top-right corner
 @args w[in]     The rectangle width
 @args h[in]     The rectangle height
 @args color[in] The RGB565 color to use when drawing the pixel
 */
/**************************************************************************/
void LCD::drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{
    rectHelper(x, y, x + w, y + h, color, false);
}

/**************************************************************************/
/*!
 Draws a HW accelerated filled rectangle on the display

 @args x[in]     The 0-based x location of the top-right corner
 @args y[in]     The 0-based y location of the top-right corner
 @args w[in]     The rectangle width
 @args h[in]     The rectangle height
 @args color[in] The RGB565 color to use when drawing the pixel
 */
/**************************************************************************/
void LCD::fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{
    rectHelper(x, y, x + w, y + h, color, true);
}

/**************************************************************************/
/*!
 Fills the screen with the spefied RGB565 color

 @args color[in] The RGB565 color to use when drawing the pixel
 */
/**************************************************************************/
void LCD::fillScreen(uint16_t color)
{
    rectHelper(0, 0, _width - 1, _height - 1, color, true);
}

/**************************************************************************/
/*!
 Draws a HW accelerated circle on the display

 @args x[in]     The 0-based x location of the center of the circle
 @args y[in]     The 0-based y location of the center of the circle
 @args w[in]     The circle's radius
 @args color[in] The RGB565 color to use when drawing the pixel
 */
/**************************************************************************/
void LCD::drawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color)
{
    circleHelper(x0, y0, r, color, false);
}

/**************************************************************************/
/*!
 Draws a HW accelerated filled circle on the display

 @args x[in]     The 0-based x location of the center of the circle
 @args y[in]     The 0-based y location of the center of the circle
 @args w[in]     The circle's radius
 @args color[in] The RGB565 color to use when drawing the pixel
 */
/**************************************************************************/
void LCD::fillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color)
{
    circleHelper(x0, y0, r, color, true);
}

/**************************************************************************/
/*!
 Draws a HW accelerated triangle on the display

 @args x0[in]    The 0-based x location of point 0 on the triangle
 @args y0[in]    The 0-based y location of point 0 on the triangle
 @args x1[in]    The 0-based x location of point 1 on the triangle
 @args y1[in]    The 0-based y location of point 1 on the triangle
 @args x2[in]    The 0-based x location of point 2 on the triangle
 @args y2[in]    The 0-based y location of point 2 on the triangle
 @args color[in] The RGB565 color to use when drawing the pixel
 */
/**************************************************************************/
void LCD::drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color)
{
    triangleHelper(x0, y0, x1, y1, x2, y2, color, false);
}

/**************************************************************************/
/*!
 Draws a HW accelerated filled triangle on the display

 @args x0[in]    The 0-based x location of point 0 on the triangle
 @args y0[in]    The 0-based y location of point 0 on the triangle
 @args x1[in]    The 0-based x location of point 1 on the triangle
 @args y1[in]    The 0-based y location of point 1 on the triangle
 @args x2[in]    The 0-based x location of point 2 on the triangle
 @args y2[in]    The 0-based y location of point 2 on the triangle
 @args color[in] The RGB565 color to use when drawing the pixel
 */
/**************************************************************************/
void LCD::fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color)
{
    triangleHelper(x0, y0, x1, y1, x2, y2, color, true);
}

/**************************************************************************/
/*!
 Draws a HW accelerated ellipse on the display

 @args xCenter[in]   The 0-based x location of the ellipse's center
 @args yCenter[in]   The 0-based y location of the ellipse's center
 @args longAxis[in]  The size in pixels of the ellipse's long axis
 @args shortAxis[in] The size in pixels of the ellipse's short axis
 @args color[in]     The RGB565 color to use when drawing the pixel
 */
/**************************************************************************/
void LCD::drawEllipse(int16_t xCenter, int16_t yCenter, int16_t longAxis, int16_t shortAxis, uint16_t color)
{
    ellipseHelper(xCenter, yCenter, longAxis, shortAxis, color, false);
}

/**************************************************************************/
/*!
 Draws a HW accelerated filled ellipse on the display

 @args xCenter[in]   The 0-based x location of the ellipse's center
 @args yCenter[in]   The 0-based y location of the ellipse's center
 @args longAxis[in]  The size in pixels of the ellipse's long axis
 @args shortAxis[in] The size in pixels of the ellipse's short axis
 @args color[in]     The RGB565 color to use when drawing the pixel
 */
/**************************************************************************/
void LCD::fillEllipse(int16_t xCenter, int16_t yCenter, int16_t longAxis, int16_t shortAxis, uint16_t color)
{
    ellipseHelper(xCenter, yCenter, longAxis, shortAxis, color, true);
}

/**************************************************************************/
/*!
 Draws a HW accelerated curve on the display

 @args xCenter[in]   The 0-based x location of the ellipse's center
 @args yCenter[in]   The 0-based y location of the ellipse's center
 @args longAxis[in]  The size in pixels of the ellipse's long axis
 @args shortAxis[in] The size in pixels of the ellipse's short axis
 @args curvePart[in] The corner to draw, where in clock-wise motion:
 0 = 180-270�
 1 = 270-0�
 2 = 0-90�
 3 = 90-180�
 @args color[in]     The RGB565 color to use when drawing the pixel
 */
/**************************************************************************/
void LCD::drawCurve(int16_t xCenter, int16_t yCenter, int16_t longAxis, int16_t shortAxis, uint8_t curvePart, uint16_t color)
{
    curveHelper(xCenter, yCenter, longAxis, shortAxis, curvePart, color, false);
}

/**************************************************************************/
/*!
 Draws a HW accelerated filled curve on the display

 @args xCenter[in]   The 0-based x location of the ellipse's center
 @args yCenter[in]   The 0-based y location of the ellipse's center
 @args longAxis[in]  The size in pixels of the ellipse's long axis
 @args shortAxis[in] The size in pixels of the ellipse's short axis
 @args curvePart[in] The corner to draw, where in clock-wise motion:
 0 = 180-270�
 1 = 270-0�
 2 = 0-90�
 3 = 90-180�
 @args color[in]     The RGB565 color to use when drawing the pixel
 */
/**************************************************************************/
void LCD::fillCurve(int16_t xCenter, int16_t yCenter, int16_t longAxis, int16_t shortAxis, uint8_t curvePart, uint16_t color)
{
    curveHelper(xCenter, yCenter, longAxis, shortAxis, curvePart, color, true);
}

/**************************************************************************/
/*!
 Helper function for higher level circle drawing code
 */
/**************************************************************************/
void LCD::circleHelper(int16_t x0, int16_t y0, int16_t r, uint16_t color, bool filled)
{
    /* Set X */
    writeCommand(0x99);
    writeData(x0);
    writeCommand(0x9a);
    writeData(x0 >> 8);

    /* Set Y */
    writeCommand(0x9b);
    writeData(y0);
    writeCommand(0x9c);
    writeData(y0 >> 8);

    /* Set Radius */
    writeCommand(0x9d);
    writeData(r);

    /* Set Color */
    writeCommand(0x63);
    writeData((color & 0xf800) >> 11);
    writeCommand(0x64);
    writeData((color & 0x07e0) >> 5);
    writeCommand(0x65);
    writeData((color & 0x001f));

    /* Draw! */
    writeCommand(RA8875_DCR);
    if (filled) {
        writeData(RA8875_DCR_CIRCLE_START | RA8875_DCR_FILL);
    }
    else {
        writeData(RA8875_DCR_CIRCLE_START | RA8875_DCR_NOFILL);
    }

    /* Wait for the command to finish */
    waitPoll(RA8875_DCR, RA8875_DCR_CIRCLE_STATUS);
}

/**************************************************************************/
/*!
 Helper function for higher level rectangle drawing code
 */
/**************************************************************************/
void LCD::rectHelper(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color, bool filled)
{
    /* Set X */
    writeCommand(0x91);
    writeData(x);
    writeCommand(0x92);
    writeData(x >> 8);

    /* Set Y */
    writeCommand(0x93);
    writeData(y);
    writeCommand(0x94);
    writeData(y >> 8);

    /* Set X1 */
    writeCommand(0x95);
    writeData(w);
    writeCommand(0x96);
    writeData((w) >> 8);

    /* Set Y1 */
    writeCommand(0x97);
    writeData(h);
    writeCommand(0x98);
    writeData((h) >> 8);

    /* Set Color */
    writeCommand(0x63);
    writeData((color & 0xf800) >> 11);
    writeCommand(0x64);
    writeData((color & 0x07e0) >> 5);
    writeCommand(0x65);
    writeData((color & 0x001f));

    /* Draw! */
    writeCommand(RA8875_DCR);
    if (filled) {
        writeData(0xB0);
    }
    else {
        writeData(0x90);
    }

    /* Wait for the command to finish */
    waitPoll(RA8875_DCR, RA8875_DCR_LINESQUTRI_STATUS);
}

/**************************************************************************/
/*!
 Helper function for higher level triangle drawing code
 */
/**************************************************************************/
void LCD::triangleHelper(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color, bool filled)
{
    /* Set Point 0 */
    writeCommand(0x91);
    writeData(x0);
    writeCommand(0x92);
    writeData(x0 >> 8);
    writeCommand(0x93);
    writeData(y0);
    writeCommand(0x94);
    writeData(y0 >> 8);

    /* Set Point 1 */
    writeCommand(0x95);
    writeData(x1);
    writeCommand(0x96);
    writeData(x1 >> 8);
    writeCommand(0x97);
    writeData(y1);
    writeCommand(0x98);
    writeData(y1 >> 8);

    /* Set Point 2 */
    writeCommand(0xA9);
    writeData(x2);
    writeCommand(0xAA);
    writeData(x2 >> 8);
    writeCommand(0xAB);
    writeData(y2);
    writeCommand(0xAC);
    writeData(y2 >> 8);

    /* Set Color */
    writeCommand(0x63);
    writeData((color & 0xf800) >> 11);
    writeCommand(0x64);
    writeData((color & 0x07e0) >> 5);
    writeCommand(0x65);
    writeData((color & 0x001f));

    /* Draw! */
    writeCommand(RA8875_DCR);
    if (filled) {
        writeData(0xA1);
    }
    else {
        writeData(0x81);
    }

    /* Wait for the command to finish */
    waitPoll(RA8875_DCR, RA8875_DCR_LINESQUTRI_STATUS);
}

/**************************************************************************/
/*!
 Helper function for higher level ellipse drawing code
 */
/**************************************************************************/
void LCD::ellipseHelper(int16_t xCenter, int16_t yCenter, int16_t longAxis, int16_t shortAxis, uint16_t color, bool filled)
{
    /* Set Center Point */
    writeCommand(0xA5);
    writeData(xCenter);
    writeCommand(0xA6);
    writeData(xCenter >> 8);
    writeCommand(0xA7);
    writeData(yCenter);
    writeCommand(0xA8);
    writeData(yCenter >> 8);

    /* Set Long and Short Axis */
    writeCommand(0xA1);
    writeData(longAxis);
    writeCommand(0xA2);
    writeData(longAxis >> 8);
    writeCommand(0xA3);
    writeData(shortAxis);
    writeCommand(0xA4);
    writeData(shortAxis >> 8);

    /* Set Color */
    writeCommand(0x63);
    writeData((color & 0xf800) >> 11);
    writeCommand(0x64);
    writeData((color & 0x07e0) >> 5);
    writeCommand(0x65);
    writeData((color & 0x001f));

    /* Draw! */
    writeCommand(0xA0);
    if (filled) {
        writeData(0xC0);
    }
    else {
        writeData(0x80);
    }

    /* Wait for the command to finish */
    waitPoll(RA8875_ELLIPSE, RA8875_ELLIPSE_STATUS);
}

/**************************************************************************/
/*!
 Helper function for higher level curve drawing code
 */
/**************************************************************************/
void LCD::curveHelper(int16_t xCenter, int16_t yCenter, int16_t longAxis, int16_t shortAxis, uint8_t curvePart, uint16_t color, bool filled)
{
    /* Set Center Point */
    writeCommand(0xA5);
    writeData(xCenter);
    writeCommand(0xA6);
    writeData(xCenter >> 8);
    writeCommand(0xA7);
    writeData(yCenter);
    writeCommand(0xA8);
    writeData(yCenter >> 8);

    /* Set Long and Short Axis */
    writeCommand(0xA1);
    writeData(longAxis);
    writeCommand(0xA2);
    writeData(longAxis >> 8);
    writeCommand(0xA3);
    writeData(shortAxis);
    writeCommand(0xA4);
    writeData(shortAxis >> 8);

    /* Set Color */
    writeCommand(0x63);
    writeData((color & 0xf800) >> 11);
    writeCommand(0x64);
    writeData((color & 0x07e0) >> 5);
    writeCommand(0x65);
    writeData((color & 0x001f));

    /* Draw! */
    writeCommand(0xA0);
    if (filled) {
        writeData(0xD0 | (curvePart & 0x03));
    }
    else {
        writeData(0x90 | (curvePart & 0x03));
    }

    /* Wait for the command to finish */
    waitPoll(RA8875_ELLIPSE, RA8875_ELLIPSE_STATUS);
}

/************************* Mid Level ***********************************/

/**************************************************************************/
/*!

 */
/**************************************************************************/
void LCD::GPIOX(bool on)
{
    if (on) writeReg(RA8875_GPIOX, 1);
    else
        writeReg(RA8875_GPIOX, 0);
}

/**************************************************************************/
/*!

 */
/**************************************************************************/
void LCD::PWM1out(uint8_t p)
{
    writeReg(RA8875_P1DCR, p);
}

/**************************************************************************/
/*!

 */
/**************************************************************************/
void LCD::PWM2out(uint8_t p)
{
    writeReg(RA8875_P2DCR, p);
}

/**************************************************************************/
/*!

 */
/**************************************************************************/
void LCD::PWM1config(bool on, uint8_t clock)
{
    if (on) {
        writeReg(RA8875_P1CR, RA8875_P1CR_ENABLE | (clock & 0xF));
    }
    else {
        writeReg(RA8875_P1CR, RA8875_P1CR_DISABLE | (clock & 0xF));
    }
}

/**************************************************************************/
/*!

 */
/**************************************************************************/
void LCD::PWM2config(bool on, uint8_t clock)
{
    if (on) {
        writeReg(RA8875_P2CR, RA8875_P2CR_ENABLE | (clock & 0xF));
    }
    else {
        writeReg(RA8875_P2CR, RA8875_P2CR_DISABLE | (clock & 0xF));
    }
}

/**************************************************************************/
/*!
 Enables or disables the on-chip touch screen controller
 */
/**************************************************************************/
void LCD::touchEnable(bool on)
{
    uint8_t adcClk = (uint8_t) RA8875_TPCR0_ADCCLK_DIV4;

    if (_size == RA8875_800x480) //match up touch size with LCD size
    adcClk = (uint8_t) RA8875_TPCR0_ADCCLK_DIV16;

    if (on) {
        /* Enable Touch Panel (Reg 0x70) */
        writeReg(RA8875_TPCR0, RA8875_TPCR0_ENABLE |
        RA8875_TPCR0_WAIT_4096CLK |
        RA8875_TPCR0_WAKEENABLE | adcClk); // 10mhz max!
        /* Set Auto Mode      (Reg 0x71) */
        writeReg(RA8875_TPCR1, RA8875_TPCR1_AUTO |
        // RA8875_TPCR1_VREFEXT |
                RA8875_TPCR1_DEBOUNCE);
        /* Enable TP INT */
        writeReg(RA8875_INTC1, readReg(RA8875_INTC1) | RA8875_INTC1_TP);
    }
    else {
        /* Disable TP INT */
        writeReg(RA8875_INTC1, readReg(RA8875_INTC1) & ~RA8875_INTC1_TP);
        /* Disable Touch Panel (Reg 0x70) */
        writeReg(RA8875_TPCR0, RA8875_TPCR0_DISABLE);
    }
}

/**************************************************************************/
/*!
 Checks if a touch event has occured

 @returns  True is a touch event has occured (reading it via
 touchRead() will clear the interrupt in memory)
 */
/**************************************************************************/
bool LCD::touched(void)
{
    if (readReg(RA8875_INTC2) & RA8875_INTC2_TP) return true;
    return false;
}

/**************************************************************************/
/*!
 Reads the last touch event

 @args x[out]  Pointer to the uint16_t field to assign the raw X value
 @args y[out]  Pointer to the uint16_t field to assign the raw Y value

 @note Calling this function will clear the touch panel interrupt on
 the RA8875, resetting the flag used by the 'touched' function
 */
/**************************************************************************/
bool LCD::touchRead(uint16_t *x, uint16_t *y)
{
    uint16_t tx, ty;
    uint8_t temp;

    tx = readReg(RA8875_TPXH);
    ty = readReg(RA8875_TPYH);
    temp = readReg(RA8875_TPXYL);
    tx <<= 2;
    ty <<= 2;
    tx |= temp & 0x03;        // get the bottom x bits
    ty |= (temp >> 2) & 0x03; // get the bottom y bits

    *x = tx;
    *y = ty;

    /* Clear TP INT Status */
    writeReg(RA8875_INTC2, RA8875_INTC2_TP);

    return true;
}

/**************************************************************************/
/*!
 Turns the display on or off
 */
/**************************************************************************/
void LCD::displayOn(bool on)
{
    if (on) writeReg(RA8875_PWRR, RA8875_PWRR_NORMAL | RA8875_PWRR_DISPON);
    else
        writeReg(RA8875_PWRR, RA8875_PWRR_NORMAL | RA8875_PWRR_DISPOFF);
}

/**************************************************************************/
/*!
 Puts the display in sleep mode, or disables sleep mode if enabled
 */
/**************************************************************************/
void LCD::sleep(bool sleep)
{
    if (sleep) writeReg(RA8875_PWRR, RA8875_PWRR_DISPOFF | RA8875_PWRR_SLEEP);
    else
        writeReg(RA8875_PWRR, RA8875_PWRR_DISPOFF);
}

/************************* Low Level ***********************************/

/**************************************************************************/
/*!

 */
/**************************************************************************/
void LCD::writeReg(uint8_t reg, uint8_t val)
{
    writeCommand(reg);
    writeData(val);
}

/**************************************************************************/
/*!

 */
/**************************************************************************/
uint8_t LCD::readReg(uint8_t reg)
{
    writeCommand(reg);
    return readData();
}

/**************************************************************************/
/*!

 */
/**************************************************************************/
void LCD::writeData(uint8_t d)
{
    _cs.set(0);

    ssp1_exchange_byte(RA8875_DATAWRITE);
    ssp1_exchange_byte(d);

    _cs.set(1);
}

/**************************************************************************/
/*!

 */
/**************************************************************************/
uint8_t LCD::readData(void)
{
    _cs.set(0);

    ssp1_exchange_byte(RA8875_DATAREAD);
    uint8_t x = ssp1_exchange_byte(0x0);

    _cs.set(1);
    return x;
}

/**************************************************************************/
/*!

 */
/**************************************************************************/
void LCD::writeCommand(uint8_t d)
{
    _cs.set(0);

    ssp1_exchange_byte(RA8875_CMDWRITE);
    ssp1_exchange_byte(d);

    _cs.set(1);
}

/**************************************************************************/
/*!

 */
/**************************************************************************/
uint8_t LCD::readStatus(void)
{
    _cs.set(0);

    ssp1_exchange_byte(RA8875_CMDREAD);
    uint8_t x = ssp1_exchange_byte(0x0);

    _cs.set(1);
    return x;
}

void LCD::drawPixels(uint16_t p[], uint16_t count, int16_t x, int16_t y)
{
    writeReg(RA8875_CURH0, x);
    writeReg(RA8875_CURH1, x >> 8);
    writeReg(RA8875_CURV0, y);
    writeReg(RA8875_CURV1, y >> 8);
    uint16_t temp = 0;
    uint16_t i;
    writeCommand(RA8875_MRWC);

    _cs.set(0);
    //set data
    ssp1_exchange_byte(RA8875_DATAWRITE);

    //the loop
    for (i = 0; i < count; i++) {
        temp = p[i];
        ssp1_exchange_byte(temp >> 8);
        ssp1_exchange_byte(temp & 0xFF);
    }
    _cs.set(1);
}

void LCD::drawPixels(uint8_t p[], uint16_t count, int16_t x, int16_t y)
{
    writeReg(RA8875_CURH0, x);
    writeReg(RA8875_CURH1, x >> 8);
    writeReg(RA8875_CURV0, y);
    writeReg(RA8875_CURV1, y >> 8);
    uint16_t temp = 0;
    uint16_t i;
    writeCommand(RA8875_MRWC);

    _cs.set(0);
    //set data
    ssp1_exchange_byte(RA8875_DATAWRITE);

    //the loop
    for (i = 0; i < count; i += 2) {
        temp = (p[i] << 8) | p[i + 1];
        ssp1_exchange_byte(temp >> 8);
        ssp1_exchange_byte(temp & 0xFF);
    }
    _cs.set(1);
}

/**************************************************************************/
/*!
 Get a pixel color from screen
 Parameters:
 x: horizontal pos
 y: vertical pos
 */
/**************************************************************************/
uint16_t LCD::getPixel(int16_t x, int16_t y)
{
    uint16_t color;
    setXY(x, y);
    graphicsMode();
    writeCommand(RA8875_MRWC);
    _cs.set(0);

    ssp1_exchange_byte(RA8875_DATAREAD);
    ssp1_exchange_byte(0x00);

    color = ssp1_exchange_byte(0x00);
    color |= ssp1_exchange_byte(0x00) << 8;

    _cs.set(1);
    return color;
}

void LCD::getPixels(uint16_t * p, uint32_t count, int16_t x, int16_t y)
{
    uint16_t color;

    graphicsMode();
    setXY(x, y);
    writeCommand(RA8875_MRWC);

    _cs.set(0);
    ssp1_exchange_byte(RA8875_DATAREAD);

    ssp1_exchange_byte(0x00);
    ssp1_exchange_byte(0x00);

    while (count--) {
        color = ssp1_exchange_byte(0x00);
        color |= ssp1_exchange_byte(0x00) << 8;
        *p++ = color;
    }
    _cs.set(1);
}

