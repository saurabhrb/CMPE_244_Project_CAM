///*
// *     SocialLedge.com - Copyright (C) 2013
// *
// *     This file is part of free software framework for embedded processors.
// *     You can use it and/or distribute it as long as this copyright header
// *     remains unmodified.  The code is free for personal use and requires
// *     permission to use in a commercial product.
// *
// *      THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
// *      OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
// *      MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
// *      I SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR
// *      CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
// *
// *     You can reach the author of this software at :
// *          p r e e t . w i k i @ g m a i l . c o m
// */
//
///**
// * @file
// * @brief This is the application entry point.
// *            FreeRTOS and stdio printf is pre-configured to use uart0_min.h before main() enters.
// *            @see L0_LowLevel/lpc_sys.h if you wish to override printf/scanf functions.
// *
// */
//#include "tasks.hpp"
//#include "examples/examples.hpp"
//
///**
// * The main() creates tasks or "threads".  See the documentation of scheduler_task class at scheduler_task.hpp
// * for details.  There is a very simple example towards the beginning of this class's declaration.
// *
// * @warning SPI #1 bus usage notes (interfaced to SD & Flash):
// *      - You can read/write files from multiple tasks because it automatically goes through SPI semaphore.
// *      - If you are going to use the SPI Bus in a FreeRTOS task, you need to use the API at L4_IO/fat/spi_sem.h
// *
// * @warning SPI #0 usage notes (Nordic wireless)
// *      - This bus is more tricky to use because if FreeRTOS is not running, the RIT interrupt may use the bus.
// *      - If FreeRTOS is running, then wireless task may use it.
// *        In either case, you should avoid using this bus or interfacing to external components because
// *        there is no semaphore configured for this bus and it should be used exclusively by nordic wireless.
// */
//int main(void)
//{
//    /**
//     * A few basic tasks for this bare-bone system :
//     *      1.  Terminal task provides gateway to interact with the board through UART terminal.
//     *      2.  Remote task allows you to use remote control to interact with the board.
//     *      3.  Wireless task responsible to receive, retry, and handle mesh network.
//     *
//     * Disable remote task if you are not using it.  Also, it needs SYS_CFG_ENABLE_TLM
//     * such that it can save remote control codes to non-volatile memory.  IR remote
//     * control codes can be learned by typing the "learn" terminal command.
//     */
//    scheduler_add_task(new terminalTask(PRIORITY_HIGH));
//
//    /* Consumes very little CPU, but need highest priority to handle mesh network ACKs */
//    scheduler_add_task(new wirelessTask(PRIORITY_CRITICAL));
//
//    /* Change "#if 0" to "#if 1" to run period tasks; @see period_callbacks.cpp */
//    #if 0
//    const bool run_1Khz = false;
//    scheduler_add_task(new periodicSchedulerTask(run_1Khz));
//    #endif
//
//    /* The task for the IR receiver to "learn" IR codes */
//    // scheduler_add_task(new remoteTask  (PRIORITY_LOW));
//
//    /* Your tasks should probably used PRIORITY_MEDIUM or PRIORITY_LOW because you want the terminal
//     * task to always be responsive so you can poke around in case something goes wrong.
//     */
//
//    /**
//     * This is a the board demonstration task that can be used to test the board.
//     * This also shows you how to send a wireless packets to other boards.
//     */
//    #if 0
//        scheduler_add_task(new example_io_demo());
//    #endif
//
//    /**
//     * Change "#if 0" to "#if 1" to enable examples.
//     * Try these examples one at a time.
//     */
//    #if 0
//        scheduler_add_task(new example_task());
//        scheduler_add_task(new example_alarm());
//        scheduler_add_task(new example_logger_qset());
//        scheduler_add_task(new example_nv_vars());
//    #endif
//
//    /**
//   * Try the rx / tx tasks together to see how they queue data to each other.
//   */
//    #if 0
//        scheduler_add_task(new queue_tx());
//        scheduler_add_task(new queue_rx());
//    #endif
//
//    /**
//     * Another example of shared handles and producer/consumer using a queue.
//     * In this example, producer will produce as fast as the consumer can consume.
//     */
//    #if 0
//        scheduler_add_task(new producer());
//        scheduler_add_task(new consumer());
//    #endif
//
//    /**
//     * If you have RN-XV on your board, you can connect to Wifi using this task.
//     * This does two things for us:
//     *   1.  The task allows us to perform HTTP web requests (@see wifiTask)
//     *   2.  Terminal task can accept commands from TCP/IP through Wifly module.
//     *
//     * To add terminal command channel, add this at terminal.cpp :: taskEntry() function:
//     * @code
//     *     // Assuming Wifly is on Uart3
//     *     addCommandChannel(Uart3::getInstance(), false);
//     * @endcode
//     */
//    #if 0
//        Uart3 &u3 = Uart3::getInstance();
//        u3.init(WIFI_BAUD_RATE, WIFI_RXQ_SIZE, WIFI_TXQ_SIZE);
//        scheduler_add_task(new wifiTask(Uart3::getInstance(), PRIORITY_LOW));
//    #endif
//
//    scheduler_start(); ///< This shouldn't return
//    return -1;
//}

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
#include "LCD.hpp"

