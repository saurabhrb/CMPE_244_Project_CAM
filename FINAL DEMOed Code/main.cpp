#include "tasks.hpp"
#include "examples/examples.hpp"
#include "io.hpp"
#include "printf_lib.h"
#include "storage.hpp"
#include "event_groups.h"
#include <stdio.h>
#include <time.h>
#include <string>
#include <sstream>
#include <iomanip>
#include <string>
#include "string.h"
#include "stdio_ext.h"
#include "cstring"
#include "ssp1.h"

#include "uart0.hpp"
#include "uart2.hpp"

//#include "LCD_MENU.hpp"
//#include "CAM.hpp"

//#include "static_images.hpp"

#include "lpc_pwm.hpp"
#include "OV2640.hpp"

TaskHandle_t cam_tsk;

//OV2640 *cam;

//#include "OV7670.hpp"
//#include "ov7670_FINAL.hpp"

//#include "OV2640.hpp"

LCD_MENU *CurrentMenuG;

OV2640 *cam;

void start_capture()
{
    cam->flush_fifo();
    cam->clear_fifo_flag();
    cam->start_capture();
}


void channge_res(RESOLUTION res)
{
    switch (res) {
        case _160x120:
            cam->wrSensorRegs8_8(OV2640_160x120_JPEG);
            break;
        case _176x144:
            cam->wrSensorRegs8_8(OV2640_176x144_JPEG);
            break;
        case _320x240:
            cam->wrSensorRegs8_8(OV2640_320x240_JPEG);
            break;
        case _352x288:
            cam->wrSensorRegs8_8(OV2640_352x288_JPEG);
            break;
        default:
            cam->wrSensorRegs8_8(OV2640_352x288_JPEG);
    }

}

