/*
 * ov7670_FINAL.cpp
 *
 *  Created on: May 18, 2018
 *      Author: GyR0
 */

#include "ov7670_FINAL.hpp"
#include "lpc_sys.h"
#include "stdio.h"

void OV::turn(on_off val)
{
    if (val == on) PWDN.set(0);
    else
        PWDN.set(1);
}

OV::OV()
{
    Cx = Cy = 0;
    _i2c = &I2C2::getInstance();

    //8MHz PWM clock, P2.0
    _pwm = new PWM(PWM::pwm1, 8000000);
    _pwm->set(50);

    //set GPIOs
    D[0].setPinPort(0, 29);
    D[1].setPinPort(0, 30);
    D[2].setPinPort(1, 19);
    D[3].setPinPort(1, 20);
    D[4].setPinPort(1, 22);
    D[5].setPinPort(1, 23);
    D[6].setPinPort(1, 28);
    D[7].setPinPort(1, 29);

    RESET.setPinPort(2, 1);
    PWDN.setPinPort(2, 2);

    VSync.setPinPort(2, 3);
    Href.setPinPort(2, 4);
    Pclk.setPinPort(2, 5);

    //input GPIOs
    for (int i = 0; i < 8; i++)
        D[i].setAsInput();

    VSync.setAsInput();
    Href.setAsInput();
    Pclk.setAsInput();

    //output GPIOs
    RESET.setAsOutput();
    PWDN.setAsOutput();

    RESET.set(1);
    PWDN.set(0);

}

OV::~OV()
{

}

void OV::wrReg(uint8_t reg, uint8_t dat)
{
    _i2c->writeReg(Dev_addr, reg, dat);
    vTaskDelayMs(1);
}

uint8_t OV::rdReg(uint8_t reg)
{
    uint8_t dat;
    dat = _i2c->readReg(Dev_addr, reg);
    vTaskDelayMs(1);
    return dat;
}

void OV::wrSensorRegs8_8(const struct regval_list reglist[])
{
    uint8_t reg_addr = 0, reg_val = 0;
    const struct regval_list *next = reglist;
    while ((reg_addr != 0xff) | (reg_val != 0xff)) {
        reg_addr = next->reg_num;
        reg_val = next->value;
        wrReg(reg_addr, reg_val);
        next++;
    }
}

void OV::setColor(const struct regval_list regli[])
{
    wrSensorRegs8_8(regli);
}

void OV::setRes(const struct regval_list regli[])
{
    wrReg(REG_COM3, 4); // REG_COM3 enable scaling
    wrSensorRegs8_8(regli);
}

void OV::camInit(void)
{
    wrReg(0x12, 0x80);
    vTaskDelayMs(100);
    wrSensorRegs8_8(ov7670_default_regs);
    wrReg(REG_COM10, 32); //PCLK does not toggle on HBLANK.
}

void OV::captureImg(uint16_t wg, uint16_t hg)
{
    uint16_t y, x;

    uint64_t t1 = sys_get_uptime_ms();

    //wait for high
    while (!VSync.getLevel()) {
        t1 = sys_get_uptime_ms();
    }

    while (VSync.getLevel())
        ; //wait for low

    y = hg;
    while (y--) {
        x = wg;
        while (x--) {

            while (Pclk.getLevel())
                ; //wait for low

            //read first byte
            uint8_t pix1 = (D[0].getLevel() << 0) | (D[1].getLevel() << 1) | (D[2].getLevel() << 2) | (D[3].getLevel() << 3) | (D[4].getLevel() << 4)
                    | (D[5].getLevel() << 5) | (D[6].getLevel() << 6) | (D[7].getLevel() << 7);
                printf("%x ",pix1);
            while (!Pclk.getLevel())
                ; //wait for high
            while (Pclk.getLevel())
                ; //wait for low

            //read second byte
            uint8_t pix2 = (D[0].getLevel() << 0) | (D[1].getLevel() << 1) | (D[2].getLevel() << 2) | (D[3].getLevel() << 3) | (D[4].getLevel() << 4)
                    | (D[5].getLevel() << 5) | (D[6].getLevel() << 6) | (D[7].getLevel() << 7);
                printf("%x ",pix2);
            while (!Pclk.getLevel())
                ; //wait for high
        }
        printf("%d: ", y);
            printf("\n\n");
    }
    printf("\nTime taken %d ms\n", (uint8_t) (sys_get_uptime_ms() - t1));
    vTaskDelayMs(100);
}