#include "static_images.hpp"

#include "Menu.hpp"

void lcd_tsk(void*p)
{
    LabGPIO_X CS(0, 0), RST(0, 1);
    LCD lcd(CS, RST);

    if (lcd.begin(RA8875_800x480)) u0_dbg_printf("DONE_INIT\n");
    else {
        u0_dbg_printf("INIT NOT DONE\n");
        while (1)
            ;
    }
    lcd.displayOn(true);
    lcd.GPIOX(true);      // Enable TFT - display enable tied to GPIOX
    lcd.PWM1config(true, RA8875_PWM_CLK_DIV1024); // PWM output for backlight
    lcd.PWM1out(255);

    lcd.textMode();
    lcd.fillScreen(RA8875_BLACK);

//    while (1) {
//        char string[15] = "Hello, World! ";
//        lcd.textTransparent(RA8875_WHITE);
//        lcd.textWrite(string);
//        lcd.textColor(RA8875_WHITE, RA8875_RED);
//        lcd.textWrite(string);
//        lcd.textTransparent(RA8875_CYAN);
//        lcd.textWrite(string);
//        lcd.textTransparent(RA8875_GREEN);
//        lcd.textWrite(string);
//        lcd.textColor(RA8875_YELLOW, RA8875_CYAN);
//        lcd.textWrite(string);
//        lcd.textColor(RA8875_BLACK, RA8875_MAGENTA);
//        lcd.textWrite(string);
//        vTaskDelay(1000);
//    }

    lcd.graphicsMode();
    lcd.fillScreen(RA8875_BLACK);
    vTaskDelay(500);

    lcd.fillScreen(RA8875_BLUE);
    vTaskDelay(500);

    //draw sjsu

    lcd.fillScreen(RA8875_YELLOW);
    vTaskDelay(500);

    //draw sjsu

    lcd.fillScreen(RA8875_WHITE);
    vTaskDelay(500);

    //draw sjsu

    lcd.fillScreen(RA8875_BLACK);
    vTaskDelay(500);
//
    uint16_t colors[] = { RA8875_BLUE, RA8875_YELLOW, RA8875_WHITE };
    char msg[] = "SJSU\nCAM\nCMPE\n244\nNEMESIS";
    int i = 0;
    int k = 0;

//
//    for (int i = 0; i < SJSU_HELMET.width; i++) {
//        for (int j = 0; j < SJSU_HELMET.height; j++) {
//            uint16_t r = uint16_t(SJSU_HELMET.arr[i + j + 0]);
//            uint16_t g = uint16_t(SJSU_HELMET.arr[i + j + 1]);
//            uint16_t b = uint16_t(SJSU_HELMET.arr[i + j + 2]);
//            uint16_t col = (r << (16 - 5)) || (g << (8 - 6)) || (b << (8 - 5));
//            lcd.drawPixel(i + 10, j + 10, col);
//
//        }
//    }
    lcd.textMode();
    lcd.textEnlarge(3);
    int x = 250, y = 10;
    lcd.textSetCursor(x, y);
    lcd.textColor(RA8875_BLUE, RA8875_BLACK);

    while (1) {
        if (msg[i] == '\n') {
            y = y + 100;
            x = 250;
            lcd.textSetCursor(x, y);
            lcd.textColor(colors[k % 3], RA8875_BLACK);
        }
        else {
            lcd.textMode();
            lcd.textEnlarge(3);
            lcd.textWriteChar((const char) msg[i]);
            vTaskDelay(1000);
        }
        i++;
        k++;
        if (i == sizeof(msg) / sizeof(char)) {
            i = 0;
            k = k % 3;
            x = 250;
            y = 10;
            lcd.graphicsMode();
            lcd.fillRect(250, 10, 800, 480, RA8875_BLACK);
            vTaskDelay(500);

//            lcd.fillScreen(RA8875_BLUE);
            lcd.fillRect(250, 10, 800, 480, RA8875_BLUE);
            vTaskDelay(500);

            //draw sjsu

//            lcd.fillScreen(RA8875_YELLOW);
            lcd.fillRect(250, 10, 800, 480, RA8875_YELLOW);
            vTaskDelay(500);

            //draw sjsu

//            lcd.fillScreen(RA8875_WHITE);
            lcd.fillRect(250, 10, 800, 480, RA8875_WHITE);
            vTaskDelay(500);

            //draw sjsu

//            lcd.fillScreen(RA8875_BLACK);
            lcd.fillRect(250, 10, 800, 480, RA8875_BLACK);
            vTaskDelay(500);

            lcd.textMode();
            lcd.textEnlarge(3);
            lcd.textColor(colors[k], RA8875_BLACK);
            lcd.textSetCursor(x, y);
        }
    }

}

