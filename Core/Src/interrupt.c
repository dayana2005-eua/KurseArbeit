#include "interrupt.h"
#include "motor.h"     
#include "sensors.h"  

// Настройки скоростей (адаптировано под массу 1 кг и L298N)
#define SPEED_FAST   550 // Ускоренное внешнее колесо в повороте
#define SPEED_BASE   450 // Базовая скорость прямо
#define SPEED_MIN    350 // Грань сваливания (минимальная скорость работы мотора)
#define MIN_PRESS_DURATION 50 // Время антидребезга кнопки в миллисекундах (тиках)

static volatile uint32_t ticks = 0;
static volatile uint32_t press_time = 0;
static volatile uint8_t button_permission = 0;
static volatile uint8_t tim2_flag = 0;

// Память состояний линии
static uint8_t last_state = 0; // 0 - прямо, 1 - поворот влево, 2 - поворот вправо

// Переменные для подсчета перекрестков
static uint8_t cross_count = 0;   // Счетчик перекрестков
static uint8_t is_on_cross = 0;   // Флаг нахождения на перекрестке (защита от множественных срабатываний)

static void led_toggle(void)
{
    GPIOC->ODR ^= GPIO_ODR_ODR_2;
}

void SysTick_Handler(void)
{
    static uint32_t led_counter = 0; 
    led_counter++;
    uint32_t current_interval = (button_permission == 1) ? 100U : 500U;

    if (led_counter >= current_interval)
    {
        led_toggle();     
        led_counter = 0;  
    }
    ticks++;
}

uint32_t get_ticks(void)
{
    return ticks;
}

void button_interrupt_init(void)
{
    SET_BIT(RCC->APB2ENR, RCC_APB2ENR_SYSCFGEN);
    MODIFY_REG(SYSCFG->EXTICR[13 / 4], 0xFU << ((13 % 4) * 4), 0x2U << ((13 % 4) * 4));
    SET_BIT(EXTI->IMR, EXTI_IMR_IM13);
    SET_BIT(EXTI->FTSR, EXTI_FTSR_TR13);
    SET_BIT(EXTI->RTSR, EXTI_RTSR_TR13);
    NVIC_SetPriority(EXTI15_10_IRQn, 2); 
    NVIC_EnableIRQ(EXTI15_10_IRQn);
}

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
                button_permission = !button_permission; 
                
                // Если мы заново запускаем робота кнопкой, сбрасываем счетчик перекрестков
                if (button_permission == 1) 
                {
                    cross_count = 0;
                    is_on_cross = 0;
                }
            }
        }
        SET_BIT(EXTI->PR, EXTI_PR_PR13);
    }
}

uint8_t get_button_permission(void)
{
    return button_permission;
}

void TIM2_IRQHandler(void)
{
    if (READ_BIT(TIM2->SR, TIM_SR_UIF)) 
    {
        CLEAR_BIT(TIM2->SR, TIM_SR_UIF); 
        tim2_flag = 1;

        if (button_permission == 0)
        {
            left_motor(0);
            right_motor(0);
            return;
        }

        uint8_t s_left   = read_right_sensor();  
        uint8_t s_middle = read_middle_sensor(); 
        uint8_t s_right  = read_left_sensor();   

       // === 1. ЛОГИКА ПЕРЕКРЕСТКОВ И ОСТАНОВКИ ===
        
        // Главный признак Х-петли или Т-перекрестка: оба крайних на черном.
        // Центральный может быть где угодно (на черном или в белом зазоре креста).
        if (s_left && s_right)
        {
            if (is_on_cross == 0)
            {
                cross_count++;      
                is_on_cross = 1;    
            }

            // Проверка финиша (3 перекрестка: вход в петлю, выход из петли, Т-конец)
            if (cross_count >= 3)
            {
                left_motor(0);
                right_motor(0);
                button_permission = 0; 
                return; 
            }
            
            // ПРОБИВАЕМ ПЕРЕКРЕСТОК НАСКВОЗЬ
            // Жестко даем команду ехать прямо, чтобы он не дернулся в поворот
            last_state = 0; 
            left_motor(SPEED_BASE); 
            right_motor(SPEED_BASE);
            return; 
        }
        else
        {
            // Сброс флага: мы съехали с перекрестка, когда ОБА крайних датчика вышли на белое
            if (!s_left && !s_right)
            {
                is_on_cross = 0;
            }
        }
        
        // === 2. ЛОГИКА РУЛЕНИЯ (ЕЗДА ПО ЛИНИИ) ===
        
        // ИДЕАЛЬНО ПРЯМО
        if (!s_left && s_middle && !s_right)
        {
            last_state = 0;
            left_motor(SPEED_BASE);
            right_motor(SPEED_BASE);
        }
        
        // ЛЕГКАЯ ДУГА ВЛЕВО
        else if (s_left && s_middle && !s_right)
        {
            last_state = 1;
            left_motor(SPEED_MIN);  
            right_motor(SPEED_FAST); 
        }
        
        // ЛЕГКАЯ ДУГА ВПРАВО
        else if (!s_left && s_middle && s_right)
        {
            last_state = 2;
            left_motor(SPEED_FAST);
            right_motor(SPEED_MIN); 
        }
        
        // КРУТАЯ ДУГА ВЛЕВО
        else if (s_left && !s_middle && !s_right)
        {
            last_state = 1;
            left_motor(0);    
            right_motor(SPEED_FAST);
        }
        
        // КРУТАЯ ДУГА ВПРАВО
        else if (!s_left && !s_middle && s_right)
        {
            last_state = 2;
            left_motor(SPEED_FAST);
            right_motor(0);   
        }
        
        // ПОТЕРЯ ЛИНИИ (угол 90 градусов) - вращение на месте для поиска
        else if (!s_left && !s_middle && !s_right)
        {
            if (last_state == 1)
            {
                left_motor(-SPEED_MIN);
                right_motor(SPEED_MIN);
            }
            else if (last_state == 2)
            {
                left_motor(SPEED_MIN);
                right_motor(-SPEED_MIN);
            }
            else 
            {
                left_motor(0);
                right_motor(0);
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