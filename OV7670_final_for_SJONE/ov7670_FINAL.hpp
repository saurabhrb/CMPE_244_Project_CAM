/*
 * ov7670_FINAL.hpp
 *
 *  Created on: May 18, 2018
 *      Author: GyR0
 */

#ifndef OV7670_FINAL_HPP_
#define OV7670_FINAL_HPP_

#include "stdint.h"
#include "i2c2.hpp"
#include "stdint.h"
#include "lpc_pwm.hpp"
#include "LabGPIO.hpp"

#include "ov7670_FINAL_I2c_reglist.hpp"

#define Dev_addr 0x42

class OV {
public:

    OV();
    ~OV();

    void wrReg(uint8_t reg, uint8_t dat);

    uint8_t rdReg(uint8_t reg);

    void wrSensorRegs8_8(const struct regval_list reglist[]);

    void setColor(const struct regval_list regli[]);

    void setRes(const struct regval_list regli[]);

    void camInit(void);

    void captureImg(uint16_t wg, uint16_t hg);

    void turn(on_off val);

private:
    I2C2 *_i2c;
    PWM *_pwm;
    int Cx, Cy;
    LabGPIO_X D[8];
    LabGPIO_X RESET, PWDN;
    LabGPIO_X VSync, Href, Pclk;
};

#endif /* OV7670_FINAL_HPP_ */