void lcd_tsk(void*p)
{
    LabGPIO_X CS(0, 0), RST(0, 1);
    cam = new OV2640();
    uint8_t vid, pid;
    uint8_t temp;

    cam->write_reg(ARDUCHIP_TEST1, 0x55);
    temp = cam->read_reg(ARDUCHIP_TEST1);
    if (temp != 0x55) {
        printf("SPI1 interface Error!");
        while (1)
            ;
    }

    //Check if the camera module type is OV2640
    cam->wrSensorReg8_8(0xff, 0x01);
    cam->rdSensorReg8_8(OV2640_CHIPID_HIGH, &vid);
    cam->rdSensorReg8_8(OV2640_CHIPID_LOW, &pid);
    if ((vid != 0x26) && ((pid != 0x41) || (pid != 0x42))) printf("Can't find OV2640 module!");
    else
        printf("OV2640 detected.");

    //    cam->set_format(JPEG);
    //    cam->write_reg(ARDUCHIP_FRAMES,2);

    cam->InitCAM();
    //    cam->OV2640_set_JPEG_size(OV2640_320x240);
    cam->clear_fifo_flag();
//    LabGPIO_X MENU_(1,23), UP(1,22), LEFT(1,20), SELECT(1,19), RIGHT(0,30), DOWN(0,29);

//    P1.23 MENU
//    P1.22 UP
//    P1.20 LEFT
//    P1.19 SELECT
//    P0.30 RIGHT
//    P0.29 DOWN

    u0_dbg_printf("STARTED\n");
//    OV2640 *camV3 = new OV2640();

//    camV3->InitCAM();
    u0_dbg_printf("INIT_DONE\n");
//    camV3->setRes(qvga_ov7670);
//    camV3->setColor(regsRGB565);
//    u0_dbg_printf("SET_COLOR_DONE\n");
//    camV3->setColor(OV7670_QVGA);
//    camV3->wrReg(0x11, 20); //Earlier it had the value: wrReg(0x11, 12); New version works better for me :) !!!!
//    camV3->turn(off);
    u0_dbg_printf("CAM TUREND OFF\n");

    LCD_MENU *MAIN = new LCD_MENU(CS, RST, 30, "MAIN", CurrentMenuG, nullptr, menuType);

    //Make the MAIN MENU
    MAIN->addSubMenu("About");
    MAIN->addSubMenu("Camera");
//    MAIN->addSubMenu("Pictures");

    //About
    LCD_MENU *Sub1 = MAIN->confirmedGetSubMenu("About");
    Sub1->addSubMenu("Members");
    Sub1->addSubMenu("Components");

    //Sub1_Sub1
//    LCD_MENU *Sub1_Sub1 = Sub1->confirmedGetSubMenu("Sub1_Sub1");
//    Sub1_Sub1->addSubMenu("Sub1_Sub1_Sub1");
//    Sub1_Sub1->addItem("Sub1_Sub1_It1");

//Sub2
    LCD_MENU *Sub2 = MAIN->confirmedGetSubMenu("Camera");
    Sub2->addSubMenu("=");
    Sub2->addItem("160x120");
    Sub2->addItem("176x144");
    Sub2->addItem("320x240");
    Sub2->addItem("352x288");

//    LCD_MENU *Pics = MAIN->confirmedGetSubMenu("Pictures");
//    Pics->addItem("");

    uint16_t *pixels = new uint16_t[352];

    bool pressed = true;
    CurrentMenuG = MAIN;

    //INIT LCD PART
    if (CurrentMenuG->lcd->begin(RA8875_800x480)) u0_dbg_printf("DONE_INIT\n");
    else {
        u0_dbg_printf("INIT NOT DONE\n");
        while (1)
            ;
    }
    CurrentMenuG->lcd->displayOn(true);
    CurrentMenuG->lcd->GPIOX(true);      // Enable TFT - display enable tied to GPIOX
    CurrentMenuG->lcd->PWM1config(true, RA8875_PWM_CLK_DIV1024); // PWM output for backlight
    CurrentMenuG->lcd->PWM1out(255);

    CurrentMenuG->lcd->graphicsMode();
    CurrentMenuG->lcd->fillScreen(RA8875_BLACK);

    vTaskDelay(500);


     //    CurrentMenuG->clear_disp((uint16_t*) photo);
     //    CurrentMenuG->drawArray((uint16_t *) photo, 100 * 100, 100, 400 - 100, 240 - 100);
     //
     //    for(uint16_t i=0;i<10000;i++){
     //    uint8_t pix8[2] = {photo[i]&0xFF, photo[i]>>8};
     //        Storage::append("1:sjsu.txt",pix8,2);
     //    }
     //    u0_dbg_printf("DONE\n");
     //    for (int i = 0; i < 640; i += 100) {
     //        for (int j = 0; j < 400; j += 100) {
     //    CurrentMenuG->drawArray((uint16_t *) photo, 100 * 100, 100, 640 / 2 - 50, 400 / 2 - 50);
     //        }
     //    }

//    for (int i = 0; i < 100; i++) {
//        uint16_t *pixels2 = new uint16_t[100];
//        Storage::read("1:sjsu.txt", pixels2, 2 * 100, i * 2 * 100);
//        CurrentMenuG->drawArray((uint16_t *) pixels2, 100, 100, 640 / 2 - 50, 400 / 2 - 50 + i);
//    }
    uint16_t width = 352, height = 288;

    while (1) {
        //DOWN
        if (SW.getSwitch(2))
        {
            pressed = true;
            CurrentMenuG->scrollDown();
        }

        //UP
        if (SW.getSwitch(1))
        {
            pressed = true;
            CurrentMenuG->scrollUp();
        }

        //SELECT
        if (SW.getSwitch(3))
        {

            pressed = true;
            if(CurrentMenuG->myName.compareTo("Camera"))
            {
                CurrentMenuG->stream = true;
                channge_res(CurrentMenuG->res);
                switch (CurrentMenuG->res) {
                    case _160x120:
                    width = 160, height = 120;
                    break;
                    case _176x144:
                    width = 176, height = 144;
                    break;
                    case _320x240:
                    width = 320, height = 240;
                    break;
                    case _352x288:
                    width = 352, height = 288;
                    break;
                    default:
                    width = 160, height = 120;
                }

            }
            else
            CurrentMenuG->stream = false;

            CurrentMenuG = CurrentMenuG->loadNext();
        }

        //BACK
        if (SW.getSwitch(4))
        {
            pressed = true;
            CurrentMenuG = CurrentMenuG->loadPrev();
        }
        //BUTTON PRESS
        if (pressed) {
            CurrentMenuG->Display();
            pressed = false;
        }

        if(CurrentMenuG->stream)
        {
//            time = sys_get_uptime_ms();
            CurrentMenuG->lcd->graphicsMode();
            start_capture();
            while (!cam->get_bit(ARDUCHIP_TRIG, CAP_DONE_MASK))
            ;
            size_t len = cam->read_fifo_length();
            if (len >= 0x07ffff) {
                printf("Over size.");
                continue;
            }
            else if (len == 0) {
                printf("Size is 0.");
                continue;
            }
            else
            printf("len = %d\n", len);

            cam->CS(0);
            cam->set_fifo_burst();
            uint16_t x = 0, y = 0;
            for (y = 0; y < height; y++) {
                for (x = 0; x < width; x++) {
                    uint8_t VH = ssp0_exchange_byte(0x00);
                    uint8_t VL = ssp0_exchange_byte(0x00);
                    pixels[x] = (VH) | (VL << 8);
                }
                CurrentMenuG->lcd->drawPixels(pixels, width, 0, y);
            }
            cam->CS(1);

            CurrentMenuG->lcd->textMode();
        }
        else
        {
            channge_res(CurrentMenuG->res);
            vTaskDelay(250);
        }
    }
}


