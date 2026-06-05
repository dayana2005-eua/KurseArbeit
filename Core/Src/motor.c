#include "motor.h"
/**
 * @brief Функция управления правым мотором. Используемые пины PA5, PA6 (направление) и
 *  таймер TIM2 скорость вращения PB10
 * @note Функция принимает скорость в диапазоне от -1000 до 1000
 * @return None
 */
void left_motor(int16_t speed)
{   
    if(speed > 0) // Вращение вперед
    {
        WRITE_REG(GPIOA->BSRR, GPIO_BSRR_BR_5 | GPIO_BSRR_BS_6); // Пятый включен шестой отключен
        WRITE_REG(TIM2->CCR3, speed);
    }
    else if(speed < 0) // Вращение назад
    {   
        speed = -speed;
        WRITE_REG(GPIOA->BSRR, GPIO_BSRR_BS_5 | GPIO_BSRR_BR_6); // Пятый выключен шестой включен
        WRITE_REG(TIM2->CCR3, speed);
    }
    else // Останов
    {
        WRITE_REG(GPIOA->BSRR, GPIO_BSRR_BS_5 | GPIO_BSRR_BS_6); // Оба выключены, плавное торможение
        WRITE_REG(TIM2->CCR3, 1000);
    }
}

/**
 * @brief Функция управления левым мотором. Используемые пины PA7 (направление), PB6 (направление) и
 *  таймер TIM2 скорость вращения PB3
 * @note Функция принимает скорость в диапазоне от -1000 до 1000
 * @return None
 */
void right_motor(int16_t speed)
{   
    if(speed > 0) // Вращение вперед
    {
        // Седьмой пин порта А включен, шестой пин порта B отключен
        WRITE_REG(GPIOA->BSRR, GPIO_BSRR_BS_7);
        WRITE_REG(GPIOB->BSRR, GPIO_BSRR_BR_6);
        WRITE_REG(TIM2->CCR2, speed);
    }
    else if(speed < 0) // Вращение назад
    {   
        speed = -speed;
        // Седьмой пин порта А выключен, шестой пин порта B включен
        WRITE_REG(GPIOA->BSRR, GPIO_BSRR_BR_7);
        WRITE_REG(GPIOB->BSRR, GPIO_BSRR_BS_6);
        WRITE_REG(TIM2->CCR2, speed);
    }
    else // Останов
    {
        // Оба выключены, плавное торможение
        WRITE_REG(GPIOA->BSRR, GPIO_BSRR_BS_7);
        WRITE_REG(GPIOB->BSRR, GPIO_BSRR_BS_6);
        WRITE_REG(TIM2->CCR2, 1000);
    }
}
