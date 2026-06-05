#include "init.h"
#include "interrupt.h"
#include "sensors.h"
#include "motor.h"

int main(void)
{
    RCC_init();
    SysTick_init();
    button_interrupt_init();
    GPIO_init();
    TIMER2_init();

    while(1)
    {
        __WFI();
    }
}
