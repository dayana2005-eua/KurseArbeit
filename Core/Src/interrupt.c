#include "interrupt.h"

static volatile uint32_t ticks = 0;
static volatile uint32_t press_time = 0;
static volatile uint8_t button_permission = 0;

static void led_toggle(void)
{
    GPIOC->ODR ^= GPIO_ODR_ODR_2;
}

/**
 * @brief Прерывание системного таймера
 */
void SysTick_Handler(void)
{
    static uint32_t led_counter = 0; 
    led_counter++;

    uint32_t current_interval = (button_permission == 1) ? 100U : 500U;

    if (led_counter >= current_interval)
    {
        led_toggle();     // Меняем состояние светодиода (вкл/выкл)
        led_counter = 0;  // Сбрасываем счетчик для нового отсчета
    }

    ticks++;
}

/**
 * @brief Получение тиков
 */
uint32_t get_ticks(void)
{
    return ticks;
}

/**
 * @brief Инициализация внешних прерываний
 */
void button_interrupt_init(void)
{
    SET_BIT(RCC->APB2ENR, RCC_APB2ENR_SYSCFGEN);
    MODIFY_REG(SYSCFG->EXTICR[13 / 4], 0xFU << ((13 % 4) * 4), 0x2U << ((13 % 4) * 4));

    SET_BIT(EXTI->IMR, EXTI_IMR_IM13);

    SET_BIT(EXTI->FTSR, EXTI_FTSR_TR13);
    SET_BIT(EXTI->RTSR, EXTI_RTSR_TR13);

    NVIC_SetPriority(EXTI15_10_IRQn, 2); // Приоритет ниже, чем у SysTick
    NVIC_EnableIRQ(EXTI15_10_IRQn);
}

/**
 * @brief Обработчик внешнего прерывания по кнопке PC13
 */
void EXTI15_10_IRQHandler(void)
{

    if (READ_BIT(EXTI->PR, EXTI_PR_PR13) != 0)
    {

        if (READ_BIT(GPIOC->IDR, GPIO_IDR_IDR_13) == 0)
        {
            press_time = ticks;
        }
        else
        {
            uint32_t release_time = ticks;
            
            if ((release_time - press_time) >= MIN_PRESS_DURATION)
            {
                button_permission = !button_permission; // Меняем флаг старт/стоп
            }
        }

        SET_BIT(EXTI->PR, EXTI_PR_PR13);
    }
}
/**
 * @brief Проверка состояния кнопки
 * @return uint8_t
 */
uint8_t get_button_permission(void)
{
    return button_permission;
}