void b_tsk(void*p)
{
    cam = new OV2640();
    LabGPIO_X CS(0, 0), RST(0, 1);
    LCD *lcd = new LCD(CS, RST, 12);
    //INIT LCD PART

    if (lcd->begin(RA8875_800x480)) u0_dbg_printf("DONE_INIT\n");
    else {
        u0_dbg_printf("INIT NOT DONE\n");
        while (1)
            ;
    }
    lcd->displayOn(true);
    lcd->GPIOX(true);      // Enable TFT - display enable tied to GPIOX
    lcd->PWM1config(true, RA8875_PWM_CLK_DIV1024); // PWM output for backlight
    lcd->PWM1out(255);

    lcd->graphicsMode();
    lcd->fillScreen(RA8875_BLACK);
    vTaskDelay(500);

    uint8_t vid, pid;
    uint8_t temp;

    cam->write_reg(ARDUCHIP_TEST1, 0x55);
    temp = cam->read_reg(ARDUCHIP_TEST1);
    if (temp != 0x55) {
        printf("SPI1 interface Error!");
        while (1)
            ;
    }

    //Check if the camera module type is OV2640
    cam->wrSensorReg8_8(0xff, 0x01);
    cam->rdSensorReg8_8(OV2640_CHIPID_HIGH, &vid);
    cam->rdSensorReg8_8(OV2640_CHIPID_LOW, &pid);
    if ((vid != 0x26) && ((pid != 0x41) || (pid != 0x42))) printf("Can't find OV2640 module!");
    else
        printf("OV2640 detected.");

    //    cam->set_format(JPEG);
    //    cam->write_reg(ARDUCHIP_FRAMES,2);

    cam->InitCAM();
//        cam->OV2640_set_JPEG_size(OV2640_320x240);
    cam->clear_fifo_flag();

    uint16_t width = 320, height = 240;
    uint16_t *pixels = new uint16_t[width];
    while (1) {
        start_capture();
        while (!cam->get_bit(ARDUCHIP_TRIG, CAP_DONE_MASK))
            ;
        size_t len = cam->read_fifo_length();
        if (len >= 0x07ffff) {
            printf("Over size.");
            continue;
        }
        else if (len == 0) {
            printf("Size is 0.");
            continue;
        }
        else
            printf("len = %d\n", len);

        cam->CS(0);
        cam->set_fifo_burst();
//        cam->set_fifo_single();
//         #if !(defined (ARDUCAM_SHIELD_V2) && defined (OV2640_CAM))
//         SPI.transfer(0xFF);
//         #endif

        uint16_t x = 0, y = 0;

//        while (len) {
        for (y = 0; y < height; y++) {
            for (x = 0; x < width; x++) {
                uint8_t VH = ssp0_exchange_byte(0x00);
                //                    len--;
                uint8_t VL = ssp0_exchange_byte(0x00);
                //                    len--;
                pixels[x] = (VH) | (VL << 8);
            }
            lcd->drawPixels(pixels, width, 0, y);
        }
//        }
        cam->CS(1);

//        vTaskDelay(1000);
    }

    delete pixels;
}

#define scheduler 0

int main()
{

// PWM PW(PWM::pwm1, 8000000);
//        PW.set(50);

    CurrentMenuG = nullptr;
#if scheduler
    scheduler_add_task(new terminalTask(PRIORITY_MEDIUM));
    xTaskCreate(lcd_tsk, "LCD", 1024 * 4, (void*) 1, PRIORITY_MEDIUM, NULL);
    scheduler_start();
#else
    xTaskCreate(lcd_tsk, "CAM", 1024 * 4, (void*) 1, PRIORITY_HIGH, NULL);
    vTaskStartScheduler();
#endif

    return 0;
}