void cam_tsk(void* p)
{
    char c1, c2 = 'X';
    Uart2::getInstance().init(38400);
    while (1) {
        //if (Uart0::getInstance().getChar(&c1)) Uart2::getInstance().putChar(c2);
        u0_dbg_put((char *) 0x56);

        if (Uart2::getInstance().getChar(&c2, portMAX_DELAY)) u0_dbg_printf("%x", c2);

    }
}

class uart_tsk: public scheduler_task {
private:
    char c1, c2, c3;
public:
    uart_tsk(uint8_t priority) :
            scheduler_task("UART", 1024 * 4, priority)
    {
    }
    bool init()
    {
        Uart2::getInstance().init(38400);
        Uart3::getInstance().init(38400);
        return true;
    }
    bool run(void *p)
    {
        //Uart2::getInstance().putChar(Uart0::getInstance().getcharIntrDriven(c1));
        //if (Uart2::getInstance().getChar(&c2)) Uart0::getInstance().putcharIntrDriven(c2);
//        if (Uart3::getInstance().getChar(&c2)) u0_dbg_put((char*)c2);//Uart0::getInstance().putChar(c2);
        return true;
    }

};

void menu_tsk(void*p)
{

    Menu *CurrentMenuG;
    Menu *MAIN = new Menu("MAIN", CurrentMenuG, nullptr, menuType);

    //Make the MAIN MENU
    MAIN->addSubMenu("Sub1");
    MAIN->addSubMenu("Sub2");
    MAIN->addItem("It1");
    MAIN->addItem("It2");

    //Sub1
    Menu *Sub1 = MAIN->confirmedGetSubMenu("Sub1");
    Sub1->addSubMenu("Sub1_Sub1");
    Sub1->addSubMenu("Sub1_Sub2");
    Sub1->addItem("Sub1_It1");
    Sub1->addItem("Sub1_It2");

    //Sub1_Sub1
    Menu *Sub1_Sub1 = Sub1->confirmedGetSubMenu("Sub1_Sub1");
    Sub1_Sub1->addSubMenu("Sub1_Sub1_Sub1");
    Sub1_Sub1->addItem("Sub1_Sub1_It1");

    //Sub2
    Menu *Sub2 = MAIN->confirmedGetSubMenu("Sub2");
    Sub2->addSubMenu("Sub2_Sub1");
    Sub2->addSubMenu("Sub2_Sub2");
    Sub2->addItem("Sub2_It1");
    Sub2->addItem("Sub2_It2");

    bool pressed = true;
    CurrentMenuG = MAIN;
    while (1) {

        //BUTTON PRESS
        if (pressed) {
            CurrentMenuG->Display();
            pressed = false;
        }

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
            CurrentMenuG = CurrentMenuG->loadNext();
        }

        //BACK
        if (SW.getSwitch(4))
        {
            pressed = true;
            CurrentMenuG = CurrentMenuG->loadPrev();
        }
        vTaskDelay(200);
    }

}

int main()
{

//    scheduler_add_task(new terminalTask(PRIORITY_HIGH));
//scheduler_add_task(new uart_tsk(PRIORITY_HIGH));
//    xTaskCreate(lcd_tsk, "LCD", 1024 * 4, (void*) 1, PRIORITY_MEDIUM, NULL);
//    xTaskCreate(cam_tsk, "CAM", 1024 * 4, (void*) 1, PRIORITY_HIGH, NULL);
    xTaskCreate(menu_tsk, "MENU", 1024 * 4, (void*) 1, PRIORITY_HIGH, NULL);
//scheduler_start();
    vTaskStartScheduler();

//    char c;
//    Uart2::getInstance().setBaudRate(38400);
//    while (1) {
//        //if (Uart0::getInstance().getChar(&c)) Uart2::getInstance().putChar(c);
//        //if (Uart2::getInstance().getChar(&c)) Uart0::getInstance().putChar(c);
//        Uart0::getInstance().putChar('A');
//        vTaskDelay(1000);
//    }
    return 0;
}
