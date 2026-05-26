#include "init.h"
#include "interrupt.h"
#include "sensors.h"

int main()
{
    RCC_init();
    SysTick_init();
    GPIO_init();
    TIMER2_init();

    uint8_t left = 0;
    uint8_t middle = 0;
    uint8_t right = 0;
    uint8_t permission = 0;

    const int16_t BASE_SPEED = 400;

    while(1)
    {
        if (permission)
        {
        
            left   = read_left_sensor();
            middle = read_middle_sensor();
            right  = read_right_sensor();

            if (middle == 1 && left == 0 && right == 0)
            {
                // Линия строго по центру — едем прямо
                left_motor(BASE_SPEED);
                right_motor(BASE_SPEED);
            }
            else if (left == 1 && right == 0)
            {
                // Линия ушла влево — плавно поворачиваем налево
                // (Левое колесо замедляем, правое толкает)
                left_motor(BASE_SPEED / 2);  
                right_motor(BASE_SPEED);      
            }
            else if (right == 1 && left == 0)
            {
                // Линия ушла вправо — плавно поворачиваем направо
                // (Правое колесо замедляем, левое толкает)
                left_motor(BASE_SPEED);
                right_motor(BASE_SPEED / 2); 
            }
            else
            {
                // Робот полностью вылетел с линии. 
                left_motor(0);
                right_motor(0);
            }
        }
        else
        {
            left = 0;
            middle = 0;
            right = 0;
        }
    }
}