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
#include "ssp0.h"
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
//
//        wrSensorRegs8_8(OV2640_JPEG_INIT);
//        wrSensorRegs8_8(OV2640_YUV422);
//        wrSensorRegs8_8(OV2640_JPEG);
        wrSensorRegs8_8(OV2640_352x288_JPEG);
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

    void clear_fifo_flag(void)
    {
        write_reg(ARDUCHIP_FIFO, FIFO_CLEAR_MASK);
    }

    uint8_t read_fifo(void)
    {
        uint8_t data;
        data = bus_read(SINGLE_FIFO_READ);
        return data;
    }

    uint8_t read_reg(uint8_t addr)
    {
        uint8_t data;
        data = bus_read(addr & 0x7F);
        return data;
    }

    uint32_t read_fifo_length(void)
    {
        uint32_t len1, len2, len3, length = 0;
        len1 = read_reg(FIFO_SIZE1);
        len2 = read_reg(FIFO_SIZE2);
        len3 = read_reg(FIFO_SIZE3) & 0x7f;
        length = ((len3 << 16) | (len2 << 8) | len1) & 0x07fffff;
        return length;
    }

    void set_fifo_burst()
    {
        ssp0_exchange_byte(BURST_FIFO_READ);

    }
    void set_fifo_single()
    {
        ssp0_exchange_byte(SINGLE_FIFO_READ);
    }

    void write_reg(uint8_t addr, uint8_t data)
    {
        bus_write(addr | 0x80, data);

    }

    //Set corresponding bit
    void set_bit(uint8_t addr, uint8_t bit)
    {
        uint8_t temp;
        temp = read_reg(addr);
        write_reg(addr, temp | bit);
    }
    //Clear corresponding bit
    void clear_bit(uint8_t addr, uint8_t bit)
    {
        uint8_t temp;
        temp = read_reg(addr);
        write_reg(addr, temp & (~bit));
    }

    //Get corresponding bit status
    uint8_t get_bit(uint8_t addr, uint8_t bit)
    {
        uint8_t temp;
        temp = read_reg(addr);
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
                write_reg(ARDUCHIP_MODE, MCU2LCD_MODE);
                break;
            case CAM2LCD_MODE:
                write_reg(ARDUCHIP_MODE, CAM2LCD_MODE);
                break;
            case LCD2MCU_MODE:
                write_reg(ARDUCHIP_MODE, LCD2MCU_MODE);
                break;
            default:
                write_reg(ARDUCHIP_MODE, MCU2LCD_MODE);
                break;
        }
    }

    uint8_t bus_write(int address, int value)
    {
        CS(0);
        ssp0_exchange_byte(address);
        ssp0_exchange_byte(value);
        CS(1);
        return 1;
    }

    uint8_t bus_read(int address)
    {
        uint8_t value;
        CS(0);

        ssp0_exchange_byte(address);
        value = ssp0_exchange_byte(0x00);

        // correction for bit rotation from readback
//              value = (byte)(value >> 1) | (value << 7);
        CS(1);
        return value;

    }

    void OV2640_set_Light_Mode(uint8_t Light_Mode)
    {
        switch (Light_Mode) {

            case Auto:
                wrSensorReg8_8(0xff, 0x00);
                wrSensorReg8_8(0xc7, 0x00); //AWB on
                break;
            case Sunny:
                wrSensorReg8_8(0xff, 0x00);
                wrSensorReg8_8(0xc7, 0x40); //AWB off
                wrSensorReg8_8(0xcc, 0x5e);
                wrSensorReg8_8(0xcd, 0x41);
                wrSensorReg8_8(0xce, 0x54);
                break;
            case Cloudy:
                wrSensorReg8_8(0xff, 0x00);
                wrSensorReg8_8(0xc7, 0x40); //AWB off
                wrSensorReg8_8(0xcc, 0x65);
                wrSensorReg8_8(0xcd, 0x41);
                wrSensorReg8_8(0xce, 0x4f);
                break;
            case Office:
                wrSensorReg8_8(0xff, 0x00);
                wrSensorReg8_8(0xc7, 0x40); //AWB off
                wrSensorReg8_8(0xcc, 0x52);
                wrSensorReg8_8(0xcd, 0x41);
                wrSensorReg8_8(0xce, 0x66);
                break;
            case Home:
                wrSensorReg8_8(0xff, 0x00);
                wrSensorReg8_8(0xc7, 0x40); //AWB off
                wrSensorReg8_8(0xcc, 0x42);
                wrSensorReg8_8(0xcd, 0x3f);
                wrSensorReg8_8(0xce, 0x71);
                break;
            default:
                wrSensorReg8_8(0xff, 0x00);
                wrSensorReg8_8(0xc7, 0x00); //AWB on
                break;
        }
    }

    void OV2640_set_Color_Saturation(uint8_t Color_Saturation)
    {
        switch (Color_Saturation) {
            case Saturation2:

                wrSensorReg8_8(0xff, 0x00);
                wrSensorReg8_8(0x7c, 0x00);
                wrSensorReg8_8(0x7d, 0x02);
                wrSensorReg8_8(0x7c, 0x03);
                wrSensorReg8_8(0x7d, 0x68);
                wrSensorReg8_8(0x7d, 0x68);
                break;
            case Saturation1:
                wrSensorReg8_8(0xff, 0x00);
                wrSensorReg8_8(0x7c, 0x00);
                wrSensorReg8_8(0x7d, 0x02);
                wrSensorReg8_8(0x7c, 0x03);
                wrSensorReg8_8(0x7d, 0x58);
                wrSensorReg8_8(0x7d, 0x58);
                break;
            case Saturation0:
                wrSensorReg8_8(0xff, 0x00);
                wrSensorReg8_8(0x7c, 0x00);
                wrSensorReg8_8(0x7d, 0x02);
                wrSensorReg8_8(0x7c, 0x03);
                wrSensorReg8_8(0x7d, 0x48);
                wrSensorReg8_8(0x7d, 0x48);
                break;
            case Saturation_1:
                wrSensorReg8_8(0xff, 0x00);
                wrSensorReg8_8(0x7c, 0x00);
                wrSensorReg8_8(0x7d, 0x02);
                wrSensorReg8_8(0x7c, 0x03);
                wrSensorReg8_8(0x7d, 0x38);
                wrSensorReg8_8(0x7d, 0x38);
                break;
            case Saturation_2:
                wrSensorReg8_8(0xff, 0x00);
                wrSensorReg8_8(0x7c, 0x00);
                wrSensorReg8_8(0x7d, 0x02);
                wrSensorReg8_8(0x7c, 0x03);
                wrSensorReg8_8(0x7d, 0x28);
                wrSensorReg8_8(0x7d, 0x28);
                break;
        }
    }

    void OV2640_set_Brightness(uint8_t Brightness)
    {
        switch (Brightness) {
            case Brightness2:
                wrSensorReg8_8(0xff, 0x00);
                wrSensorReg8_8(0x7c, 0x00);
                wrSensorReg8_8(0x7d, 0x04);
                wrSensorReg8_8(0x7c, 0x09);
                wrSensorReg8_8(0x7d, 0x40);
                wrSensorReg8_8(0x7d, 0x00);
                break;
            case Brightness1:
                wrSensorReg8_8(0xff, 0x00);
                wrSensorReg8_8(0x7c, 0x00);
                wrSensorReg8_8(0x7d, 0x04);
                wrSensorReg8_8(0x7c, 0x09);
                wrSensorReg8_8(0x7d, 0x30);
                wrSensorReg8_8(0x7d, 0x00);
                break;
            case Brightness0:
                wrSensorReg8_8(0xff, 0x00);
                wrSensorReg8_8(0x7c, 0x00);
                wrSensorReg8_8(0x7d, 0x04);
                wrSensorReg8_8(0x7c, 0x09);
                wrSensorReg8_8(0x7d, 0x20);
                wrSensorReg8_8(0x7d, 0x00);
                break;
            case Brightness_1:
                wrSensorReg8_8(0xff, 0x00);
                wrSensorReg8_8(0x7c, 0x00);
                wrSensorReg8_8(0x7d, 0x04);
                wrSensorReg8_8(0x7c, 0x09);
                wrSensorReg8_8(0x7d, 0x10);
                wrSensorReg8_8(0x7d, 0x00);
                break;
            case Brightness_2:
                wrSensorReg8_8(0xff, 0x00);
                wrSensorReg8_8(0x7c, 0x00);
                wrSensorReg8_8(0x7d, 0x04);
                wrSensorReg8_8(0x7c, 0x09);
                wrSensorReg8_8(0x7d, 0x00);
                wrSensorReg8_8(0x7d, 0x00);
                break;
        }
    }

    void OV2640_set_Contrast(uint8_t Contrast)
    {
        switch (Contrast) {
            case Contrast2:

                wrSensorReg8_8(0xff, 0x00);
                wrSensorReg8_8(0x7c, 0x00);
                wrSensorReg8_8(0x7d, 0x04);
                wrSensorReg8_8(0x7c, 0x07);
                wrSensorReg8_8(0x7d, 0x20);
                wrSensorReg8_8(0x7d, 0x28);
                wrSensorReg8_8(0x7d, 0x0c);
                wrSensorReg8_8(0x7d, 0x06);
                break;
            case Contrast1:
                wrSensorReg8_8(0xff, 0x00);
                wrSensorReg8_8(0x7c, 0x00);
                wrSensorReg8_8(0x7d, 0x04);
                wrSensorReg8_8(0x7c, 0x07);
                wrSensorReg8_8(0x7d, 0x20);
                wrSensorReg8_8(0x7d, 0x24);
                wrSensorReg8_8(0x7d, 0x16);
                wrSensorReg8_8(0x7d, 0x06);
                break;
            case Contrast0:
                wrSensorReg8_8(0xff, 0x00);
                wrSensorReg8_8(0x7c, 0x00);
                wrSensorReg8_8(0x7d, 0x04);
                wrSensorReg8_8(0x7c, 0x07);
                wrSensorReg8_8(0x7d, 0x20);
                wrSensorReg8_8(0x7d, 0x20);
                wrSensorReg8_8(0x7d, 0x20);
                wrSensorReg8_8(0x7d, 0x06);
                break;
            case Contrast_1:
                wrSensorReg8_8(0xff, 0x00);
                wrSensorReg8_8(0x7c, 0x00);
                wrSensorReg8_8(0x7d, 0x04);
                wrSensorReg8_8(0x7c, 0x07);
                wrSensorReg8_8(0x7d, 0x20);
                wrSensorReg8_8(0x7d, 0x20);
                wrSensorReg8_8(0x7d, 0x2a);
                wrSensorReg8_8(0x7d, 0x06);
                break;
            case Contrast_2:
                wrSensorReg8_8(0xff, 0x00);
                wrSensorReg8_8(0x7c, 0x00);
                wrSensorReg8_8(0x7d, 0x04);
                wrSensorReg8_8(0x7c, 0x07);
                wrSensorReg8_8(0x7d, 0x20);
                wrSensorReg8_8(0x7d, 0x18);
                wrSensorReg8_8(0x7d, 0x34);
                wrSensorReg8_8(0x7d, 0x06);
                break;
        }

    }

    void OV2640_set_Special_effects(uint8_t Special_effect)
    {
        switch (Special_effect) {
            case Antique:

                wrSensorReg8_8(0xff, 0x00);
                wrSensorReg8_8(0x7c, 0x00);
                wrSensorReg8_8(0x7d, 0x18);
                wrSensorReg8_8(0x7c, 0x05);
                wrSensorReg8_8(0x7d, 0x40);
                wrSensorReg8_8(0x7d, 0xa6);
                break;
            case Bluish:
                wrSensorReg8_8(0xff, 0x00);
                wrSensorReg8_8(0x7c, 0x00);
                wrSensorReg8_8(0x7d, 0x18);
                wrSensorReg8_8(0x7c, 0x05);
                wrSensorReg8_8(0x7d, 0xa0);
                wrSensorReg8_8(0x7d, 0x40);
                break;
            case Greenish:
                wrSensorReg8_8(0xff, 0x00);
                wrSensorReg8_8(0x7c, 0x00);
                wrSensorReg8_8(0x7d, 0x18);
                wrSensorReg8_8(0x7c, 0x05);
                wrSensorReg8_8(0x7d, 0x40);
                wrSensorReg8_8(0x7d, 0x40);
                break;
            case Reddish:
                wrSensorReg8_8(0xff, 0x00);
                wrSensorReg8_8(0x7c, 0x00);
                wrSensorReg8_8(0x7d, 0x18);
                wrSensorReg8_8(0x7c, 0x05);
                wrSensorReg8_8(0x7d, 0x40);
                wrSensorReg8_8(0x7d, 0xc0);
                break;
            case BW:
                wrSensorReg8_8(0xff, 0x00);
                wrSensorReg8_8(0x7c, 0x00);
                wrSensorReg8_8(0x7d, 0x18);
                wrSensorReg8_8(0x7c, 0x05);
                wrSensorReg8_8(0x7d, 0x80);
                wrSensorReg8_8(0x7d, 0x80);
                break;
            case Negative:
                wrSensorReg8_8(0xff, 0x00);
                wrSensorReg8_8(0x7c, 0x00);
                wrSensorReg8_8(0x7d, 0x40);
                wrSensorReg8_8(0x7c, 0x05);
                wrSensorReg8_8(0x7d, 0x80);
                wrSensorReg8_8(0x7d, 0x80);
                break;
            case BWnegative:
                wrSensorReg8_8(0xff, 0x00);
                wrSensorReg8_8(0x7c, 0x00);
                wrSensorReg8_8(0x7d, 0x58);
                wrSensorReg8_8(0x7c, 0x05);
                wrSensorReg8_8(0x7d, 0x80);
                wrSensorReg8_8(0x7d, 0x80);

                break;
            case Normal:

                wrSensorReg8_8(0xff, 0x00);
                wrSensorReg8_8(0x7c, 0x00);
                wrSensorReg8_8(0x7d, 0x00);
                wrSensorReg8_8(0x7c, 0x05);
                wrSensorReg8_8(0x7d, 0x80);
                wrSensorReg8_8(0x7d, 0x80);

                break;

        }
    }

    int wrSensorRegs8_8(const struct sensor_reg reglist[])
    {

        int err = 0;
        uint16_t reg_addr = 0;
        uint16_t reg_val = 0;
        const struct sensor_reg *next = reglist;
        while ((reg_addr != 0xff) | (reg_val != 0xff)) {
            reg_addr = next->reg;
            reg_val = next->val;
            err = wrSensorReg8_8(reg_addr, reg_val);
            next++;

        }
        return 1;
    }

    int wrSensorReg8_8(int regID, int regDat)
    {
        _i2c->writeReg(Dev_addr, regID, regDat);
        vTaskDelay(1);
        return 1;

    }

    int rdSensorReg8_8(uint8_t regID, uint8_t* regDat)
    {
        *regDat = _i2c->readReg(Dev_addr, regID);
        vTaskDelay(1);
        return 1;

    }

private:
    I2C2 *_i2c;
    LabGPIO_X _cs;
};

#endif /* OV2640_HPP_ */
