#include "interrupt.h"
static volatile uint32_t ticks = 0;
/**
 * @brief Прерывание системного таймера
 */
void SysTick_Handler(void)
{
    ticks++;
}

/**
 * @brief Получение тиков
 */
uint32_t get_ticks(void)
{
    return ticks;
}