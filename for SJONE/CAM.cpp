/*
 * CAM.cpp
 *
 *  Created on: May 1, 2018
 *      Author: GyR0
 */

#include "CAM.hpp"
#include "string.h"

// Initialization code used by all constructor types
void CAM::common_init(void)
{
    frameptr = 0;
    bufferLen = 0;
    serialNum = 0;
    for (int i = 0; i < CAMERABUFFSIZ + 1; i++)
        camerabuff[i] = 0;
    U = &Uart3::getInstance();
}

// Constructor when using HardwareSerial
CAM::CAM()
{
    common_init();  // Set everything to common state, then...
    fail_begin = false;
}

bool CAM::begin(uint32_t baud)
{
    fail_begin = false;
    U->init(baud, 10, 10);
    vTaskDelayMs(100);
    bool f = reset();
//    if (!f)
//    {
//        U->init(115200, 10, 10);
//        vTaskDelayMs(100);
//        f = reset();
//    }
//    else
//    {
//        u0_dbg_printf("\nBaud => %s\n",setBaud115200());
//    }

    fail_begin = true;
    return f;
}

bool CAM::reset()
{
    uint8_t args[] = { 0x0 };
    return runCommand(VC0706_RESET, args, 1, 5);
}

bool CAM::motionDetected()
{
    if (readResponse(4, 200) != 4) {
        return false;
    }
    if (!verifyResponse(VC0706_COMM_MOTION_DETECTED)) return false;

    return true;
}

bool CAM::setMotionStatus(uint8_t x, uint8_t d1, uint8_t d2)
{
    uint8_t args[] = { 0x03, x, d1, d2 };

    return runCommand(VC0706_MOTION_CTRL, args, sizeof(args), 5);
}

uint8_t CAM::getMotionStatus(uint8_t x)
{
    uint8_t args[] = { 0x01, x };

    return runCommand(VC0706_MOTION_STATUS, args, sizeof(args), 5);
}

bool CAM::setMotionDetect(bool flag)
{
    if (!setMotionStatus(VC0706_MOTIONCONTROL,
    VC0706_UARTMOTION, VC0706_ACTIVATEMOTION)) return false;

    uint8_t args[] = { 0x01, flag };

    runCommand(VC0706_COMM_MOTION_CTRL, args, sizeof(args), 5);
}

bool CAM::getMotionDetect(void)
{
    uint8_t args[] = { 0x0 };

    if (!runCommand(VC0706_COMM_MOTION_STATUS, args, 1, 6)) return false;

    return camerabuff[5];
}

uint8_t CAM::getImageSize()
{
    uint8_t args[] = { 0x4, 0x4, 0x1, 0x00, 0x19 };
    if (!runCommand(VC0706_READ_DATA, args, sizeof(args), 6)) return -1;

    return camerabuff[5];
}

bool CAM::setImageSize(uint8_t x)
{
    uint8_t args[] = { 0x05, 0x04, 0x01, 0x00, 0x19, x };

    return runCommand(VC0706_WRITE_DATA, args, sizeof(args), 5);
}

/****************** downsize image control */

uint8_t CAM::getDownsize(void)
{
    uint8_t args[] = { 0x0 };
    if (!runCommand(VC0706_DOWNSIZE_STATUS, args, 1, 6)) return -1;

    return camerabuff[5];
}

bool CAM::setDownsize(uint8_t newsize)
{
    uint8_t args[] = { 0x01, newsize };

    return runCommand(VC0706_DOWNSIZE_CTRL, args, 2, 5);
}

/***************** other high level commands */

char * CAM::getVersion(void)
{
    uint8_t args[] = { 0x01 };

    sendCommand(VC0706_GEN_VERSION, args, 1);
    // get reply
    if (!readResponse(11, 200)) return 0;
    camerabuff[bufferLen] = 0;  // end it!
    return (char *) camerabuff;  // return it!
}

/***************** baud rate commands */

char* CAM::setBaud9600()
{
    uint8_t args[] = { 0x03, 0x01, 0xAE, 0xC8 };

    sendCommand(VC0706_SET_PORT, args, sizeof(args));
    // get reply
    if (!readResponse(CAMERABUFFSIZ, 200)) return 0;
    camerabuff[bufferLen] = 0;  // end it!
    return (char *) camerabuff;  // return it!

}

char* CAM::setBaud19200()
{
    uint8_t args[] = { 0x03, 0x01, 0x56, 0xE4 };

    sendCommand(VC0706_SET_PORT, args, sizeof(args));
    // get reply
    if (!readResponse(CAMERABUFFSIZ, 200)) return 0;
    camerabuff[bufferLen] = 0;  // end it!
    return (char *) camerabuff;  // return it!
}

