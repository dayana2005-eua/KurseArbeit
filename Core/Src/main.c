#include "init.h"
#include "interrupt.h"
#include "sensors.h"
#include "motor.h"

// ================= НАСТРОЙКА ПАРАМЕТРОВ РОБОТА =================
#define HIGH_SPEED   450   // Маршевая скорость бега вперед по прямой
#define MOTOR_TRIM   (-30) // Аппаратный баланс левого мотора
#define INNER_SPEED  0     // Скорость внутреннего колеса при повороте
// ===============================================================

int main(void)
{
    RCC_init();
    SysTick_init();
    button_interrupt_init();
    GPIO_init();
    TIMER2_init(); 

    uint8_t left = 0;
    uint8_t right = 0;

    while(1)
    {
        while (get_tim2_flag() == 0)
        {
            __WFI(); 
        }
        set_tim2_flag(0); 

        if (get_button_permission())
        {
            left  = read_left_sensor();
            right = read_right_sensor();

            if (left == 0 && right == 0)
            {
                left_motor(HIGH_SPEED + MOTOR_TRIM);
                right_motor(HIGH_SPEED);
            }
            else if (left == 1 && right == 0)
            {
                left_motor(HIGH_SPEED + MOTOR_TRIM);
                right_motor(INNER_SPEED); 
            }
            else if (right == 1 && left == 0)
            {
                left_motor(INNER_SPEED + MOTOR_TRIM); 
                right_motor(HIGH_SPEED);
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
            right = 0;
        }
    }
}
