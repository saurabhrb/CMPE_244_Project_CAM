/*
 * OV2640.hpp
 *
 *  Created on: May 21, 2018
 *      Author: GyR0
 */

#ifndef OV2640_HPP_
#define OV2640_HPP_

#define Dev_addr 0x60

#include "stdint.h"
#include "i2c2.hpp"
#include "stdint.h"
#include "ov2640_regs.hpp"
#include "storage.hpp"
#include "LCD_MENU.hpp"
#include "ssp1.h"
#include "LabGPIO.hpp"
#include "lpc_sys.h"
#include "stdio.h"

static const size_t bufferSize = 4096;
static uint8_t buffer[bufferSize] = { 0xFF };

class OV2640 {
public:

    OV2640();
    ~OV2640();

    void InitCAM()
    {
        wrSensorReg8_8(0xff, 0x01);
                wrSensorReg8_8(0x12, 0x80);
                wrSensorRegs8_8(OV2640_QVGA);
    }
    void CS(bool val)
    {
        _cs.set(val);
    }

    void flush_fifo(void)
    {
        write_reg(ARDUCHIP_FIFO, FIFO_CLEAR_MASK);
    }

    void start_capture(void)
    {
        write_reg(ARDUCHIP_FIFO, FIFO_START_MASK);
    }

    void clear_fifo_flag(void )
    {
        write_reg(ARDUCHIP_FIFO, FIFO_CLEAR_MASK);
    }

    uint32_t read_fifo_length(void)
    {
        uint32_t len1,len2,len3,length=0;
        len1 = read_reg(FIFO_SIZE1);
      len2 = read_reg(FIFO_SIZE2);
      len3 = read_reg(FIFO_SIZE3) & 0x7f;
      length = ((len3 << 16) | (len2 << 8) | len1) & 0x07fffff;
        return length;
    }

    ////OLD
        void start_capture(void);
        void clear_fifo_flag(void);
        uint8_t read_fifo(void);

    void wrReg(uint8_t reg, uint8_t dat);


    uint8_t rdReg(uint8_t reg);

    void wrSReg(uint8_t reg, uint8_t dat)
    {
        _i2c->writeReg(Dev_addr, reg, dat);
//        bus_write(reg,dat);
        vTaskDelayMs(1);
    }

    uint8_t rdSReg(uint8_t reg)
    {
        uint8_t dat;
        dat = _i2c->readReg(Dev_addr, reg);
//        dat = bus_read(reg);
        vTaskDelayMs(1);
        return dat;
    }

    void wrSensorRegs8_8(const struct regval_list reglist[]);

    bool rdSensorRegs8_8(const struct regval_list reglist[]);

    void camInit(void);

    void captureImg(uint16_t wg, uint16_t hg, LCD_MENU *myLCD);

    void flush_fifo(void)
    {
        wrReg(ARDUCHIP_FIFO, FIFO_CLEAR_MASK);
    }

    void capture(void)
    {
        wrReg(ARDUCHIP_FIFO, FIFO_START_MASK);
    }

    void clear_fifo_flag(void)
    {
        wrReg(ARDUCHIP_FIFO, FIFO_CLEAR_MASK);
    }

    uint8_t read_fifo(void)
    {
        uint8_t data;
        data = bus_read(0x3D);
        return data;
    }

    uint32_t read_fifo_length(void)
    {
        uint32_t len1, len2, len3, length = 0;
        len1 = rdReg(FIFO_SIZE1);
        len2 = rdReg(FIFO_SIZE2);
        len3 = rdReg(FIFO_SIZE3) & 0x7f;
        length = ((len3 << 16) | (len2 << 8) | len1) & 0x07fffff;
        return length;
    }

    void set_single_fifo()
    {

        _cs.set(0);
        ssp1_exchange_byte(SINGLE_FIFO_READ);
        _cs.set(1);
    }
    void set_fifo_burst()

    {
        _cs.set(0);
        ssp1_exchange_byte(BURST_FIFO_READ);
        _cs.set(1);
    }

    void set_bit(uint8_t addr, uint8_t bit)
    {
        uint8_t temp;
        temp = rdReg(addr);
        wrReg(addr, temp | bit);
    }
    //Clear corresponding bit
    void clear_bit(uint8_t addr, uint8_t bit)
    {
        uint8_t temp;
        temp = rdReg(addr);
        wrReg(addr, temp & (~bit));
    }

    //Get corresponding bit status
    uint8_t get_bit(uint8_t addr, uint8_t bit)
    {
        uint8_t temp;
        temp = rdReg(addr);
        temp = temp & bit;
        return temp;
    }

    //Set ArduCAM working mode
    //MCU2LCD_MODE: MCU writes the LCD screen GRAM
    //CAM2LCD_MODE: Camera takes control of the LCD screen
    //LCD2MCU_MODE: MCU read the LCD screen GRAM
    void set_mode(uint8_t mode)
    {
        switch (mode) {
            case MCU2LCD_MODE:
                wrReg(ARDUCHIP_MODE, MCU2LCD_MODE);
                break;
            case CAM2LCD_MODE:
                wrReg(ARDUCHIP_MODE, CAM2LCD_MODE);
                break;
            case LCD2MCU_MODE:
                wrReg(ARDUCHIP_MODE, LCD2MCU_MODE);
                break;
            default:
                wrReg(ARDUCHIP_MODE, MCU2LCD_MODE);
                break;
        }
    }

    uint8_t bus_write(int address, int value)
    {
        _cs.set(0);
        ssp1_exchange_byte(address);
        ssp1_exchange_byte(value);
        _cs.set(1);
        return 1;
    }

    uint8_t bus_read(int address)
    {
        uint8_t value;
        _cs.set(0);
        ssp1_exchange_byte(address);
        value = ssp1_exchange_byte(0x00);
        _cs.set(1);
        return value;
    }

    void start_capture(void)
    {
        wrReg(ARDUCHIP_FIFO, FIFO_START_MASK);
    }

    void OV2640_set_JPEG_size(uint8_t size)
    {
        switch (size) {
            case OV2640_160x120:
                wrSensorRegs8_8(OV2640_160x120_JPEG);
                break;
            case OV2640_176x144:
                wrSensorRegs8_8(OV2640_176x144_JPEG);
                break;
            case OV2640_320x240:
                wrSensorRegs8_8(OV2640_320x240_JPEG);
                break;
            case OV2640_352x288:
                wrSensorRegs8_8(OV2640_352x288_JPEG);
                break;
            case OV2640_640x480:
                wrSensorRegs8_8(OV2640_640x480_JPEG);
                break;
            case OV2640_800x600:
                wrSensorRegs8_8(OV2640_800x600_JPEG);
                break;
            case OV2640_1024x768:
                wrSensorRegs8_8(OV2640_1024x768_JPEG);
                break;
            case OV2640_1280x1024:
                wrSensorRegs8_8(OV2640_1280x1024_JPEG);
                break;
            case OV2640_1600x1200:
                wrSensorRegs8_8(OV2640_1600x1200_JPEG);
                break;
            default:
                wrSensorRegs8_8(OV2640_320x240_JPEG);
                break;
        }
    }

private:
    I2C2 *_i2c;
    LabGPIO_X _cs;
};

#endif /* OV2640_HPP_ */