char* CAM::setBaud38400()
{
    uint8_t args[] = { 0x03, 0x01, 0x2A, 0xF2 };
    sendCommand(VC0706_SET_PORT, args, sizeof(args));
    // get reply
    if (!readResponse(5, 200)) return 0;
    U->setBaudRate(38400);
    vTaskDelay(500);
    camerabuff[bufferLen] = 0;  // end it!
    return (char *) camerabuff;  // return it!
}

char* CAM::setBaud57600()
{
    uint8_t args[] = { 0x03, 0x01, 0x1C, 0x1C };

    sendCommand(VC0706_SET_PORT, args, sizeof(args));
    // get reply
    if (!readResponse(CAMERABUFFSIZ, 200)) return 0;
    camerabuff[bufferLen] = 0;  // end it!
    return (char *) camerabuff;  // return it!
}

char* CAM::setBaud115200()
{

    uint8_t args[] = { 0x03, 0x01, 0x0D, 0xA6 };

    sendCommand(VC0706_SET_PORT, args, sizeof(args));

    // get reply
    if (!readResponse(5, 200)) return 0;
    U->setBaudRate(115200);
    vTaskDelay(500);
    camerabuff[bufferLen] = 0;  // end it!
    return (char *) camerabuff;  // return it!
}

/****************** high level photo comamnds */

void CAM::OSD(uint8_t x, uint8_t y, char *str)
{
    if (strlen(str) > 14) {
        str[13] = 0;
    }

    uint8_t args[17] = { strlen(str), strlen(str) - 1, (y & 0xF) | ((x & 0x3) << 4) };

    for (uint8_t i = 0; i < strlen(str); i++) {
        char c = str[i];
        if ((c >= '0') && (c <= '9')) {
            str[i] -= '0';
        }
        else if ((c >= 'A') && (c <= 'Z')) {
            str[i] -= 'A';
            str[i] += 10;
        }
        else if ((c >= 'a') && (c <= 'z')) {
            str[i] -= 'a';
            str[i] += 36;
        }

        args[3 + i] = str[i];
    }

    runCommand(VC0706_OSD_ADD_CHAR, args, strlen(str) + 3, 5);
    printBuff();
}

bool CAM::setCompression(uint8_t c)
{
    uint8_t args[] = { 0x5, 0x1, 0x1, 0x12, 0x04, c };
    return runCommand(VC0706_WRITE_DATA, args, sizeof(args), 5);
}

uint8_t CAM::getCompression(void)
{
    uint8_t args[] = { 0x4, 0x1, 0x1, 0x12, 0x04 };
    runCommand(VC0706_READ_DATA, args, sizeof(args), 6);
    printBuff();
    return camerabuff[5];
}

bool CAM::setPTZ(uint16_t wz, uint16_t hz, uint16_t pan, uint16_t tilt)
{
    uint8_t args[] = { 0x08, wz >> 8, wz, hz >> 8, wz, pan >> 8, pan, tilt >> 8, tilt };

    return (!runCommand(VC0706_SET_ZOOM, args, sizeof(args), 5));
}

bool CAM::getPTZ(uint16_t &w, uint16_t &h, uint16_t &wz, uint16_t &hz, uint16_t &pan, uint16_t &tilt)
{
    uint8_t args[] = { 0x0 };

    if (!runCommand(VC0706_GET_ZOOM, args, sizeof(args), 16)) return false;
    printBuff();

    w = camerabuff[5];
    w <<= 8;
    w |= camerabuff[6];

    h = camerabuff[7];
    h <<= 8;
    h |= camerabuff[8];

    wz = camerabuff[9];
    wz <<= 8;
    wz |= camerabuff[10];

    hz = camerabuff[11];
    hz <<= 8;
    hz |= camerabuff[12];

    pan = camerabuff[13];
    pan <<= 8;
    pan |= camerabuff[14];

    tilt = camerabuff[15];
    tilt <<= 8;
    tilt |= camerabuff[16];

    return true;
}

bool CAM::takePicture()
{
    frameptr = 0;
    return cameraFrameBuffCtrl(VC0706_STOPCURRENTFRAME);
}

bool CAM::resumeVideo()
{
    return cameraFrameBuffCtrl(VC0706_RESUMEFRAME);
}

