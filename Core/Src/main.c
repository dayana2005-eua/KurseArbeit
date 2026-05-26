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

    uint8_t left = 0;
    uint8_t middle = 0;
    uint8_t right = 0;

    const int16_t BASE_SPEED = 400;

    while(1)
    {
        if (get_button_permission())
        {
            left   = read_left_sensor();
            middle = read_middle_sensor();
            right  = read_right_sensor();

            if (middle == 1)
            {
                left_motor(BASE_SPEED);
                right_motor(BASE_SPEED);
            }
            else if (left == 1)
            {
                left_motor(BASE_SPEED / 2);  
                right_motor(BASE_SPEED);      
            }
            else if (right == 1)
            {
                left_motor(BASE_SPEED);
                right_motor(BASE_SPEED / 2); 
            }
            else
            {
                left_motor(0);
                right_motor(0);
            }
        }
        else
        {
            left_motor(0);
            right_motor(0);
            left = 0;
            middle = 0;
            right = 0;
        }
    }
}
