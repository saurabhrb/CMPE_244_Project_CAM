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
    ssp0_init(4);
    ssp0_set_max_clock(4); //4MHz
    _cs.setPinPort(2, 7); //P2.7
    _cs.setAsOutput();
    _cs.set(1);
}

OV2640::~OV2640()
{

}
