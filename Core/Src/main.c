#include "init.h"
#include "interrupt.h"
#include "sensors.h"
#include "motor.h"

volatile uint8_t tim2_flag = 0;

const int16_t HIGH_SPEED = 450; 
const int16_t MOTOR_TRIM = -30; 

int main(void)
{
    RCC_init();
    SysTick_init();
    button_interrupt_init();
    GPIO_init();
    TIMER2_init(); // Запуск ШИМ и прерываний таймера

    uint8_t left = 0;
    uint8_t right = 0;

    while(1)
    {
        // Процессор физически засыпает и ждет прерывания таймера
        while (get_tim2_flag() == 0)
        {
            __WFI(); 
        }

        set_tim2_flag(0); // Сбрасываем флаг и выполняем шаг управления строго раз в 1 мс

        if (get_button_permission())
        {
            // Опрос датчиков выполняется в main
            left  = read_left_sensor();
            right = read_right_sensor();

            // Алгоритм "вилки" (линия удерживается между датчиками)
            if (left == 0 && right == 0)
            {
                // Линия по центру -> едем прямо
                left_motor(HIGH_SPEED + MOTOR_TRIM);
                right_motor(HIGH_SPEED);
            }
            else if (left == 1 && right == 0)
            {
                // Линия слева -> доворачиваем влево
                left_motor(HIGH_SPEED + MOTOR_TRIM);
                right_motor(0); 
            }
            else if (right == 1 && left == 0)
            {
                // Линия справа -> доворачиваем вправо
                left_motor(0 + MOTOR_TRIM); 
                right_motor(HIGH_SPEED);
            }
            else
            {
                // Перекресток или полная потеря линии -> стоп
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