bool CAM::TVon()
{
    uint8_t args[] = { 0x1, 0x1 };
    return runCommand(VC0706_TVOUT_CTRL, args, sizeof(args), 5);
}
bool CAM::TVoff()
{
    uint8_t args[] = { 0x1, 0x0 };
    return runCommand(VC0706_TVOUT_CTRL, args, sizeof(args), 5);
}

bool CAM::cameraFrameBuffCtrl(uint8_t command)
{
    uint8_t args[] = { 0x1, command };
    return runCommand(VC0706_FBUF_CTRL, args, sizeof(args), 5);
}

uint32_t CAM::frameLength(void)
{
    uint8_t args[] = { 0x01, 0x00 };
    if (!runCommand(VC0706_GET_FBUF_LEN, args, sizeof(args), 9)) return 0;

    uint32_t len;
    len = camerabuff[5];
    len <<= 8;
    len |= camerabuff[6];
    len <<= 8;
    len |= camerabuff[7];
    len <<= 8;
    len |= camerabuff[8];

    return len;
}

uint8_t CAM::available(void)
{
    return bufferLen;
}

uint8_t * CAM::readPicture(uint8_t n)
{
    uint8_t args[] = { 0x0C, 0x0, 0x0A, 0, 0, frameptr >> 8, frameptr & 0xFF, 0, 0, 0, n, 0x00, 0x050 };

    if (!runCommand(VC0706_READ_FBUF, args, sizeof(args), 5, false)) return 0;
//    u0_dbg_printf("\nh\n");
    // read into the buffer PACKETLEN!
    if (readResponse(n + 5, 0x050) == 0) return 0;
//    u0_dbg_printf("\ni\n");
    frameptr += n;

    return camerabuff;
}

void CAM::flush_buff()
{
    U->resetQs();
//    for(int i=0;i<CAMERABUFFSIZ+1;i++)
//        camerabuff[i] = 0;

    bufferLen = 0;
}

/**************** low level commands */

bool CAM::runCommand(uint8_t cmd, uint8_t *args, uint8_t argn, uint8_t resplen, bool flushflag)
{
    // flush out anything in the buffer?
    if (flushflag) {
        flush_buff();
    }

    sendCommand(cmd, args, argn);
    if (readResponse(resplen, 200) != resplen) return false;
    //u0_dbg_printf("RespLen correct\n");

    bufferLen = 0;
    //u0_dbg_printf("Resp => ");
//    for (int i = 0; i < resplen; i++)
    //u0_dbg_printf("0x%x ", camerabuff[i]);

    //u0_dbg_printf("\n");
    if (!verifyResponse(cmd)) return false;
    //u0_dbg_printf("RespVerified correct\n");

    return true;
}

void CAM::sendCommand(uint8_t cmd, uint8_t args[] = 0, uint8_t argn = 0)
{
//    u0_dbg_printf("CMD => 0x56 0x%x 0x%x ", serialNum, cmd);
//    for (uint8_t i = 0; i < argn; i++) {
//        u0_dbg_printf("0x%x ", args[i]);
//    }
//    u0_dbg_printf("\n");
//    //u0_dbg_printf("\n[%d]\n",U->getRxQueueWatermark());
//    U->flush();
//    vTaskDelay(1000);
//    //u0_dbg_printf("[%d]\n",U->getRxQueueWatermark());
    U->putChar(0x56);
    U->putChar(serialNum);
    U->putChar(cmd);

    for (uint8_t i = 0; i < argn; i++) {
        U->putChar(args[i]);
    }
}

uint8_t CAM::readResponse(uint8_t numbytes, uint8_t timeout)
{
    uint8_t counter = 0;
    this->bufferLen = 0;
    char c;
    while (this->bufferLen != numbytes) {
        if (this->U->getChar(&c, timeout) == 1) this->camerabuff[this->bufferLen++] = c;
        //else if (!fail_begin) return 0;
    }
//    this->U->flush();
    return this->bufferLen;
}

bool CAM::verifyResponse(uint8_t command)
{
//    //u0_dbg_printf("verify_resp = %s\n",camerabuff);
    if ((camerabuff[0] != 0x76) || (camerabuff[1] != serialNum) || (camerabuff[2] != command) || (camerabuff[3] != 0x0)) return false;
    return true;

}

void CAM::printBuff()
{
    //u0_dbg_printf("CamBuff => ");
    for (uint8_t i = 0; i < bufferLen; i++) {
        //u0_dbg_printf("0x%x, ", camerabuff[i]);
    }
    //u0_dbg_printf("\nBufferLen = %d\n\n", bufferLen);

}

