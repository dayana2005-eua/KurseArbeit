#include "interrupt.h"
#include "motor.h"     
#include "sensors.h"  

#define SPEED_FAST   450 // Высокая скорость для забегания вперед
#define SPEED_BASE   400 // Базовая скорость для движения прямо
#define SPEED_SLOW   300 // Пониженная скорость для плавного прохождения поворота

static volatile uint32_t ticks = 0;
static volatile uint32_t press_time = 0;
static volatile uint8_t button_permission = 0;
static volatile uint8_t tim2_flag = 0;

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

/**
 * @brief Обработчик прерывания таймера TIM2 (1 кГц)
 */
/**
 * @brief Обработчик прерывания таймера TIM2 (1 кГц)
 */
void TIM2_IRQHandler(void)
{
    if (READ_BIT(TIM2->SR, TIM_SR_UIF)) // Проверяем флаг прерывания по обновлению таймера
    {
        CLEAR_BIT(TIM2->SR, TIM_SR_UIF); // Сбрасываем флаг прерывания
        tim2_flag = 1;                   // Сигнализируем в main о прошедшем тике таймера

        // Если движение запрещено кнопкой — глушим моторы и выходим
        if (button_permission == 0)
        {
            left_motor(0);
            right_motor(0);
            return;
        }

        uint8_t sensor_left  = read_left_sensor();  
        uint8_t sensor_right = read_right_sensor();

        // Память направления для защиты от вылета (0 - центр, 1 - ушел влево, 2 - ушел вправо)
        static uint8_t last_state = 0;

        if (sensor_left && !sensor_right)
        {
            left_motor(0);            // Сместились влево: плавно подруливаем направо
            right_motor(SPEED_BASE); 
            last_state = 1;           // Запоминаем, что линия теряется с левой стороны
        }
        else if (!sensor_left && sensor_right)
        {
            left_motor(SPEED_BASE);   // Сместились вправо: плавно подруливаем налево
            right_motor(0);
        } // Ошибка исправлена: изменено с right_motor(0) на left_motor(SPEED_BASE); right_motor(0);
        else if (sensor_left && sensor_right)
        {
            left_motor(SPEED_SLOW);   // Перекресток или резкий занос: разворот на месте
            right_motor(-SPEED_SLOW); 
            last_state = 0;           // На перекрестке сбрасываем память направления
        }
        else // !sensor_left && !sensor_right — оба датчика видят белое field
        {
            if (last_state == 1)      // Если до этого робот слишком сильно ушел влево
            {
                left_motor(0);            // Продолжаем активно доворачивать направо
                right_motor(SPEED_BASE);
            }
            else if (last_state == 2) // Если до этого робот слишком сильно ушел вправо
            {
                left_motor(SPEED_BASE);   // Продолжаем активно доворачивать налево
                right_motor(0);
            }
            else                      // Идеальное состояние: линия ровно посередине между датчиками
            {
                left_motor(SPEED_BASE);   // Спокойно едем вперед
                right_motor(SPEED_BASE);
            }
        }
    }
}



uint8_t get_tim2_flag(void)
{
    return tim2_flag;
}

void set_tim2_flag(uint8_t flag)
{
    tim2_flag = flag;
}
