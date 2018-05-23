/*
 * OV2640.cpp
 *
 *  Created on: May 21, 2018
 *      Author: GyR0
 */

#include "OV2640.hpp"

OV2640::OV2640()
{
    _i2c = &I2C2::getInstance();
    ssp1_init();
    ssp1_set_max_clock(4);
    _cs.setPinPort(0, 26); //P0.26
    _cs.setAsOutput();
    _cs.set(1);
}

OV2640::~OV2640()
{

}

void OV2640::wrReg(uint8_t reg, uint8_t dat)
{
//    _i2c->writeReg(Dev_addr, reg, dat);
    bus_write(reg,dat);
    vTaskDelayMs(1);
}

uint8_t OV2640::rdReg(uint8_t reg)
{
    uint8_t dat;
//    dat = _i2c->readReg(Dev_addr, reg);
    dat = bus_read(reg);
    vTaskDelayMs(1);
    return dat;
}

bool OV2640::rdSensorRegs8_8(const struct regval_list reglist[])
{

    uint8_t reg_addr = 0, reg_val = 0;
        const struct regval_list *next = reglist;
        while ((reg_addr != 0xff) | (reg_val != 0xff)) {
            reg_addr = next->reg_num;
            if(rdSReg(reg_addr) != reg_val)
                return false;
            next++;
        }

    return true;
}
void OV2640::wrSensorRegs8_8(const struct regval_list reglist[])
{
    uint8_t reg_addr = 0, reg_val = 0;
    const struct regval_list *next = reglist;
    while ((reg_addr != 0xff) | (reg_val != 0xff)) {
        reg_addr = next->reg_num;
        reg_val = next->value;
        wrSReg(reg_addr, reg_val);
        next++;
    }
}

void OV2640::camInit(void)
{

    uint8_t vid, pid;
    uint8_t temp;
    wrReg(ARDUCHIP_TEST1, 0x55);
    temp = rdReg(ARDUCHIP_TEST1);
    if (temp != 0x55) {
        printf("SPI1 interface Error!\n");
        return;
    }

    temp = rdSReg(0xff);
    printf("temp = %d \n",temp);

    wrReg(0xff, 0x01);
    vid = rdSReg(OV2640_CHIPID_HIGH);
    pid = rdSReg(OV2640_CHIPID_LOW);
    if ((vid != 0x26) && ((pid != 0x41) || (pid != 0x42))) {
        printf("Can't find OV2640 module!");
//        return;
    }
    else
        printf("OV2640 detected.\n");

    temp = rdSReg(0xff);
    if(temp!=0x01)
        printf("temp = %d ERROR1\n",temp);

    wrReg(0x12, 0x80);
    vTaskDelayMs(100);
    temp = rdSReg(0x12);
    if(temp!=0x80)
        printf("temp = %d ERROR2\n",temp);
    vTaskDelayMs(100);
    wrSensorRegs8_8(OV2640_QVGA);
    if(!rdSensorRegs8_8(OV2640_QVGA))
        printf("ERROR3\n");
    wrSensorRegs8_8(RGB565_SVGA);
    if(!rdSensorRegs8_8(RGB565_SVGA))
        printf("ERROR4\n");
//    set_mode(CAM2LCD_MODE);

}

void OV2640::captureImg(uint16_t wg, uint16_t hg, LCD_MENU *myLCD)
{
    uint8_t temp = 0, temp_last = 0;
    int i = 0;
    bool is_header = false;

    clear_fifo_flag();
    start_capture();
    while (!get_bit(ARDUCHIP_TRIG, CAP_DONE_MASK))
        ;

    size_t len = read_fifo_length();
    if (len >= MAX_FIFO_SIZE) //8M
    {
        printf("Over size.");
    }

    if (len == 0) //0 kb
            {
        printf("Size is 0.");
    }

    _cs.set(0);
    set_fifo_burst();
    while (len--) {
        temp_last = temp;
        temp = ssp1_exchange_byte(0x00);
        if ((temp == 0xD9) && (temp_last == 0xFF)) //If find the end ,break while,
                {
            buffer[i++] = temp;  //save the last  0XD9
            //Write the remain bytes in the buffer
            _cs.set(1);
            uint16_t y = (i / (2 * wg));
            uint16_t x = i % (2 * wg);
            myLCD->lcd->drawPixels(buffer, i, x, y);
            is_header = false;
            i = 0;
        }
        if (is_header == true) {
            //Write image data to buffer if not full
            if (i < bufferSize) buffer[i++] = temp;
            else {
                //Write bufferSize bytes image data to file
                _cs.set(1);
                uint16_t y = (i / (2 * wg));
                uint16_t x = i % (2 * wg);
                myLCD->lcd->drawPixels(buffer, bufferSize, x, y);
                i = 0;
                buffer[i++] = temp;
                _cs.set(0);
                set_fifo_burst();
            }

        }
        else if ((temp == 0xD8) & (temp_last == 0xFF)) {
            is_header = true;
            buffer[i++] = temp_last;
            buffer[i++] = temp;
        }

    }

}

