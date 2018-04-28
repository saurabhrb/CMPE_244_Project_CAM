#include "LabGPIOInterrupts.h"

/**
 * This should configure NVIC to notice EINT3 IRQs; use NVIC_EnableIRQ()
 */

LabGPIOInterrupts::LabGPIOInterrupts()
{

}

void LabGPIOInterrupts::init()
{
    isr_register(EINT3_IRQn, handle_interrupt);
    NVIC_EnableIRQ(EINT3_IRQn);
}

/**
 * This handler should place a function pointer within the lookup table for the handle_interrupt() to find.
 *
 * @param[in] port         specify the GPIO port
 * @param[in] pin          specify the GPIO pin to assign an ISR to
 * @param[in] pin_isr      function to run when the interrupt event occurs
 * @param[in] condition    condition for the interrupt to occur on. RISING, FALLING or BOTH edges.
 * @return should return true if valid ports, pins, isrs were supplied and pin isr insertion was sucessful
 */
bool LabGPIOInterrupts::attachInterruptHandler(uint8_t port, uint32_t pin, void (*pin_isr)(void), InterruptCondition_E condition)
{

    if (!((port == 0 || port == 2) && (pin >= 0 && pin <= 31) && (pin_isr != nullptr) && (condition < both_edges))) {
        return false;
    }

    if (port) {
        //port 2
        LabGPIOInterrupts::getInstance().isrs[1][pin] = pin_isr;
        if (condition == rising_edge) {
            LPC_GPIOINT->IO2IntEnR |= (1 << pin);
        }
        else if (condition == falling_edge) {
            LPC_GPIOINT->IO2IntEnF |= (1 << pin);
        }
        else {
            LPC_GPIOINT->IO2IntEnR |= (1 << pin);
            LPC_GPIOINT->IO2IntEnF |= (1 << pin);
        }
    }
    else {
        //port 0
        LabGPIOInterrupts::getInstance().isrs[0][pin] = pin_isr;
        if (condition == rising_edge) {
            LPC_GPIOINT->IO0IntEnR |= (1 << pin);
        }
        else if (condition == falling_edge) {
            LPC_GPIOINT->IO0IntEnF |= (1 << pin);
        }
        else {
            LPC_GPIOINT->IO0IntEnR |= (1 << pin);
            LPC_GPIOINT->IO0IntEnF |= (1 << pin);
        }
    }

    return true;
}
/**
 * This function is invoked by the CPU (through c_eint3_handler) asynchronously when a Port/Pin
 * interrupt occurs. This function is where you will check the Port status, such as IO0IntStatF,
 * and then invoke the user's registered callback and find the entry in your lookup table.
 *
 * VERY IMPORTANT!
 *  - Be sure to clear the interrupt flag that caused this interrupt, or this function will be called
 *    repetitively and lock your system.
 *  - NOTE that your code needs to be able to handle two GPIO interrupts occurring at the same time.
 */
void LabGPIOInterrupts::handle_interrupt(void)
{
    uint32_t stat = 0;
    uint8_t pin = 0;

    if (LPC_GPIOINT->IO0IntStatR || LPC_GPIOINT->IO0IntStatF) {
        if (LPC_GPIOINT->IO0IntStatR) {
            stat = LPC_GPIOINT->IO0IntStatR;
        }
        else {
            stat = LPC_GPIOINT->IO0IntStatF;
        }

        while (stat != 0x1) {
            stat = stat >> 1;
            pin++;
        }

        LabGPIOInterrupts::getInstance().isrs[0][pin]();
        LPC_GPIOINT->IO0IntClr |= (1 << pin);

    }

    pin = 0;
    if (LPC_GPIOINT->IO2IntStatR || LPC_GPIOINT->IO2IntStatF) {
        if (LPC_GPIOINT->IO2IntStatR) {
            stat = LPC_GPIOINT->IO2IntStatR;
        }
        else {
            stat = LPC_GPIOINT->IO2IntStatF;
        }

        while (stat != 0x1) {
            stat = stat >> 1;
            pin++;
        }

        LabGPIOInterrupts::getInstance().isrs[1][pin]();
        LPC_GPIOINT->IO2IntClr |= (1 << pin);
    }

}

