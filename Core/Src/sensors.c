#include "sensors.h"

/**
 * @brief Чтение правого датчика
 * @returns 1 если черная линия, иначе 0
 */
uint8_t read_left_sensor(void)
{
    return READ_BIT(GPIOA->IDR, GPIO_IDR_IDR_0) ? 1U : 0U;
}

/**
 * @brief Чтение центрального датчика
 * @returns 1 если черная линия, иначе 0
 */
uint8_t read_middle_sensor(void)
{
    return READ_BIT(GPIOA->IDR, GPIO_IDR_IDR_1) ? 1U : 0U;
}

/**
 * @brief Чтение левого датчика
 * @returns 1 если черная линия, иначе 0
 */
uint8_t read_right_sensor(void)
{
    return READ_BIT(GPIOA->IDR, GPIO_IDR_IDR_4) ? 1U : 0U;
}